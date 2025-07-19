#ifndef _INTEGER_WORLD_ADAFRUIT_SURFACE_SSD1331_h
#define _INTEGER_WORLD_ADAFRUIT_SURFACE_SSD1331_h

#include "AdafruitSurface.h"
#include <Adafruit_SSD1331.h>

namespace IntegerWorld
{
	namespace Adafruit
	{
		namespace Ssd1331
		{
			/// <summary>
			/// Direct Draw 96x64 Display with 16 bit color.
			/// Implementation can blank screen while drawing, resulting in a soft black-frame-insertion.
			/// </summary>
			class DirectDrawSurface : public TemplateAdafruitDirectDrawSurface<Adafruit_SSD1331, Adafruit16BitColorConverter>
			{
			private:
				using Base = TemplateAdafruitDirectDrawSurface<Adafruit_SSD1331, Adafruit16BitColorConverter>;

			private:
				Adafruit_SSD1331 Display;
				bool BfiEnabled;

			public:
				DirectDrawSurface(SpiType& spi, int8_t cs, int8_t dc, int8_t rst,
					const bool bfiEnabled = false,
					const uint32_t targetPeriod = 33333)
					: Base(Display, targetPeriod)
					, Display(&spi, cs, dc, rst)
					, BfiEnabled(bfiEnabled)
				{
				}

				void PrintName(Print& serial)
				{
					serial.print(F("SSD1331 SPI Adafruit"));
				}

			protected:
				bool StartScreen() final
				{
					Display.begin();
#if defined(F_CPU)
					Display.setSPISpeed(MinValue(uint32_t(20000000), uint32_t(F_CPU / 2)));
#else
					Display.setSPISpeed(20000000);
#endif

					return true;
				}

				void OnDrawStart() final
				{
					if (BfiEnabled)
					{
						// Blank screen before clearing buffer and starting drawing.
						Display.sendCommand(SSD1331_CMD_DISPLAYALLOFF);
					}
				}

				void OnDrawEnd() final
				{
					if (BfiEnabled)
					{
						// Show drawn content.
						Display.sendCommand(SSD1331_CMD_NORMALDISPLAY);
					}
				}

			};
		}
	}
}
#endif

