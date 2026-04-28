#ifndef IMAGE_ARRAY_HANUMAN_H
#define IMAGE_ARRAY_HANUMAN_H

#include <stdint.h>

#define LCD_W 176
#define LCD_H 220
#define LCD_PIXELS (LCD_W * LCD_H)

/*
 * Replace these with your real RGB565 frame arrays.
 * These defaults are single-color demo frames so the app compiles/runs.
 */
static const uint16_t image1[LCD_PIXELS] = { [0 ... (LCD_PIXELS - 1)] = 0xF800 }; /* Red */
static const uint16_t image2[LCD_PIXELS] = { [0 ... (LCD_PIXELS - 1)] = 0x07E0 }; /* Green */
static const uint16_t image3[LCD_PIXELS] = { [0 ... (LCD_PIXELS - 1)] = 0x001F }; /* Blue */
static const uint16_t image4[LCD_PIXELS] = { [0 ... (LCD_PIXELS - 1)] = 0xFFFF }; /* White */
static const uint16_t image5[LCD_PIXELS] = { [0 ... (LCD_PIXELS - 1)] = 0x0000 }; /* Black */
static const uint16_t image6[LCD_PIXELS] = { [0 ... (LCD_PIXELS - 1)] = 0xFFE0 }; /* Yellow */
static const uint16_t image7[LCD_PIXELS] = { [0 ... (LCD_PIXELS - 1)] = 0xF81F }; /* Magenta */

#endif
