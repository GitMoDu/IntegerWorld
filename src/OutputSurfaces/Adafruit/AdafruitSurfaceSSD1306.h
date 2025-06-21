#ifndef _INTEGER_WORLD_ADAFRUIT_SURFACE_SSD1306_h
#define _INTEGER_WORLD_ADAFRUIT_SURFACE_SSD1306_h

#include "AdafruitSurface.h"
#if defined(ARDUINO_ARCH_STM32F1)
#include <Adafruit_SSD1306_STM32.h>
#else
#include <Adafruit_SSD1306.h>
#endif


namespace IntegerWorld
{
	namespace Adafruit
	{
		namespace Sdd1306I2c
		{
			class FramebufferSurface : public TemplateAdafruitFramebufferSurface<Adafruit_SSD1306, AdafruitMonochromeColorConverter>
			{
			private:
				using Base = TemplateAdafruitFramebufferSurface<Adafruit_SSD1306, AdafruitMonochromeColorConverter>;

			public:
				static constexpr uint8_t DisplayWidth = 128;
				static constexpr uint8_t DisplayHeight = 64;

			private:
				Adafruit_SSD1306 Display;
				const uint8_t Address;

			protected:
				bool StartScreen() final
				{
#if defined(ARDUINO_ARCH_STM32F1)
					Display.begin(SSD1306_SWITCHCAPVCC, Address);
					return true;
#else
					return Display.begin(SSD1306_SWITCHCAPVCC, Address);
#endif
				}

			public:
				FramebufferSurface(TwoWire& wire, const uint8_t address = 0x3C, const int8_t rst = -1, const uint32_t pushPauseDuration = 10000)
					: Base(Display, pushPauseDuration)
#if defined(ARDUINO_ARCH_STM32F1)
					, Display(DisplayWidth, DisplayHeight, rst)
#else
					, Display(DisplayWidth, DisplayHeight, &wire, rst)
#endif
					, Address(address)
				{
				}

				void PrintName(Print& serial)
				{
					serial.print(F("SSD1306 SPI Adafruit"));
				}
			};
		}

		namespace Sdd1306Spi
		{
			class FramebufferSurface : public TemplateAdafruitFramebufferSurface<Adafruit_SSD1306, AdafruitMonochromeColorConverter>
			{
			private:
				using Base = TemplateAdafruitFramebufferSurface<Adafruit_SSD1306, AdafruitMonochromeColorConverter>;

			public:
				static constexpr uint8_t DisplayWidth = 128;
				static constexpr uint8_t DisplayHeight = 64;

			private:
				Adafruit_SSD1306 Display;

			protected:
				bool StartScreen() final
				{
#if defined(ARDUINO_ARCH_STM32F1)
					Display.begin(0, true);
					return true;
#else
					return Display.begin(0, true);
#endif
				}


			public:
				FramebufferSurface(SpiType& spi, int8_t cs, int8_t dc, int8_t rst, const uint32_t pushPauseDuration = 10000)
					: Base(Display, pushPauseDuration)
#if defined(ARDUINO_ARCH_STM32F1)
					, Display(dc, rst, cs)
#else
					, Display(DisplayWidth, DisplayHeight, &spi, dc, rst, cs)
#endif
				{
				}

				void PrintName(Print& serial)
				{
					serial.print(F("SSD1306 SPI Adafruit"));
				}
			};
		}
	}
}
#endif

