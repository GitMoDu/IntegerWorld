#ifndef _INTEGER_WORLD_ADAFRUIT_SURFACE_SH1106_h
#define _INTEGER_WORLD_ADAFRUIT_SURFACE_SH1106_h

#include "AdafruitSurface.h"
#include <Adafruit_SH110X.h>

namespace IntegerWorld
{
	namespace Adafruit
	{
		namespace Sh1106Spi
		{
			class FramebufferSurface : public TemplateAdafruitFramebufferMonochromeSurface<Adafruit_SH1106G>
			{
			private:
				using Base = TemplateAdafruitFramebufferMonochromeSurface<Adafruit_SH1106G>;

			public:
				static constexpr uint8_t DisplayWidth = 128;
				static constexpr uint8_t DisplayHeight = 64;

			private:
				Adafruit_SH1106G Display;

			public:
				FramebufferSurface(SpiType& spi, int8_t cs, int8_t dc, int8_t rst, const uint32_t pushPauseDuration = 10000)
					: Base(Display, pushPauseDuration)
#if defined(ARDUINO_ARCH_STM32F1)
					, Display(DisplayWidth, DisplayHeight, &spi, dc, rst, cs)
#else
					, Display(DisplayWidth, DisplayHeight, &spi, dc, rst, cs)
#endif
				{
				}

				void PrintName(Print& serial)
				{
					serial.print(F("SSD1306 SPI Adafruit"));
				}

			protected:
				bool StartScreen() final
				{
					return Display.begin(0, true);
				}
			};
		}
	}
}
#endif

