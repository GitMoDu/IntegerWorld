/*
* Integer World Demo Scene for EGFX engine (https://github.com/GitMoDu/EGFX).
* Includes configurations for multiple screen drivers (uncomment driver).
*/

#define DEBUG
#define SERIAL_BAUD_RATE 115200


//#define USE_DYNAMIC_FRAME_BUFFER // Enable dynamic allocation of framebuffer.
#define GRAPHICS_ENGINE_MEASURE // Enable engine measuring and logging.
#define GRAPHICS_ENGINE_DISPLAY_FPS // Enable display FPS counter.

#define INTEGER_WORLD_PERFORMANCE_LOG // Enable engine render status logging.
#define INTEGER_WORLD_PERFORMANCE_DEBUG // Enable engine debug level logging..


// Preset of SPI pin definitions for various platforms.
#if defined(STM32H7xx) 
#define TFT_DC		PE13
#define TFT_RST     UINT8_MAX
#define TFT_CS		PE11
#elif defined(STM32F4)
#define TFT_CS		PA15
#define TFT_DC		PB9
#define TFT_RST     PB8
#elif defined(ARDUINO_ARCH_STM32F1) || defined(STM32F1)
#define TFT_CS		7
#define TFT_DC		10
#define TFT_RST     11
#elif defined(ARDUINO_ARCH_AVR)
#define TFT_CS		10
#define TFT_DC		9
#define TFT_RST		8
#elif defined(ARDUINO_ARCH_ESP32)
#define TFT_CS		19
#define TFT_DC		20
#define TFT_RST		21
#elif defined(ARDUINO_SEEED_XIAO_RP2350)
#define TFT_CS		D3
#define TFT_DC		D7
#define TFT_RST		D6
#elif defined(ARDUINO_ARCH_RP2040)
#define TFT_CS		19
#define TFT_DC		20
#define TFT_RST		21
#elif defined(ARDUINO_ARCH_NRF52)
#define TFT_CS		D3
#define TFT_DC		D7
#define TFT_RST		D6
#endif

#define _TASK_OO_CALLBACKS
#include <TScheduler.hpp>

#include <ArduinoGraphicsDrivers.h>
#include <ArduinoGraphicsEngineTask.h>
#include <ArduinoGraphicsDrawers.h>

#include <IntegerWorld.h>
#include <IntegerWorldTasks.h>
#include <IntegerWorldEgfx.h>

#include "AnimatedDemoScene.h"
#include "DemoSceneAssets.h"


// Process scheduler.
TS::Scheduler SchedulerBase{};

// Forward declare the used communications hardware.
using namespace Egfx;
TwoWire& WireInstance(Wire);
#if defined(ARDUINO_ARCH_STM32F1) || defined(STM32F1) || defined(ARDUINO_ARCH_STM32F4) || defined(STM32F4)
Egfx::SpiType SpiInstance(3);
#else
Egfx::SpiType& SpiInstance(SPI);
#endif

// Uncomment Driver and matching framebuffer type. Drivers will have Async, DMA, and RTOS variants, depending on the platform.
//ScreenDriverSSD1306_64x32x1_I2C ScreenDriver(WireInstance);
//ScreenDriverSSD1306_64x48x1_I2C ScreenDriver(WireInstance);
//ScreenDriverSSD1306_72x40x1_I2C ScreenDriver(WireInstance);
//ScreenDriverSSD1306_128x32x1_I2C ScreenDriver(WireInstance);
//ScreenDriverSSD1306_128x64x1_I2C_Rtos<> ScreenDriver(WireInstance);
//ScreenDriverSSD1306_128x64x1_SPI<TFT_CS, TFT_DC, TFT_RST> ScreenDriver(SpiInstance);
//ScreenDriverSH1106_128x64x1_SPI<TFT_CS, TFT_DC, TFT_RST> ScreenDriver(SpiInstance);
//using FramebufferType = BinaryFrameBuffer<ScreenDriver.ScreenWidth, ScreenDriver.ScreenHeight>;

//ScreenDriverSSD1331_96x64x8_SPI_Dma<TFT_CS, TFT_DC, TFT_RST> ScreenDriver(SpiInstance);
//using FramebufferType = Color8FrameBuffer<ScreenDriver.ScreenWidth, ScreenDriver.ScreenHeight>;

//ScreenDriverSSD1331_96x64x16_SPI_Dma<TFT_CS, TFT_DC, TFT_RST> ScreenDriver(SpiInstance);
//ScreenDriverSSD1351_128x128x16_SPI_Dma<TFT_CS, TFT_DC, TFT_RST> ScreenDriver(SpiInstance);
//ScreenDriverST7789_240x240x16_SPI_Dma<TFT_CS, TFT_DC, TFT_RST> ScreenDriver(SpiInstance);
//ScreenDriverST7735S_80x160x16_SPI<TFT_CS, TFT_DC, TFT_RST> ScreenDriver(SpiInstance);
//using FramebufferType = Color16FrameBuffer<ScreenDriver.ScreenWidth, ScreenDriver.ScreenHeight>;


// In-memory frame-buffer.
#if defined(USE_DYNAMIC_FRAME_BUFFER)
uint8_t* Buffer = nullptr;
#else
uint8_t Buffer[FramebufferType::BufferSize]{};
#endif
FramebufferType FrameBuffer(Buffer);

// Graphics engine task and drawer wrapper.
#if defined(ARDUINO_ARCH_AVR)
static constexpr uint32_t FramePeriod = 33333 - 1;
#else
static constexpr uint32_t FramePeriod = 16666 - 1;
#endif
Egfx::GraphicsEngineTask GraphicsEngine(&SchedulerBase, &FrameBuffer, &ScreenDriver, FramePeriod);
Egfx::MultiDrawerWrapper<2> DrawerWrapper{};

// Optional FPS drawer with shaded font.
#if defined(GRAPHICS_ENGINE_DISPLAY_FPS)
using FpsLayout = Egfx::LayoutElement<0, 0, FramebufferType::FrameWidth, FramebufferType::FrameHeight>;
Egfx::DisplayFpsDrawer<FpsLayout,
	Egfx::SpriteShader::ColorShader<Egfx::SpriteFont3x5Renderer>,
	Egfx::FpsDrawerPosition::TopRight>
	FpsDrawer(&GraphicsEngine);
#endif

// Optional EGFX Graphics engine log task.
#if defined(GRAPHICS_ENGINE_MEASURE)
EngineLogTask<2000> EngineLog(SchedulerBase, GraphicsEngine);
#endif

// Integer World output Surface for the EGFX graphics engine.
IntegerWorld::EgfxSurface::FramebufferSurface Surface{};

// Integer World engine renderer.
IntegerWorld::EngineRenderTask<AnimatedDemoScene::ObjectsCount, AnimatedDemoScene::MaxDrawCallCount> EngineRenderer(SchedulerBase, Surface);

// Demo scene and animator.
AnimatedDemoScene DemoScene(SchedulerBase);

// Optional Integer World performance log task.
#if defined(INTEGER_WORLD_PERFORMANCE_LOG)
IntegerWorld::PerformanceLogTask<2000> IntegerWorldEngineLog(SchedulerBase, EngineRenderer);
#endif

void halt()
{
#if defined(USE_DYNAMIC_FRAME_BUFFER)
	delete[] Buffer;
#endif
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
		;;
	delay(1000);
#endif

#if defined(USE_DYNAMIC_FRAME_BUFFER)
	// Allocate memory and set frame buffer.
	Buffer = new uint8_t[FramebufferType::BufferSize]{};
	FrameBuffer.SetBuffer(Buffer);
#endif

	// Initialize comms hardware.
	WireInstance.begin();
	SpiInstance.begin();

	// Optional callback for RTOS driver variants.
	GraphicsEngine.SetBufferTaskCallback(BufferTaskCallback);

	// Framebuffer can be inverted at any time.
	GraphicsEngine.SetInverted(false);

	// Set the Display Sync Type.
	GraphicsEngine.SetSyncType(DisplaySyncType::Vrr);

	// Start EGFX graphics engine.
	if (!GraphicsEngine.Start())
	{
		halt();
	}

	// Attach the surface layer to renderer.
	if (!DrawerWrapper.AddDrawer(Surface)) { halt(); }

	// Add FPS drawer if enabled.
#if defined(GRAPHICS_ENGINE_DISPLAY_FPS)
	if (!DrawerWrapper.AddDrawer(FpsDrawer)) { halt(); }
#endif

	// Set the drawer wrapper for the graphics engine.
	GraphicsEngine.SetDrawer(&DrawerWrapper);

	// Start Internet World Engine renderer.
	EngineRenderer.Start();

	// EGFX Surface can be windowed.
	//Surface.OffsetX = 16;
	//Surface.SizeX = 64;

	// Start the scene with the visible dimensions.
	int16_t width;
	int16_t height;
	uint8_t colorDepth;
	Surface.GetSurfaceDimensions(width, height, colorDepth);
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
	Surface.PrintName(Serial);
	Serial.println();
#endif
}

void loop()
{
	SchedulerBase.execute();
}

#if defined(ARDUINO_ARCH_NRF52)
void BufferTaskCallback()
{
	GraphicsEngine.BufferTaskCallback(nullptr);
}
#else
void BufferTaskCallback(void* parameter)
{
	GraphicsEngine.BufferTaskCallback(parameter);
}
#endif