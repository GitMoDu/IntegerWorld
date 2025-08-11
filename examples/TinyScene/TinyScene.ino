/*
* Integer World Tiny Scene that fits in an AVR328 (Arduino OG).
* Scene has no lights or scene shader.
* Includes configurations for multiple screen drivers (uncomment driver and required include).
* With DEMO_SCENE_EDGE_OBJECT: 2 edge objects. Without: 2 mesh objects.
*/

#define DEBUG
#define SERIAL_BAUD_RATE 115200


//#define DEMO_SCENE_EDGE_OBJECT // Enable for monochrome and low RAM devices.
//#define INTEGER_WORLD_PERFORMANCE_LOG // Enable engine render status logging.
//#define INTEGER_WORLD_PERFORMANCE_DEBUG // Enable engine debug level status measuring.


#define _TASK_OO_CALLBACKS
#include <TScheduler.hpp>

#include <SPI.h>
#include <Wire.h>

#include <IntegerWorld.h>
#include <IntegerWorldTasks.h>
#include <IntegerWorldOutputs.h>
#include "AnimatedTinyScene.h"

// Include used output drivers. 
// Each driver has its own dependencies, so you may need to install them.
// Some drivers may not be included at the same time, as they may conflict, such as SumotoySsd1331 and Adafruit's.
//#include <IntegerWorldAdafruitSsd1306.h>
//#include <IntegerWorldAdafruitSh1106.h>
//#include <IntegerWorldAdafruitSsd1331.h>
//#include <IntegerWorldAdafruitSt77XX.h>
//#include <IntegerWorldLexus2kSsd1306.h>
//#include <IntegerWorldSumotoySsd1331.h>


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
#elif defined(ARDUINO_ARCH_ESP32)
#define TFT_CS		5
#define TFT_DC		16
#define TFT_RST     23
#else
#define TFT_CS		10
#define TFT_DC		9
#define TFT_RST     8
#endif

#if defined(ARDUINO_ARCH_STM32F1) || defined(STM32F1) || defined(ARDUINO_ARCH_STM32F4) || defined(STM32F4)
SPIClass SpiInstance(1);
#elif defined(ARDUINO_ARCH_RP2040)
SPIClassRP2040& SpiInstance(SPI);
#else
SPIClass& SpiInstance(SPI);
#endif


// Uncomment output driver to use.
//IntegerWorld::Adafruit::Ssd1306I2c::FramebufferSurface Driver(Wire); // https://github.com/adafruit/Adafruit_SSD1306
//IntegerWorld::Adafruit::Ssd1306Spi::FramebufferSurface Driver(SpiInstance, TFT_CS, TFT_DC, TFT_RST); // https://github.com/adafruit/Adafruit_SSD1306
//IntegerWorld::Adafruit::Sh1106Spi::FramebufferSurface Driver(SpiInstance, TFT_CS, TFT_DC, TFT_RST); // https://github.com/adafruit/Adafruit_SH110X
//IntegerWorld::Adafruit::Ssd1331::DirectDrawSurface Driver(SpiInstance, TFT_CS, TFT_DC, TFT_RST); // https://github.com/adafruit/Adafruit-SSD1331-OLED-Driver-Library-for-Arduino
//IntegerWorld::Adafruit::St7735::DirectDrawSurface Driver(IntegerWorld::Adafruit::St7735::VariantEnum::Mini160x80Plugin, SpiInstance, TFT_CS, TFT_DC, TFT_RST, TFT_BKL); // https://github.com/adafruit/Adafruit-ST7735-Library
//IntegerWorld::Adafruit::St7789::DirectDrawSurface Driver(SpiInstance, TFT_CS, TFT_DC, TFT_RST); // https://github.com/adafruit/Adafruit-ST7735-Library

//IntegerWorld::Lexus2k::Ssd1306I2c::DirectDrawSurface Driver{}; // https://github.com/lexus2k/ssd1306

//IntegerWorld::Sumotoy::Ssd1331::DirectDrawSurface Driver(TFT_CS, TFT_DC, TFT_RST); // https://github.com/sumotoy/SSD_13XX

//IntegerWorld::MockOutput::OutputSurface<128, 128> Driver{}; // Mock output.

// Task scheduler.
TS::Scheduler SchedulerBase;

// World engine renderer and drawer.
IntegerWorld::EngineRenderTask<
	AnimatedTinyScene::RenderObjectCount,
	AnimatedTinyScene::MaxDrawCallCount>
	EngineRenderer(SchedulerBase, Driver);

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

	EngineRenderer.Start();

	int16_t width, height;
	uint8_t colorDepth;
	Driver.GetSurfaceDimensions(width, height, colorDepth);

	Scene.Start(EngineRenderer, width, height, colorDepth == 1);

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