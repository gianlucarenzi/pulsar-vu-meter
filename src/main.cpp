/*
 * main.cpp - Firmware for a CPU VU-meter using NeoPixel LEDs
 *
 * This sketch reads CPU load data from a serial port and displays it
 * on a 12-LED WS2812B strip in a multi-segmented VU-meter style.
 *
 * This project is configured for PlatformIO and supports multiple boards
 * like Arduino Nano and ESP32.
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

/* Pin and LED configuration */
#ifndef VU_METER_DATA_PIN_1
#error "VU_METER_DATA_PIN_1 is not defined! Please define it for your environment in platformio.ini"
#endif

#define VU_METER_NUM_LEDS   12   /* Number of LEDs in each strip */
#define MAX_STRIPS          3    /* Maximum number of strips supported */

/* Serial communication protocol defines (only used in serial mode) */
#ifndef INPUT_MODE_ANALOG
#define VU_METER_HEADER_1   0xAB
#define VU_METER_HEADER_2   0xBA
#endif

/* Command definitions */
#define CMD_REVERSE_ORDER   0x01

/*
 * LED color segment definitions.
 * Defines the end-point for each color segment (index starts at 0).
 */
#define SEGMENT_END_GREEN   3    /* LEDs 0, 1, 2 */
#define SEGMENT_END_YELLOW  6    /* LEDs 3, 4, 5 */
#define SEGMENT_END_ORANGE  9    /* LEDs 6, 7, 8 */
/* The last segment (red) goes from SEGMENT_END_ORANGE to VU_METER_NUM_LEDS */

/* Global state for the device */
static bool vu_meter_reverse_order = false; /* Flag to reverse VU-Meter direction */

/*
 * Array to hold one or more LED strip objects.
 * We use pointers to avoid issues with object copying and a C-style
 * array for maximum compatibility (AVR does not have std::vector).
 */
Adafruit_NeoPixel* strips[MAX_STRIPS];
int active_strips_count = 0;


/*
 * vu_meter_get_color - Determine color based on LED position.
 * @led_index: The 0-based index of the LED in the strip.
 *
 * Returns the uint32_t color value for the given LED index based on
 * the defined color segments.
 */
static uint32_t vu_meter_get_color(int led_index)
{
    // Color values are the same for all strips, so we can use the first one to generate them.
    if (active_strips_count == 0) return 0;

    if (led_index < SEGMENT_END_GREEN) {
        return strips[0]->Color(0, 150, 0);   /* Green */
    } else if (led_index < SEGMENT_END_YELLOW) {
        return strips[0]->Color(150, 150, 0); /* Yellow */
    } else if (led_index < SEGMENT_END_ORANGE) {
        return strips[0]->Color(255, 100, 0); /* Orange */
    } else {
        return strips[0]->Color(200, 0, 0);   /* Red */
    }
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
            pixel_index = vu_meter_reverse_order ? (VU_METER_NUM_LEDS - 1) : 0;
            strips[i]->setPixelColor(pixel_index, vu_meter_get_color(0));
            strips[i]->show();
        }
        return; /* Exit function */
    }

    /* Map level (5-100) to number of LEDs (1-12) */
    leds_to_light = map(level, 5, 100, 1, VU_METER_NUM_LEDS);
    leds_to_light = constrain(leds_to_light, 0, VU_METER_NUM_LEDS);

    /* Update each individual LED on all strips */
    for (i = 0; i < VU_METER_NUM_LEDS; i++) {
        pixel_index = vu_meter_reverse_order ? (VU_METER_NUM_LEDS - 1 - i) : i;
        uint32_t color = (i < leds_to_light) ? vu_meter_get_color(i) : color_off;
        
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
#ifdef VU_METER_DATA_PIN_1
    if (active_strips_count < MAX_STRIPS) {
        strips[active_strips_count++] = new Adafruit_NeoPixel(VU_METER_NUM_LEDS, VU_METER_DATA_PIN_1, NEO_GRB + NEO_KHZ800);
    }
#endif
#ifdef VU_METER_DATA_PIN_2
    if (active_strips_count < MAX_STRIPS) {
        strips[active_strips_count++] = new Adafruit_NeoPixel(VU_METER_NUM_LEDS, VU_METER_DATA_PIN_2, NEO_GRB + NEO_KHZ800);
    }
#endif
#ifdef VU_METER_DATA_PIN_3
    if (active_strips_count < MAX_STRIPS) {
        strips[active_strips_count++] = new Adafruit_NeoPixel(VU_METER_NUM_LEDS, VU_METER_DATA_PIN_3, NEO_GRB + NEO_KHZ800);
    }
#endif

    // Initialize all created strips
    for (int i = 0; i < active_strips_count; i++) {
        strips[i]->begin();
        strips[i]->show(); // Turn off all LEDs at startup
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
     * Map the peak-to-peak amplitude to the 0-100 range.
     * A 2Vp-p signal on a 5V Arduino is ~410 ADC units.
     * We map a range from 10 (noise floor) to 500 for good sensitivity.
     * This may need tuning based on the actual audio circuit.
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
        if (Serial.read() == VU_METER_HEADER_1 && Serial.read() == VU_METER_HEADER_2) {
            int cpu_val;
            int command_val;

            /* 2. Read packet data */
            cpu_val = Serial.read();    /* CPU load value (0-100) */
            command_val = Serial.read();/* Special command value */

            /* 3. Process commands */
            if (command_val == CMD_REVERSE_ORDER) {
                vu_meter_reverse_order = !vu_meter_reverse_order; /* Toggle the reverse flag */
            }

            /* 4. Update LEDs based on CPU value */
            update_leds(cpu_val);
        }
    }
#endif
}

