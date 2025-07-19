#ifndef _INTEGER_WORLD_OUTPUTS_ADAFRUIT_ST7735_AND_ST7789_INCLUDE_h
#define _INTEGER_WORLD_OUTPUTS_ADAFRUIT_ST7735_AND_ST7789_INCLUDE_h

// Include Adafruit ST7735 and ST7789 output surfaces. 
// Depends on Adafruit_ST7735 and Adafruit_ST7789 library (https://github.com/adafruit/Adafruit-ST7735-Library).
#if defined(ARDUINO) && !defined(ARDUINO_ARCH_STM32F4)
#include "OutputSurfaces/Adafruit/AdafruitSurfaceST7735.h"
#include "OutputSurfaces/Adafruit/AdafruitSurfaceST7789.h"
#endif

#endif