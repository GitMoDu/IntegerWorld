#ifndef _INTEGER_WORLD_ADAFRUIT_SURFACE_ST7789_h
#define _INTEGER_WORLD_ADAFRUIT_SURFACE_ST7789_h

#include "AdafruitSurface.h"
#include <Adafruit_ST7789.h>

namespace IntegerWorld
{
	namespace Adafruit
	{
		namespace St7789
		{
			/// <summary>
			/// Direct Draw 240x240 Display with 16 bit color.
			/// Implementation can disable backlight while drawing, resulting in a soft black-frame-insertion.
			/// </summary>
			class DirectDrawSurface : public TemplateAdafruitDirectDrawSurface<Adafruit_ST7789, Adafruit16BitColorConverter>
			{
			private:
				using Base = TemplateAdafruitDirectDrawSurface<Adafruit_ST7789, Adafruit16BitColorConverter>;

			private:
				Adafruit_ST7789 Display;
				int8_t Bkl;
				bool BfiEnabled;

			public:
				DirectDrawSurface(SpiType& spi,
					const  int8_t cs, const int8_t dc, const int8_t rst,
					const int8_t bkl = -1,
					const bool bfiEnabled = false,
					const uint32_t targetPeriod = 50000)
					: Base(Display, targetPeriod)
					, Display(&spi, cs, dc, rst)
					, Bkl(bkl)
					, BfiEnabled(bfiEnabled)
				{
				}

				void PrintName(Print& serial)
				{
					serial.print(F("ST7789 SPI Adafruit"));
				}

			public:
				void GetSurfaceDimensions(int16_t& width, int16_t& height, uint8_t& colorDepth) final
				{
					width = 240;
					height = 240;
					colorDepth = 1;
				}

			protected:
				bool StartScreen() final
				{
					if (Bkl != -1)
					{
						pinMode(Bkl, OUTPUT);
						digitalWrite(Bkl, LOW);
					}

					Display.init(240, 240);
					Display.setRotation(2);
					Display.invertDisplay(true);
					Display.fillScreen(ST77XX_BLACK);

					Display.setSPISpeed(MinValue(uint32_t(18000000), uint32_t(F_CPU / 2)));

					return true;
				}

				void OnDrawStart() final
				{
					// Blank screen before clearing buffer and starting drawing.
					if (BfiEnabled)
					{
						Display.enableDisplay(false);
					}
					if (Bkl != -1)
					{
						digitalWrite(Bkl, LOW);
					}
				}

				void OnDrawEnd() final
				{
					// Show drawn content.
					if (BfiEnabled)
					{
						Display.enableDisplay(true);
					}
					if (Bkl != -1)
					{
						digitalWrite(Bkl, HIGH);
					}
				}
			};
		}
	}
}
#endif

