#ifndef _INTEGER_WORLD_SUMOTOY_SSD1331_SURFACE_h
#define _INTEGER_WORLD_SUMOTOY_SSD1331_SURFACE_h

#if defined(ARDUINO)
#include "../Framework/Interface.h"

#define NO_ADAFRUIT_SSD1306_COLOR_COMPATIBILITY
#include <SSD_13XX.h>

namespace IntegerWorld
{
	namespace Sumotoy
	{
		namespace Ssd1331
		{
			/// <summary>
			/// Direct Draw 96x64 Display with 16 bit color.
			/// Based on Sumotoy SSD1331 library (https://github.com/sumotoy/SSD_13XX).
			/// </summary>
			class DirectDrawSurface : public IOutputSurface
			{
			private:
				enum class SurfaceStateEnum : uint8_t
				{
					Disabled,
					WaitingForDraw,
					DrawLocked
				};

			public:
				uint32_t TargetPeriod;

			private:
				uint32_t LastPush = 0;
				SurfaceStateEnum State = SurfaceStateEnum::Disabled;

			private:
				SSD_13XX Display;
				bool BfiEnabled;

			public:
				DirectDrawSurface(const int8_t cs, const int8_t dc, const int8_t rst,
					const bool bfiEnabled = false,
					const uint32_t targetPeriod = 33333)
					: Display(cs, dc, rst)
					, TargetPeriod(targetPeriod)
					, BfiEnabled(bfiEnabled)
				{
					pinMode(3, OUTPUT);
				}

				void PrintName(Print& serial)
				{
					serial.print(F("SSD1331 SPI Sumotoy"));
				}

				bool StartSurface() final
				{
					Display.begin();
					State = SurfaceStateEnum::WaitingForDraw;

					return true;
				}

				void StopSurface() final
				{
					Display.endPushData();
				}

				bool IsSurfaceReady() final
				{
					if (State == SurfaceStateEnum::WaitingForDraw
						&& ((micros() - LastPush) > TargetPeriod))
					{
						digitalWrite(3, 0);
						if (BfiEnabled)
						{
							// Blank screen before clearing buffer and starting drawing.
							Display.changeMode(SSD_13XX_modes::ALL_OFF);
							//Display.changeMode(SSD_13XX_modes::DISP_OFF);
						}
						Display.fillScreen(0);
						State = SurfaceStateEnum::DrawLocked;
					}

					return State == SurfaceStateEnum::DrawLocked;
				}

				void FlipSurface() final
				{
					switch (State)
					{
					case SurfaceStateEnum::DrawLocked:
						LastPush = micros();
						if (BfiEnabled)
						{
							Display.changeMode(SSD_13XX_modes::NORMAL);
						}
						State = SurfaceStateEnum::WaitingForDraw;
						digitalWrite(3, HIGH);
						break;
					default:
						break;
					}
				}

			public:// Buffer window interface.
				void GetSurfaceDimensions(int16_t& width, int16_t& height, uint8_t& colorDepth) final
				{
					width = Display.width();
					height = Display.height();
					colorDepth = 16; // 16 bit color.
				}

			public:// Buffer drawing interface.
				void Pixel(const color_fraction16_t color, const int16_t x, const int16_t y)
				{
					Display.drawPixel(x, y, GetNativeColor(color));
				}

				void Line(const color_fraction16_t color, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2)
				{
					if (x1 == x2)
					{
						const int16_t y_start = y1 < y2 ? y1 : y2;
						const int16_t y_end = y1 > y2 ? y1 : y2;
						Display.drawFastVLine(x1, y_start, y_end - y_start + 1, GetNativeColor(color));
					}
					else if (y1 == y2)
					{
						const int16_t x_start = x1 < x2 ? x1 : x2;
						const int16_t x_end = x1 > x2 ? x1 : x2;
						Display.drawFastHLine(x_start, y1, x_end - x_start + 1, GetNativeColor(color));
					}
					else
					{
						Display.drawLine(x1, y1, x2, y2, GetNativeColor(color));
					}
				}

				void TriangleFill(const color_fraction16_t color, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2, const int16_t x3, const int16_t y3)
				{
					Display.fillTriangle(x1, y1, x2, y2, x3, y3, GetNativeColor(color));
				}

				void RectangleFill(const color_fraction16_t color, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2)
				{
					Display.fillRect(x1, y1, x2 - x1 + 1, y2 - y1 + 1, GetNativeColor(color));
				}

			protected:
				//bool StartScreen() final
				//{
				//	Display.begin();
				//	Display.setSPISpeed(MinValue(uint32_t(20000000), uint32_t(F_CPU / 1)));

				//	return true;
				//}

				//void OnDrawStart() final
				//{
				//	if (BfiEnabled)
				//	{
				//		// Blank screen before clearing buffer and starting drawing.
				//		Display.sendCommand(SSD1331_CMD_DISPLAYALLOFF);
				//	}
				//	digitalWrite(3, 0);
				//}

				//void OnDrawEnd() final
				//{
				//	if (BfiEnabled)
				//	{
				//		// Show drawn content.
				//		Display.sendCommand(SSD1331_CMD_NORMALDISPLAY);
				//	}
				//	digitalWrite(3, HIGH);
				//}

			private:
				static constexpr uint16_t GetNativeColor(const color_fraction16_t shaderColor)
				{
					return uint16_t(Fraction::Scale(shaderColor.r, uint8_t(31))) << 11
						| Fraction::Scale(shaderColor.g, uint8_t(63)) << 5
						| Fraction::Scale(shaderColor.b, uint8_t(31));
				}

			};
		}
	}
}
#endif
#endif