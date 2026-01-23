/*
 * main.cpp - Firmware for the Pulsar VU project
 *
 * This firmware can operate in two modes:
 * 1. CPU Monitor: Reads CPU load data from a serial port and displays it.
 * 2. Audio VU-Meter: Reads an analog audio signal and displays its volume.
 *
 * This project is configured for PlatformIO and supports multiple boards
 * and configurations.
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

/* Pin and LED configuration */
#ifndef PULSAR_VU_DATA_PIN_1
#error "PULSAR_VU_DATA_PIN_1 is not defined! Please define it for your environment in platformio.ini"
#endif

#define PULSAR_VU_NUM_LEDS   12   /* Number of LEDs in each strip */
#define MAX_STRIPS          3    /* Maximum number of strips supported */

/*
 * To use a custom color, define these in your build flags or before including this file:
 * #define LED_CUSTOM_R 100
 * #define LED_CUSTOM_G 100
 * #define LED_CUSTOM_B 100
 */
/* Serial communication protocol defines (only used in serial mode) */
#ifndef INPUT_MODE_ANALOG
#define PULSAR_VU_HEADER_1   0xAB
#define PULSAR_VU_HEADER_2   0xBA
#endif

/* Command definitions */
#define CMD_REVERSE_ORDER   0x01

/*
 * LED color segment definitions.
 * Defines the end-point for each color segment (index starts at 0).
 *
 * To force all LEDs to a fixed color, define LED_FIXED_COLOR as one of:
 *   LED_COLOR_GREEN, LED_COLOR_YELLOW, LED_COLOR_ORANGE, LED_COLOR_RED, LED_COLOR_CUSTOM
 * If LED_COLOR_CUSTOM is used, set LED_CUSTOM_R, LED_CUSTOM_G, LED_CUSTOM_B to desired values.
 */
#define SEGMENT_END_GREEN   3    /* LEDs 0, 1, 2 */
#define SEGMENT_END_YELLOW  6    /* LEDs 3, 4, 5 */
#define SEGMENT_END_ORANGE  9    /* LEDs 6, 7, 8 */
/* The last segment (red) goes from SEGMENT_END_ORANGE to PULSAR_VU_NUM_LEDS */
#define LED_COLOR_GREEN   1
#define LED_COLOR_YELLOW  2
#define LED_COLOR_ORANGE  3
#define LED_COLOR_RED     4
#define LED_COLOR_CUSTOM  5
/* Example: #define LED_FIXED_COLOR LED_COLOR_GREEN */

/* Global state for the device */
static bool pulsar_reverse_order = false; /* Flag to reverse VU-Meter direction */

/*
 * Array to hold one or more LED strip objects.
 * We use pointers to avoid issues with object copying and a C-style
 * array for maximum compatibility (AVR does not have std::vector).
 */
Adafruit_NeoPixel* strips[MAX_STRIPS];
int active_strips_count = 0;


/*
 * pulsar_get_color - Determine color based on LED position.
 * @led_index: The 0-based index of the LED in the strip.
 *
 * Returns the uint32_t color value for the given LED index based on
 * the defined color segments.
 */
static uint32_t pulsar_get_color(int led_index)
{
    if (active_strips_count == 0) return 0;
#ifdef LED_FIXED_COLOR
    #if LED_FIXED_COLOR == LED_COLOR_GREEN
        return strips[0]->Color(0, 150, 0);
    #elif LED_FIXED_COLOR == LED_COLOR_YELLOW
        return strips[0]->Color(150, 150, 0);
    #elif LED_FIXED_COLOR == LED_COLOR_ORANGE
        return strips[0]->Color(255, 100, 0);
    #elif LED_FIXED_COLOR == LED_COLOR_RED
        return strips[0]->Color(200, 0, 0);
    #elif LED_FIXED_COLOR == LED_COLOR_CUSTOM
        return strips[0]->Color(LED_CUSTOM_R, LED_CUSTOM_G, LED_CUSTOM_B);
    #else
        return strips[0]->Color(0, 0, 0); // fallback: off
    #endif
#else
    if (led_index < SEGMENT_END_GREEN) {
        return strips[0]->Color(0, 150, 0);   /* Green */
    } else if (led_index < SEGMENT_END_YELLOW) {
        return strips[0]->Color(150, 150, 0); /* Yellow */
    } else if (led_index < SEGMENT_END_ORANGE) {
        return strips[0]->Color(255, 100, 0); /* Orange */
    } else {
        return strips[0]->Color(200, 0, 0);   /* Red */
    }
#endif
}

/*
 * update_leds - Updates all LED strips based on the level.
 * @level: The level to display, 0-100.
 *
 * This function implements the multi-segment VU-meter logic, including
 * the special case for low CPU usage and color segmentation.
 */
static void update_leds(int level)
{
    if (active_strips_count == 0) return;

    int i;
    int leds_to_light;
    int pixel_index;
    uint32_t color_off = strips[0]->Color(0, 0, 0);

    /* Special case: if level is very low, light only the first LED green */
    if (level < 5) {
        for (i = 0; i < active_strips_count; i++) {
            strips[i]->clear();
            pixel_index = pulsar_reverse_order ? (PULSAR_VU_NUM_LEDS - 1) : 0;
            strips[i]->setPixelColor(pixel_index, pulsar_get_color(0));
            strips[i]->show();
        }
        return; /* Exit function */
    }

    /* Map level (5-100) to number of LEDs (1-12) */
    leds_to_light = map(level, 5, 100, 1, PULSAR_VU_NUM_LEDS);
    leds_to_light = constrain(leds_to_light, 0, PULSAR_VU_NUM_LEDS);

    /* Update each individual LED on all strips */
    for (i = 0; i < PULSAR_VU_NUM_LEDS; i++) {
        pixel_index = pulsar_reverse_order ? (PULSAR_VU_NUM_LEDS - 1 - i) : i;
        uint32_t color = (i < leds_to_light) ? pulsar_get_color(i) : color_off;
        
        for (int j = 0; j < active_strips_count; j++) {
            strips[j]->setPixelColor(pixel_index, color);
        }
    }

    /* Send the new colors to all strips at once */
    for (i = 0; i < active_strips_count; i++) {
        strips[i]->show();
    }
}

/*
 * setup - Arduino's initialization function.
 *
 * Initializes serial communication (if needed) and creates/initializes
 * all NeoPixel strip objects defined in the build flags.
 */
void setup()
{
#ifndef INPUT_MODE_ANALOG
    Serial.begin(9600); /* Initialize serial communication only in serial mode */
#endif

    // Conditionally create strip objects based on defined pins
#ifdef PULSAR_VU_DATA_PIN_1
    if (active_strips_count < MAX_STRIPS) {
        strips[active_strips_count++] = new Adafruit_NeoPixel(PULSAR_VU_NUM_LEDS, PULSAR_VU_DATA_PIN_1, NEO_GRB + NEO_KHZ800);
    }
#endif
#ifdef PULSAR_VU_DATA_PIN_2
    if (active_strips_count < MAX_STRIPS) {
        strips[active_strips_count++] = new Adafruit_NeoPixel(PULSAR_VU_NUM_LEDS, PULSAR_VU_DATA_PIN_2, NEO_GRB + NEO_KHZ800);
    }
#endif
#ifdef PULSAR_VU_DATA_PIN_3
    if (active_strips_count < MAX_STRIPS) {
        strips[active_strips_count++] = new Adafruit_NeoPixel(PULSAR_VU_NUM_LEDS, PULSAR_VU_DATA_PIN_3, NEO_GRB + NEO_KHZ800);
    }
#endif

    // Initialize all created strips
    for (int i = 0; i < active_strips_count; i++) {
        strips[i]->begin();
        strips[i]->show(); // Turn off all LEDs at startup
    }

    // --- STARTUP ANIMATION ---
    // 1. Scorrimento avanti (bianco, barra crescente in 1 secondo)
    for (int i = 0; i < PULSAR_VU_NUM_LEDS; i++) {
        for (int s = 0; s < active_strips_count; s++) {
            strips[s]->clear();
            for (int j = 0; j <= i; j++)
                strips[s]->setPixelColor(j, strips[s]->Color(255,255,255));
            strips[s]->show();
        }
        delay(1000/PULSAR_VU_NUM_LEDS);
    }
    // 2. Scorrimento indietro (bianco, barra decrescente in 1 secondo)
    for (int i = PULSAR_VU_NUM_LEDS-1; i >= 0; i--) {
        for (int s = 0; s < active_strips_count; s++) {
            strips[s]->clear();
            for (int j = 0; j <= i; j++)
                strips[s]->setPixelColor(j, strips[s]->Color(255,255,255));
            strips[s]->show();
        }
        delay(1000/PULSAR_VU_NUM_LEDS);
    }
    // 3. Accensione di tutti i LED bianchi
    for (int s = 0; s < active_strips_count; s++) {
        for (int i = 0; i < PULSAR_VU_NUM_LEDS; i++)
            strips[s]->setPixelColor(i, strips[s]->Color(255,255,255));
        strips[s]->show();
    }
    delay(500);
    // 4. Tutti rossi
    for (int s = 0; s < active_strips_count; s++) {
        for (int i = 0; i < PULSAR_VU_NUM_LEDS; i++)
            strips[s]->setPixelColor(i, strips[s]->Color(255,0,0));
        strips[s]->show();
    }
    delay(300);
    // 5. Fade rosso -> arancio (barra piena)
    for (int step = 0; step <= 40; step++) {
        int r = 255, g = step*6, b = 0;
        for (int s = 0; s < active_strips_count; s++) {
            for (int i = 0; i < PULSAR_VU_NUM_LEDS; i++)
                strips[s]->setPixelColor(i, strips[s]->Color(r,g,b));
            strips[s]->show();
        }
        delay(40);
    }
    // 6. Fade arancio -> giallo (barra piena)
    for (int step = 0; step <= 40; step++) {
        int r = 255-step*6, g = 100+step*4, b = 0;
        for (int s = 0; s < active_strips_count; s++) {
            for (int i = 0; i < PULSAR_VU_NUM_LEDS; i++)
                strips[s]->setPixelColor(i, strips[s]->Color(r,g,b));
            strips[s]->show();
        }
        delay(40);
    }
    // 7. Fade giallo -> verde (barra piena)
    for (int step = 0; step <= 40; step++) {
        int r = 0, g = 200-step*5, b = 0;
        for (int s = 0; s < active_strips_count; s++) {
            for (int i = 0; i < PULSAR_VU_NUM_LEDS; i++)
                strips[s]->setPixelColor(i, strips[s]->Color(r,g<0?0:g,b));
            strips[s]->show();
        }
        delay(40);
    }
    // 5. Fade rosso -> arancio
    for (int step = 0; step <= 20; step++) {
        int r = 255, g = step*5, b = 0;
        for (int s = 0; s < active_strips_count; s++) {
            for (int i = 0; i < PULSAR_VU_NUM_LEDS; i++)
                strips[s]->setPixelColor(i, strips[s]->Color(r,g,b));
            strips[s]->show();
        }
        delay(30);
    }
    // 6. Fade arancio -> giallo
    for (int step = 0; step <= 20; step++) {
        int r = 255-step*5, g = 100+step*5, b = 0;
        for (int s = 0; s < active_strips_count; s++) {
            for (int i = 0; i < PULSAR_VU_NUM_LEDS; i++)
                strips[s]->setPixelColor(i, strips[s]->Color(r,g,b));
            strips[s]->show();
        }
        delay(30);
    }
    // 7. Fade giallo -> verde
    for (int step = 0; step <= 20; step++) {
        int r = 0, g = 200-step*2, b = 0;
        for (int s = 0; s < active_strips_count; s++) {
            for (int i = 0; i < PULSAR_VU_NUM_LEDS; i++)
                strips[s]->setPixelColor(i, strips[s]->Color(r,g<0?0:g,b));
            strips[s]->show();
        }
        delay(30);
    }
    // 8. Spegnimento
    for (int s = 0; s < active_strips_count; s++) {
        strips[s]->clear();
        strips[s]->show();
    }
}


/*
 * loop - Arduino's main continuous loop function.
 *
 * Reads data from the selected source (analog or serial), processes it,
 * and updates the LED display.
 */
void loop()
{
#ifdef INPUT_MODE_ANALOG
    /**************************/
    /* --- MODO AUDIO/ADC --- */
    /**************************/
    const int sample_window_ms = 50; /* Sample window for peak detection */
    unsigned int sample;
    unsigned long start_millis = millis();
    unsigned int peak_to_peak = 0;
    unsigned int signal_max = 0;
    unsigned int signal_min = 1024;

    while (millis() - start_millis < sample_window_ms) {
        sample = analogRead(ANALOG_INPUT_PIN);
        if (sample < 1024) { /* Sanity check for faulty readings */
            if (sample > signal_max) {
                signal_max = sample;
            } else if (sample < signal_min) {
                signal_min = sample;
            }
        }
    }
    peak_to_peak = signal_max - signal_min;

    /*
     * Normalizza la dinamica tra AVR (10 bit, 5V) e ESP32 (12 bit, 3.3V)
     */
    #if defined(ESP32)
        peak_to_peak = peak_to_peak / 4; // Porta il range 0-4095 a circa 0-1023
    #endif

    /*
     * Map the peak-to-peak amplitude to the 0-100 range.
     * A 2Vp-p signal on una board 5V è ~410 ADC units.
     * Si mappa da 10 (rumore) a 500 per buona sensibilità.
     */
    int level = map(peak_to_peak, 10, 500, 0, 100);
    level = constrain(level, 0, 100);

    update_leds(level);

#else
    /***************************/
    /* --- MODO SERIAL/CPU --- */
    /***************************/
    /* Check if enough bytes are available for a complete packet */
    if (Serial.available() >= 4) {
        /* 1. Search for the Header sequence */
        if (Serial.read() == PULSAR_VU_HEADER_1 && Serial.read() == PULSAR_VU_HEADER_2) {
            int cpu_val;
            int command_val;

            /* 2. Read packet data */
            cpu_val = Serial.read();    /* CPU load value (0-100) */
            command_val = Serial.read();/* Special command value */

            /* 3. Process commands */
            if (command_val == CMD_REVERSE_ORDER) {
                pulsar_reverse_order = !pulsar_reverse_order; /* Toggle the reverse flag */
            }

            /* 4. Update LEDs based on CPU value */
            update_leds(cpu_val);
        }
    }
#endif
}

