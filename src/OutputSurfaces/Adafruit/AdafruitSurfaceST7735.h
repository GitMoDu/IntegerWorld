#ifndef _INTEGER_WORLD_ADAFRUIT_SURFACE_ST7735_h
#define _INTEGER_WORLD_ADAFRUIT_SURFACE_ST7735_h

#include "AdafruitSurface.h"
#include <Adafruit_ST7735.h>

namespace IntegerWorld
{
	namespace Adafruit
	{
		namespace St7735
		{
			enum class VariantEnum
			{
				GreenTab = INITR_GREENTAB,
				RedTab = INITR_REDTAB,
				BlackTab = INITR_BLACKTAB,
				Green144Tab = INITR_144GREENTAB,
				Mini160x80Tab = INITR_MINI160x80,
				Mini160x80Plugin = INITR_MINI160x80_PLUGIN,
				HallowWing = INITR_HALLOWING
			};

			/// <summary>
			/// Direct Draw 160x160/160x80 Display with 16 bit color.
			/// Implementation can disable backlight while drawing, resulting in a soft black-frame-insertion.
			/// Supports different variants through VariantEnum.
			/// </summary>
			class DirectDrawSurface : public TemplateAdafruitDirectDrawSurface<Adafruit_ST7735, Adafruit16BitColorConverter>
			{
			private:
				using Base = TemplateAdafruitDirectDrawSurface<Adafruit_ST7735, Adafruit16BitColorConverter>;

			private:
				Adafruit_ST7735 Display;
				VariantEnum DisplayType;
				int8_t Bkl;
				bool BfiEnabled;

			public:
				DirectDrawSurface(const VariantEnum displayType, SpiType& spi,
					const  int8_t cs, const int8_t dc, const int8_t rst,
					const int8_t bkl = -1, 
					const bool bfiEnabled = false,
					const uint32_t targetPeriod = 0000)
					: Base(Display, targetPeriod)
					, Display(&spi, cs, dc, rst)
					, DisplayType(displayType)
					, Bkl(bkl)
					, BfiEnabled(bfiEnabled)
				{
				}

				void PrintName(Print& serial)
				{
					serial.print(F("SSD7735 SPI Adafruit"));
				}

			public:
				void GetSurfaceDimensions(int16_t& width, int16_t& height, uint8_t& colorDepth) final
				{
					switch (DisplayType)
					{
					case VariantEnum::Mini160x80Plugin:
					case VariantEnum::Mini160x80Tab:
						width = 160;
						height = 80;
						break;
					default:
						width = Display.width();
						height = Display.height();
						break;
					}
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
					Display.initR(uint8_t(DisplayType));
					switch (DisplayType)
					{
					case VariantEnum::Mini160x80Plugin:
					case VariantEnum::Mini160x80Tab:
						Display.setRotation(3);
						break;
					default:
						Display.setRotation(0);
						break;
					}

					Display.invertDisplay(false);
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

