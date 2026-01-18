#ifndef _EGFX_EXPERIMENTAL_PRE_MADE_GRAPHICS_DISPLAY_h
#define _EGFX_EXPERIMENTAL_PRE_MADE_GRAPHICS_DISPLAY_h

#include <EgfxDrawer.h>
#include <EgfxDrawers.h>
#include <EgfxDisplayEngine.h>
#include <EgfxScreenDrivers.h>

#include "../OutputSurfaces/EgfxSurface.h"

namespace Egfx
{
	/// <summary>
	/// Ready to use EGFX graphics display engine.
	/// Handles frame buffers, screen communications, screen driver, render task and FPS logging/drawing.
	/// </summary>
	namespace PremadeGraphicsDisplay
	{
		/// <summary>
		/// Single draw layer, EGFX graphics engine for a display. 
		/// Manages frame buffers (static or dynamic), screen communications, the screen driver, render task, optional FPS logging/drawing, and drawer composition.
		/// Options (un)defined via #preprocessor directives:
		/// USE_DYNAMIC_FRAME_BUFFER - Enable dynamic allocation of the frame buffer(s) on engine start. May be required for large frame buffers on platforms with limited stack/heap memory.
		/// USE_DOUBLE_FRAME_BUFFER - Enable double buffering for the frame buffer(s). Enables rendering to one buffer while the other is being pushed to the display.
		/// USE_LOG_FPS - Enable logging of FPS and engine performance to serial output. Requires GRAPHICS_ENGINE_MEASURE to be defined?
		/// USE_DISPLAY_FPS - Enable on-screen display of FPS counter.
		/// </summary>
		/// <typeparam name="FramebufferType">Type that defines framebuffer properties (e.g., BufferSize, FrameWidth, FrameHeight) and provides the framebuffer interface used by the engine.</typeparam>
		/// <typeparam name="ScreenDriverType">Screen driver implementation type that uses the selected communications type to drive the display hardware.</typeparam>
		/// <typeparam name="TargetFramePeriod">Compile-time value that specifies the target frame period for the graphics engine (controls render/update timing).</typeparam>
		template<uint32_t TargetFramePeriod,
			typename FramebufferType,
			typename ScreenDriverType>
		class TemplateEngine
		{
		private:
#if defined(USE_DYNAMIC_FRAME_BUFFER) // In-memory frame-buffer.
			uint8_t* Buffer = nullptr;
#if defined(USE_DOUBLE_FRAME_BUFFER)
			uint8_t* AltBuffer = nullptr;
#endif
#else		// Static frame-buffer.
			uint8_t Buffer[FramebufferType::BufferSize]{};
#if defined(USE_DOUBLE_FRAME_BUFFER)
			uint8_t AltBuffer[FramebufferType::BufferSize]{};
#endif
#endif

		public:
			// Frame buffer instance.
#if defined(USE_DOUBLE_FRAME_BUFFER)
			Egfx::TemplateDoubleBufferedFramebuffer<FramebufferType> Framebuffer;
#else
			FramebufferType Framebuffer;
#endif

		private:
			// Screen comms type is based on the selected communications type.
			using ScreenCommsInterfaceType = typename ScreenDriverType::InterfaceType;

			ScreenCommsInterfaceType& ScreenComms;

			// Screen driver using specified communications hardware.
			ScreenDriverType ScreenDriver;

			// Render push task for the graphics engine.
			Egfx::DisplayEngineTask<FramebufferType, ScreenDriverType> GraphicsEngine;

		public:
			// Optional EGFX Graphics engine log task.
#if defined(USE_LOG_FPS)
			Egfx::PerformanceLogTask<2000> EngineLog;
#endif

#if defined(USE_DISPLAY_FPS) // View composition with FPS display layer.
			static constexpr uint8_t Margin = 10;
			using FpsLayout = Framework::TemplateLayout<Margin, Margin,
				FramebufferType::FrameWidth - Margin * 2, FramebufferType::FrameHeight - Margin * 2>;
			template<typename UserViewType>
			using HostViewType = Egfx::Framework::Assets::FpsDisplay::Views::CompositeWithFps<FpsLayout, UserViewType>;
#else		// Surface view layer only.
			template<typename UserViewType>
			using HostViewType = Egfx::Framework::View::CompositeView<UserViewType>;
#endif
			// Complete view layer type for the graphics engine.
			using ViewLayersType = HostViewType<IntegerWorld::EgfxSurface::FramebufferPointerTemplate<FramebufferType>>;

			// Graphics engine task and drawer wrapper for user layer and FPS drawer.
			Egfx::Framework::View::FrameAdapter<ViewLayersType> DrawerWrapper{};

		public:
			TemplateEngine(TS::Scheduler& scheduler, ScreenCommsInterfaceType& screenComms)
#if defined(USE_DOUBLE_FRAME_BUFFER)
				: Framebuffer(Buffer, AltBuffer)
#else
				: Framebuffer(Buffer)
#endif
				, ScreenComms(screenComms)
				, ScreenDriver(ScreenComms)
				, GraphicsEngine(scheduler, Framebuffer, ScreenDriver, TargetFramePeriod)
#if defined(USE_LOG_FPS)
				, EngineLog(scheduler, GraphicsEngine)
#endif
			{
			}

			~TemplateEngine() = default;

			// Integer World output Surface for the EGFX graphics engine.
			auto& Surface()
			{
				return DrawerWrapper.ViewInstance.template view<0>();
			}

			void OnSetupFail()
			{
#if defined(USE_DYNAMIC_FRAME_BUFFER)
				delete[] Buffer;
#if defined(USE_DOUBLE_FRAME_BUFFER)
				delete[] AltBuffer;
#endif
#endif
			}

			bool Start(void (*taskCallback)(void* parameter), const Egfx::DisplaySyncType syncType = Egfx::DisplaySyncType::Vrr)
			{
#if defined(USE_DYNAMIC_FRAME_BUFFER)
				// Allocate memory and set frame buffer.
				Buffer = new uint8_t[FramebufferType::BufferSize]{};
				Framebuffer.SetBuffer(Buffer);
#if defined(USE_DOUBLE_FRAME_BUFFER)
				AltBuffer = new uint8_t[FramebufferType::BufferSize]{};
				Framebuffer.SetAltBuffer(AltBuffer);
#endif
#endif
				// Optional callback for RTOS driver variants.
				GraphicsEngine.SetBufferTaskCallback(taskCallback);

				// Initialize comms hardware.
				ScreenComms.begin();

				// Start EGFX graphics engine.
				if (!GraphicsEngine.Start())
				{
					return false;
				}

				// Set the output surface buffer for the Integer World engine.
				Surface().SetBuffer(Framebuffer);

				// Set the drawer wrapper for the graphics engine.
				GraphicsEngine.SetDrawer(&DrawerWrapper);

				// Set the Display Sync Type.
				GraphicsEngine.SetSyncType(syncType);

#if defined(USE_LOG_FPS) // Optional performance logging.
				EngineLog.Start();
#endif
				return true;
			}

			void BufferTaskCallback(void* parameter)
			{
				GraphicsEngine.BufferTaskCallback(parameter);
			}
		};
	}
}

#endif