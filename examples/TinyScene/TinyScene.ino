/*
* Direct Draw Tiny Scene.
* Includes configurations for multiple screens (uncomment screen driver).
* Drivers draw directly to screen (no framebuffer).
* Tiny scene that fits in an AVR328 (Arduino OG): 2 edge objects and no lights.
*/

#define DEBUG
#define SERIAL_BAUD_RATE 115200

// Enable engine measuring and logging.
#define INTEGER_WORLD_PERFORMANCE_LOG

#define _TASK_OO_CALLBACKS
#include <TScheduler.hpp>

#include <IntegerWorld.h>
#include <IntegerWorldTasks.h>
#include <IntegerWorldOutputs.h>
#include "AnimatedTinyScene.h"


// Forward declare the used communications hardware.
#if defined(ARDUINO_MAPLE_MINI)
#define TFT_CS		7
#define TFT_DC		10
#define TFT_RST     11
#define TFT_BKL     -1
#elif defined(ARDUINO_ARCH_RP2040)
#define TFT_CS		D3
#define TFT_DC		D7
#define TFT_RST     D6
#define TFT_BKL     -1
#else
#define TFT_CS		10
#define TFT_DC		9
#define TFT_RST     8
#define TFT_BKL     -1
#endif
#if defined(ARDUINO_MAPLE_MINI)
SPIClass SpiInstance(1);
#elif defined(ARDUINO_ARCH_RP2040)
SPIClassRP2040& SpiInstance(SPI);
#else
SPIClass& SpiInstance(SPI);
#endif


// Uncomment output target to use.
//IntegerWorld::Lexus2k::Ssd1306I2c::DirectDrawSurface Driver{}; // https://github.com/lexus2k/ssd1306
//IntegerWorld::Adafruit::St7789::DirectDrawSurface Driver(SpiInstance, TFT_CS, TFT_DC, TFT_RST, TFT_BKL); // https://github.com/adafruit/Adafruit-ST7735-Library
//IntegerWorld::Adafruit::St7735::DirectDrawSurface Driver(IntegerWorld::Adafruit::St7735::VariantEnum::Mini160x80Plugin, SpiInstance, TFT_CS, TFT_DC, TFT_RST, TFT_BKL); // https://github.com/adafruit/Adafruit-ST7735-Library
IntegerWorld::Adafruit::Sdd1331::DirectDrawSurface Driver(SpiInstance, TFT_CS, TFT_DC, TFT_RST); // https://github.com/adafruit/Adafruit-SSD1331-OLED-Driver-Library-for-Arduino
//IntegerWorld::Adafruit::Sdd1306I2c::FramebufferSurface Driver(Wire); // https://github.com/adafruit/Adafruit_SSD1306
//IntegerWorld::Adafruit::Sdd1306Spi::FramebufferSurface Driver(SpiInstance, TFT_CS, TFT_DC, TFT_RST); // https://github.com/adafruit/Adafruit_SSD1306
//IntegerWorld::Adafruit::Sh1106Spi::FramebufferSurface Driver(SpiInstance, TFT_CS, TFT_DC, TFT_RST); // https://github.com/adafruit/Adafruit_SH110X

//IntegerWorld::MockOutput::OutputSurface<128, 128> Driver{}; // Mock output.

// Task scheduler.
TS::Scheduler SchedulerBase;

// World engine renderer and drawer.
IntegerWorld::EngineRenderTask<
	AnimatedTinyScene::RenderObjectCount>
	EngineRenderer(SchedulerBase, &Driver);

// Scene manager and animator task.
AnimatedTinyScene Scene(SchedulerBase);

#if defined(INTEGER_WORLD_PERFORMANCE_LOG)
IntegerWorld::PerformanceLogTask<2000> IntegerWorldEngineLog(SchedulerBase, EngineRenderer);
#endif

void halt()
{
#if defined(DEBUG)
	Serial.println(F("Screen Setup Failed."));
#endif
	while (true)
		;
}

void setup()
{
#if defined(DEBUG)
	Serial.begin(SERIAL_BAUD_RATE);
	while (!Serial)
		;
	delay(1000);
#endif

	int16_t width, height;
	uint8_t colorDepth;
	Driver.GetSurfaceDimensions(width, height, colorDepth);

	EngineRenderer.Start();
	Scene.Start(EngineRenderer, colorDepth);

#if defined(DEBUG)
	Serial.println(F("Integer World 3D Tiny Scenee"));
	Serial.print(F("Display "));
	Serial.print(width);
	Serial.print('x');
	Serial.print(height);
	Serial.print(' ');
	Serial.print(colorDepth);
	Serial.println(F(" bit color"));
	Serial.print(F("Using "));
	Driver.PrintName(Serial);
	Serial.println();
#endif
}

void loop()
{
	SchedulerBase.execute();
}