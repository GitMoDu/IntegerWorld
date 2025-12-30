/*
* Integer World Demo Scene for EGFX engine (https://github.com/GitMoDu/EGFX).
* Configure screen in DisplayConfiguration.h.
* Options:
* #define EGFX_PERFORMANCE_LOG // Enable performance logging for EGFX engine.
* #define EGFX_PERFORMANCE_DEBUG // Enable performance debug logging for EGFX engine.
* #define USE_DYNAMIC_FRAME_BUFFER // Enable dynamic allocation of framebuffer.
* #define USE_DOUBLE_FRAME_BUFFER // Enable double framebuffer.
* #define USE_DISPLAY_FPS // Enable FPS display counter.
* #define USE_LOG_FPS // Enable serial logging of FPS and engine performance.
* #define INTEGER_WORLD_PERFORMANCE_LOG // Enable engine render status logging.
* #define INTEGER_WORLD_PERFORMANCE_DEBUG // Enable engine debug level logging.
* #define INTEGER_WORLD_TEXTURED_CUBE_DEMO // Use textured cube object in the demo scene instead of colored cube.
* #define INTEGER_WORLD_TEXTURED_CUBE_HIGH_QUALITY // Textured cube object with perspective correct and accurate texture mapping.
* #define INTEGER_WORLD_LIGHTS_SHADER_DEBUG // Enable light component toggles in the scene lights shader.
* #define INTEGER_WORLD_FRUSTUM_DEBUG // Enable engine frustum visualization in scene.
* 
*/

#define DEBUG
#define SERIAL_BAUD_RATE 115200

//#define EGFX_PERFORMANCE_LOG // Enable performance logging for EGFX engine.
//#define EGFX_PERFORMANCE_DEBUG // Enable performance debug logging for EGFX engine.

#define USE_DYNAMIC_FRAME_BUFFER // Enable dynamic allocation of framebuffer.
#define USE_DOUBLE_FRAME_BUFFER // Enable double framebuffer.
//#define USE_DISPLAY_FPS // Enable FPS display counter.
//#define USE_LOG_FPS // Enable serial logging of FPS and engine performance.

#define INTEGER_WORLD_PERFORMANCE_LOG // Enable engine render status logging.
#define INTEGER_WORLD_PERFORMANCE_DEBUG // Enable engine debug level logging.

#define INTEGER_WORLD_TEXTURED_CUBE_DEMO // Use textured cube object in the demo scene instead of colored cube.
#define INTEGER_WORLD_TEXTURED_CUBE_HIGH_QUALITY // Textured cube object with perspective correct and accurate texture mapping.


#define _TASK_OO_CALLBACKS
#include <TScheduler.hpp>


#include <IntegerWorld.h>
#include <IntegerWorldTasks.h>
#include <IntegerWorldEgfx.h>
#include <IntegerWorldExperimental.h>

#include "DisplayConfiguration.h" // Screen and framebuffer configuration.


// Process scheduler.
TS::Scheduler SchedulerBase{};

// Display frame period in microseconds.
static constexpr uint32_t FramePeriod = 16666 - 1;

// EGFX graphics display engine, with optional logging task.
Egfx::PremadeGraphicsDisplay::TemplateEngine<FramePeriod,
	FramebufferType, ScreenDriverType> DisplayEngine(SchedulerBase, DisplayCommsInstance);

// Integer World output Surface for the EGFX graphics engine.
IntegerWorld::EgfxSurface::FramebufferTemplate<FramebufferType> Surface(DisplayEngine.Framebuffer);

// Integer World engine renderer.
IntegerWorld::EngineRenderTask<AnimatedDemoScene::ObjectsCount, AnimatedDemoScene::MaxDrawCallCount, 64> EngineRenderer(SchedulerBase, Surface);

// Demo scene and animator.
AnimatedDemoScene DemoScene(SchedulerBase);

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
	while (!Serial)
		;;
	delay(1000);
#endif

	// Start EGFX graphics engine.
	if (!DisplayEngine.Start(BufferTaskCallback, Egfx::DisplaySyncType::Vrr))
	{
		halt();
	}

	// Attach the surface layer to renderer.
	DisplayEngine.SetDrawLayer(Surface);

	// Enable backlight pin if defined.
	if (DisplayPins::BACKLIGHT != UINT8_MAX)
	{
		pinMode(DisplayPins::BACKLIGHT, OUTPUT);
		digitalWrite(DisplayPins::BACKLIGHT, HIGH);
	}

	// Start Internet World Engine renderer.
	EngineRenderer.Start();

#if defined(INTEGER_WORLD_PERFORMANCE_LOG) // Start performance logging.
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

