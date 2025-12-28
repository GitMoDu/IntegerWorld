/*
* Integer World Demo Scene for EGFX engine (https://github.com/GitMoDu/EGFX).
* Includes configurations for multiple screen drivers (uncomment driver).
*/

#define DEBUG
#define SERIAL_BAUD_RATE 115200

//#define GRAPHICS_ENGINE_MEASURE // Enable engine measuring and logging.
//#define EGFX_PERFORMANCE_LOG // Enable performance logging for EGFX engine.
//#define EGFX_PERFORMANCE_DEBUG // Enable performance debug logging for EGFX engine.

//#define USE_DYNAMIC_FRAME_BUFFER // Enable dynamic allocation of framebuffer.
#define USE_DOUBLE_FRAME_BUFFER // Enable double framebuffer.
#define USE_DISPLAY_FPS // Enable FPS display drawer.
//#define USE_LOG_FPS // Enable serial logging of FPS and engine performance.
//#define USE_TFT_BACKLIGHT_PIN 7 // Optional TFT backlight control pin.
//#define USE_TFT_BACKLIGHT_PIN 21 // Optional TFT backlight control pin.


//#define INTEGER_WORLD_PERFORMANCE_LOG // Enable engine render status logging.
//#define INTEGER_WORLD_PERFORMANCE_DEBUG // Enable engine debug level logging.


#define _TASK_OO_CALLBACKS
#include <TScheduler.hpp>


#include <IntegerWorld.h>
#include <IntegerWorldTasks.h>
#include <IntegerWorldEgfx.h>

#include <IntegerWorldExperimental.h>

#include "DisplayConfiguration.h"

//#define USE_TFT_BACKLIGHT_PIN DisplayPins::TFT_BACKLIGHT

#include "ChristmasScene.h"



// Process scheduler.
TS::Scheduler SchedulerBase{};

// Display frame period in microseconds. Target 20 FPS.
static constexpr uint32_t FramePeriod = 50000 - 1;

// EGFX graphics display engine, with optional logging task.
Egfx::PremadeGraphicsDisplay::TemplateEngine<FramePeriod,
	FramebufferType, ScreenDriverType> DisplayEngine(SchedulerBase, DisplayCommsInstance);

// Integer World output Surface for the EGFX graphics engine.
IntegerWorld::EgfxSurface::FramebufferTemplate<FramebufferType> Surface(DisplayEngine.Framebuffer);

// Integer World engine renderer.
IntegerWorld::EngineRenderTask<ChristmasScene::ObjectsCount, ChristmasScene::MaxDrawCallCount, 64> EngineRenderer(SchedulerBase, Surface);

// Demo scene and animator.
ChristmasScene Scene(EngineRenderer);

// Optional Integer World performance log task.
#if defined(INTEGER_WORLD_PERFORMANCE_LOG)
IntegerWorld::PerformanceLogTask<2000> IntegerWorldEngineLog(SchedulerBase, EngineRenderer);
#endif

void halt()
{
	DisplayEngine.OnSetupFail();

#if defined(DEBUG)
	Serial.println(F("Setup Failed."));
#endif
	while (true)
		;
}



void setup()
{
#if defined(DEBUG)
	Serial.begin(SERIAL_BAUD_RATE);
	//while (!Serial)
		//;;
	delay(1000);
#endif

	if (DisplayPins::BACKLIGHT != UINT8_MAX)
	{
		pinMode(DisplayPins::BACKLIGHT, OUTPUT);
		digitalWrite(DisplayPins::BACKLIGHT, HIGH);
	}

	// Start EGFX graphics engine.
	if (!DisplayEngine.Start(BufferTaskCallback, Egfx::DisplaySyncType::VSync))
	{
		halt();
	}
	Scene.PrintLightsPositions();
	//halt();
// Attach the surface layer to renderer.
	DisplayEngine.SetDrawLayer(Surface);

	// Start Internet World Engine renderer.
	EngineRenderer.Start();

#if defined(INTEGER_WORLD_PERFORMANCE_LOG)
	IntegerWorldEngineLog.Start();
#endif

	// EGFX Surface can be windowed.
	const uint8_t margin = 0;
	Surface.OffsetX = margin;
	Surface.OffsetY = margin;
	Surface.SizeX = FramebufferType::FrameWidth - margin;
	Surface.SizeY = FramebufferType::FrameHeight - margin;

	// Start the scene with the visible dimensions.
	int16_t width, height;
	uint8_t colorDepth;
	Surface.GetSurfaceDimensions(width, height, colorDepth);

	if (!Scene.Start(EngineRenderer, width, height))
	{
#if defined(DEBUG)
		Serial.println(F("Scene failed to start."));
#endif
		halt();
	}

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
	Serial.print(F("EGFX Framebuffer"));
	Serial.println();
#endif	
}

void loop()
{
	SchedulerBase.execute();
}


void BufferTaskCallback(void* parameter)
{
	DisplayEngine.BufferTaskCallback(parameter);
}




