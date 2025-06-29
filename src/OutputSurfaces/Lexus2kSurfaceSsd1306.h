#ifndef _INTEGER_WORLD_LEXUS2K_SSD1306_SURFACE_h
#define _INTEGER_WORLD_LEXUS2K_SSD1306_SURFACE_h

#if defined(ARDUINO)
#include "../Framework/Interface.h"
#include <Wire.h>
#include <ssd1306.h>

namespace IntegerWorld
{
	namespace Lexus2k
	{
		namespace Ssd1306I2c
		{
			/// <summary>
			/// IOutputSurface wrapper for direct drawing on SSD1306 (over I2C)
			/// using Lexus2k (https://github.com/lexus2k/ssd1306) library.
			/// </summary>
			class DirectDrawSurface : public IOutputSurface
			{
			private:
				enum class StateEnum : uint8_t
				{
					Disabled,
					WaitingForDraw,
					DrawLocked
				};

				static constexpr ufraction16_t ColorThreshold = UFRACTION16_1X / 2;

			public:
				uint32_t TargetPeriod;

			private:
				uint32_t LastPush = 0;
				uint16_t LastColor = 0;
				StateEnum State = StateEnum::Disabled;

			public:
				DirectDrawSurface(const uint32_t targetPeriod = 70000)
					: IOutputSurface()
					, TargetPeriod(targetPeriod)
				{
				}

				void PrintName(Print& serial)
				{
					serial.print(F("SSD1306 I2C Lexus2k"));
				}

			public:
				bool StartSurface() final
				{
					ssd1306_128x64_i2c_init();
					ssd1306_clearScreen();
#if defined(ARDUINO_ARCH_AVR)
					Wire.setClock(400000);
#endif
					LastPush = micros() - TargetPeriod;
					State = StateEnum::WaitingForDraw;

					return true;
				}

				void StopSurface() final
				{
					ssd1306_fillScreen(0x00);
				}

			public:
				bool IsSurfaceReady() final
				{
					if (State == StateEnum::WaitingForDraw
						&& ((micros() - LastPush) > TargetPeriod))
					{
						LastPush = micros();
						ssd1306_clearScreen();
						LastColor = 0;
						SetColor(UINT16_MAX);
						State = StateEnum::DrawLocked;
					}

					return State == StateEnum::DrawLocked;
				}

				void FlipSurface() final
				{
					switch (State)
					{
					case StateEnum::DrawLocked:
						State = StateEnum::WaitingForDraw;
						break;
					default:
						break;
					}
				}

				void GetSurfaceDimensions(int16_t& width, int16_t& height, uint8_t& colorDepth) final
				{
					width = 128;
					height = 64;
					colorDepth = 1;
				}

				void Pixel(const color_fraction16_t color, const int16_t x, const int16_t y) final
				{
					SetColor(GetNativeColor(color));
					ssd1306_putPixel(x, y);
				}

				void Line(const color_fraction16_t color, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2) final
				{
					SetColor(GetNativeColor(color));
					ssd1306_drawLine(x1, y1, x2, y2);
				}

				void TriangleFill(const color_fraction16_t color, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2, const int16_t x3, const int16_t y3) final
				{
					//TODO:
				}

				void RectangleFill(const color_fraction16_t color, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2) final
				{
					SetColor(GetNativeColor(color));
					ssd1306_fillRect(x1, y1, x2, y2);
				}

			private:
				void SetColor(const uint16_t color)
				{
					if (color)
					{
						LastColor = color;
						ssd1306_setColor(LastColor);
					}
				}

				static constexpr uint16_t GetNativeColor(const color_fraction16_t shaderColor)
				{
					return (shaderColor.r >= ColorThreshold || shaderColor.g >= ColorThreshold || shaderColor.b >= ColorThreshold) * UINT16_MAX;
				}
			};
		}
	}
}
#endif
#endif