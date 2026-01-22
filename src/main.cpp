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
#ifndef VU_METER_DATA_PIN
#error "VU_METER_DATA_PIN is not defined! Please define it for your environment in platformio.ini"
#endif

#define VU_METER_NUM_LEDS   12   /* Number of LEDs in the strip */

/* Serial communication protocol defines */
#define VU_METER_HEADER_1   0xAB
#define VU_METER_HEADER_2   0xBA

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
 * C++ object instantiation for the NeoPixel strip.
 * This is a necessary deviation from pure C kernel style.
 */
Adafruit_NeoPixel strip = Adafruit_NeoPixel(VU_METER_NUM_LEDS,
                                            VU_METER_DATA_PIN,
                                            NEO_GRB + NEO_KHZ800);

/*
 * vu_meter_get_color - Determine color based on LED position.
 * @led_index: The 0-based index of the LED in the strip.
 *
 * Returns the uint32_t color value for the given LED index based on
 * the defined color segments.
 */
static uint32_t vu_meter_get_color(int led_index)
{
    if (led_index < SEGMENT_END_GREEN) {
        return strip.Color(0, 150, 0);   /* Green */
    } else if (led_index < SEGMENT_END_YELLOW) {
        return strip.Color(150, 150, 0); /* Yellow */
    } else if (led_index < SEGMENT_END_ORANGE) {
        return strip.Color(255, 100, 0); /* Orange */
    } else {
        return strip.Color(200, 0, 0);   /* Red */
    }
}

/*
 * update_leds - Updates the LED strip based on the CPU level.
 * @cpu_level: The CPU load percentage (0-100).
 *
 * This function implements the multi-segment VU-meter logic, including
 * the special case for low CPU usage and color segmentation.
 */
static void update_leds(int cpu_level)
{
    int i;
    int leds_to_light;
    int pixel_index;
    uint32_t color_off = strip.Color(0, 0, 0);

    /* Special case: if CPU is very low, light only the first LED green */
    if (cpu_level < 5) {
        strip.clear(); /* Turn off all LEDs */
        pixel_index = vu_meter_reverse_order ? (VU_METER_NUM_LEDS - 1) : 0;
        strip.setPixelColor(pixel_index, vu_meter_get_color(0)); /* First LED is always green */
        strip.show();
        return; /* Exit function */
    }

    /* Map CPU level (5-100) to number of LEDs (1-12) */
    leds_to_light = map(cpu_level, 5, 100, 1, VU_METER_NUM_LEDS);
    leds_to_light = constrain(leds_to_light, 0, VU_METER_NUM_LEDS);

    /* Update each individual LED */
    for (i = 0; i < VU_METER_NUM_LEDS; i++) {
        pixel_index = vu_meter_reverse_order ? (VU_METER_NUM_LEDS - 1 - i) : i;

        if (i < leds_to_light) {
            /* This LED should be on. Determine its color based on its position. */
            strip.setPixelColor(pixel_index, vu_meter_get_color(i));
        } else {
            /* This LED should be off */
            strip.setPixelColor(pixel_index, color_off);
        }
    }

    strip.show(); /* Send the new colors to all LEDs */
}

/*
 * setup - Arduino's initialization function.
 *
 * Sets up serial communication and initializes the NeoPixel strip.
 */
void setup()
{
    Serial.begin(9600); /* Initialize serial communication */
    strip.begin();      /* Initialize NeoPixel library */
    strip.show();       /* Turn off all LEDs at startup */
}

/*
 * loop - Arduino's main continuous loop function.
 *
 * Reads serial data, processes commands, and updates the LED display.
 */
void loop()
{
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
}
