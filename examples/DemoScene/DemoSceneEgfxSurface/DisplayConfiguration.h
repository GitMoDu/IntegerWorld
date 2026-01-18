#ifndef _DISPLAY_CONFIGURATION_h
#define _DISPLAY_CONFIGURATION_h

#include <EgfxScreenDrivers.h>
#include <EgfxCore.h>
#include <EgfxPlatformPresets.h>


// Configuration, based on display wiring and platform. Uncomment the desired pin configuration.
#if defined(STM32H7xx)
using DisplayConfig = Egfx::PlatformPresets::Configs::STM32H7_SPI_GENERIC;
#elif defined(STM32F4)
using DisplayConfig = Egfx::PlatformPresets::Configs::STM32F4_SPI_GENERIC;
#elif defined(ARDUINO_ARCH_STM32F1) || defined(STM32F1)
using DisplayConfig = Egfx::PlatformPresets::Configs::STM32F1_SPI_GENERIC;
#elif defined(ARDUINO_ARCH_AVR)
using DisplayConfig = Egfx::PlatformPresets::Configs::AVR_SPI_GENERIC;
#elif defined(ARDUINO_ARCH_ESP32)
using DisplayConfig = Egfx::PlatformPresets::Configs::ESP32_SPI_GENERIC;
#elif defined(ARDUINO_SEEED_XIAO_RP2350)
using DisplayConfig = Egfx::PlatformPresets::Configs::RP2350_XIAO_SPI_GENERIC;
#elif defined(ARDUINO_ARCH_RP2040)
using DisplayConfig = Egfx::PlatformPresets::Configs::RP2350_SPI_GENERIC;
#elif defined(ARDUINO_ARCH_NRF52)
using DisplayConfig = Egfx::PlatformPresets::Configs::NRF52_SPI_GENERIC;
#endif

// Display options definition. Adjust as needed.
using EgfxOptions = Egfx::DisplayOptions::TemplateOptions<
	Egfx::DisplayOptions::MirrorEnum::None,
	Egfx::DisplayOptions::RotationEnum::None,
	false,
	Egfx::DisplayOptions::AntiAliasingEnum::None
>;

// Uncomment Driver and matching Framebuffer type. Drivers will have Async, DMA, and RTOS variants, depending on the platform.
//using ScreenDriverType = Egfx::ScreenDriverSSD1306_64x32x1_I2C;
//using ScreenDriverType = Egfx::ScreenDriverSSD1306_64x48x1_I2C;
//using ScreenDriverType = Egfx::ScreenDriverSSD1306_72x40x1_I2C;
//using ScreenDriverType = Egfx::ScreenDriverSSD1306_128x32x1_I2C;
//using ScreenDriverType = Egfx::ScreenDriverSSD1306_128x64x1_I2C;
//using FramebufferType = Egfx::BinaryFramebuffer<ScreenDriverType::ScreenWidth, ScreenDriverType::ScreenHeight>;

//using ScreenDriverType = Egfx::ScreenDriverSSD1306_128x64x1_SPI<DisplayConfig::CS, DisplayConfig::DC, DisplayConfig::RESET>;
//using ScreenDriverType = Egfx::ScreenDriverSH1106_128x64x1_SPI<DisplayConfig::CS, DisplayConfig::DC, DisplayConfig::RESET>;
//using FramebufferType = Egfx::BinaryFramebuffer<ScreenDriverType::ScreenWidth, ScreenDriverType::ScreenHeight>;

//using ScreenDriverType = Egfx::ScreenDriverSSD1331_96x64x8_SPI<DisplayConfig::CS, DisplayConfig::DC, DisplayConfig::RESET>;
//using FramebufferType = Egfx::Color8Framebuffer<ScreenDriverType::ScreenWidth, ScreenDriverType::ScreenHeight>;

using ScreenDriverType = Egfx::ScreenDriverSSD1331_96x64x16_SPI<DisplayConfig::CS, DisplayConfig::DC, DisplayConfig::RESET>;
//using ScreenDriverType = Egfx::ScreenDriverSSD1351_128x128x16_SPI<DisplayConfig::CS, DisplayConfig::DC, DisplayConfig::RESET>;
//using ScreenDriverType = Egfx::ScreenDriverST7789_240x240x16_SPI<DisplayConfig::CS, DisplayConfig::DC, DisplayConfig::RESET>;
//using ScreenDriverType = Egfx::ScreenDriverST7735S_80x160x16_SPI<DisplayConfig::CS, DisplayConfig::DC, DisplayConfig::RESET>;
//using ScreenDriverType = Egfx::ScreenDriverSSD1331_96x64x16_SPI<DisplayConfig::CS, DisplayConfig::DC, DisplayConfig::RESET>;
//using ScreenDriverType = Egfx::ScreenDriverST7789_172x320x16_SPI<DisplayConfig::CS, DisplayConfig::DC, DisplayConfig::RESET>;
//using ScreenDriverType = Egfx::ScreenDriverST7789T3_240x320x16_SPI_Dma<DisplayConfig::CS, DisplayConfig::DC, DisplayConfig::RESET>;
//using ScreenDriverType = Egfx::ScreenDriverGC9107_128x128x16_SPI<DisplayConfig::CS, DisplayConfig::DC, DisplayConfig::RESET>;
using FramebufferType = Egfx::Color16Framebuffer<ScreenDriverType::ScreenWidth, ScreenDriverType::ScreenHeight, 0, EgfxOptions>;

#endif

