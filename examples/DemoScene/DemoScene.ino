/*
* Integer World Demo Scene.
* Includes configurations for multiple screens (uncomment screen driver).

*/

#define DEBUG
#define SERIAL_BAUD_RATE 115200

#define INTEGER_WORLD_PERFORMANCE_LOG // Enable engine measuring and logging.

#define _TASK_OO_CALLBACKS
#include <TScheduler.hpp>

#include <IntegerWorld.h>
#include <IntegerWorldTasks.h>
#include <IntegerWorldOutputs.h>
#include "AnimatedDemoScene.h"
#include "DemoSceneAssets.h"


// Forward declare the used communications hardware.
#if defined(ARDUINO_MAPLE_MINI)
SPIClass SpiInstance(1);
#elif defined(ARDUINO_ARCH_RP2040)
SPIClassRP2040& SpiInstance(SPI);
#else
SPIClass& SpiInstance(SPI);
#endif

#if defined(ARDUINO_MAPLE_MINI)
#define TFT_CS		7
#define TFT_DC		10
#define TFT_RST     11
#define TFT_BKL     -1
#elif defined(ARDUINO_ARCH_RP2040)
#define TFT_CS		D3
#define TFT_DC		D7
#define TFT_RST     D6
#define TFT_BKL     -1//D2
#else
#define TFT_CS		10
#define TFT_DC		9
#define TFT_RST     8
#define TFT_BKL     -1
#endif



// Uncomment output target driver to use.
//IntegerWorld::Lexus2k::Ssd1306I2c::DirectDrawSurface Driver{}; // https://github.com/lexus2k/ssd1306
//IntegerWorld::Adafruit::St7789::DirectDrawSurface Driver(SpiInstance, TFT_CS, TFT_DC, TFT_RST, TFT_BKL); // https://github.com/adafruit/Adafruit-ST7735-Library
//IntegerWorld::Adafruit::St7735::DirectDrawSurface Driver(IntegerWorld::Adafruit::St7735::VariantEnum::Mini160x80Plugin, SpiInstance, TFT_CS, TFT_DC, TFT_RST, TFT_BKL); // https://github.com/adafruit/Adafruit-ST7735-Library
IntegerWorld::Adafruit::Sdd1331::DirectDrawSurface Driver(SpiInstance, TFT_CS, TFT_DC, TFT_RST); // https://github.com/adafruit/Adafruit-SSD1331-OLED-Driver-Library-for-Arduino
//IntegerWorld::Adafruit::Sdd1306I2c::FramebufferSurface Driver(Wire); // https://github.com/adafruit/Adafruit_SSD1306
//IntegerWorld::Adafruit::Sdd1306Spi::FramebufferSurface Driver(SpiInstance, TFT_CS, TFT_DC, TFT_RST); // https://github.com/adafruit/Adafruit_SSD1306
//IntegerWorld::Adafruit::Sh1106Spi::FramebufferSurface Driver(SpiInstance, TFT_CS, TFT_DC, TFT_RST); // https://github.com/adafruit/Adafruit_SH110X

//IntegerWorld::MockOutput::OutputSurface<128, 128> Driver{}; // Mock output.


// Process scheduler.
TS::Scheduler SchedulerBase;

// World engine renderer and drawer.
IntegerWorld::EngineRenderTask<AnimatedDemoScene::ObjectsCount> EngineRenderer(SchedulerBase, &Driver);

// Objects animator for the world.
AnimatedDemoScene DemoScene(SchedulerBase);

#if defined(INTEGER_WORLD_PERFORMANCE_LOG)
IntegerWorld::PerformanceLogTask<2000> IntegerWorldEngineLog(SchedulerBase, EngineRenderer);
#endif

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

	DemoScene.Start(EngineRenderer, width, height);

#if defined(DEBUG)
	Serial.println(F("Integer World 3D Demo Scene"));
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