#ifndef _INTEGER_WORLD_OUTPUTS_INCLUDE_h
#define _INTEGER_WORLD_OUTPUTS_INCLUDE_h

#include "OutputSurfaces/MockSurface.h"

#if defined(ARDUINO)
#include "OutputSurfaces/Adafruit/AdafruitSurfaceSSD1306.h"
#include "OutputSurfaces/Adafruit/AdafruitSurfaceSH1106.h"
#include "OutputSurfaces/Adafruit/AdafruitSurfaceSSD1331.h"
#if !defined(ARDUINO_ARCH_STM32F4)
#include "OutputSurfaces/Adafruit/AdafruitSurfaceST7735.h"
#include "OutputSurfaces/Adafruit/AdafruitSurfaceST7789.h"
#endif

#if defined(ARDUINO_ARCH_AVR)
#include "OutputSurfaces/Lexus2kSurfaceSsd1306.h"
#endif
#endif

#include "OutputSurfaces/EgfxSurface.h"

#endif