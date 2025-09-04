#ifndef _INTEGER_WORLD_PERFORMANCE_LOG_TASK_h
#define _INTEGER_WORLD_PERFORMANCE_LOG_TASK_h

#define _TASK_OO_CALLBACKS
#include <TSchedulerDeclarations.hpp>

#include "../Framework/Interface.h"

namespace IntegerWorld
{
	template<const uint32_t LogPeriodMillis = 1000>
	class PerformanceLogTask : private TS::Task
	{
	private:
		IEngineRenderer& Engine;
#if defined(INTEGER_WORLD_PERFORMANCE_DEBUG)
		render_debug_status_struct RendererStatus{};
#else
		render_status_struct RendererStatus{};
#endif

	public:
		PerformanceLogTask(TS::Scheduler& scheduler, IEngineRenderer& engine)
			: TS::Task(LogPeriodMillis, TASK_FOREVER, &scheduler, true), Engine(engine)
		{
		}

	public:
		bool Callback() final
		{
			Engine.GetRendererStatus(RendererStatus);

			if (RendererStatus.GetRenderDuration() > 0)
			{
				Serial.println();
				Serial.print(F("Integer World Renderer\t"));
				if (RendererStatus.FrameDuration > 0)
				{
					const uint32_t subUnits = 1000000000 / RendererStatus.FrameDuration;
					Serial.print(subUnits / 1000);
					Serial.print(',');
					Serial.print((subUnits / 100) % 10);
					Serial.print((subUnits / 10) % 10);
				}
				else
				{
					Serial.print(F("0.00"));
				}
				Serial.println(F(" FPS"));

#if defined(INTEGER_WORLD_PERFORMANCE_DEBUG)
				Serial.print(F("\tFramePreparation "));
				Serial.print(RendererStatus.FramePreparation);
				Serial.println(F("us"));
				Serial.print(F("\tObjectShade("));
				Serial.print(RendererStatus.ObjectShades);
				Serial.print(F(") "));
				Serial.print(RendererStatus.ObjectShade);
				Serial.println(F("us"));
				Serial.print(F("\tVertexShade("));
				Serial.print(RendererStatus.VertexShades);
				Serial.print(F(") "));
				Serial.print(RendererStatus.VertexShade);
				Serial.println(F("us"));
				Serial.print(F("\tWorldTransform("));
				Serial.print(RendererStatus.WorldTransforms);
				Serial.print(F(") "));
				Serial.print(RendererStatus.WorldTransform);
				Serial.println(F("us"));
				Serial.print(F("\tWorldShade("));
				Serial.print(RendererStatus.WorldShades);
				Serial.print(F(") "));
				Serial.print(RendererStatus.WorldShade);
				Serial.println(F("us"));
				Serial.print(F("\tCameraTransform("));
				Serial.print(RendererStatus.CameraTransforms);
				Serial.print(F(") "));
				Serial.print(RendererStatus.CameraTransform);
				Serial.println(F("us"));
				Serial.print(F("\tScreenProject("));
				Serial.print(RendererStatus.ScreenProjects);
				Serial.print(F(") "));
				Serial.print(RendererStatus.ScreenProject);
				Serial.println(F("us"));
				Serial.print(F("\tScreenShade("));
				Serial.print(RendererStatus.ScreenShades);
				Serial.print(F(") "));
				Serial.print(RendererStatus.ScreenShade);
				Serial.println(F("us"));
				Serial.print(F("\tFragmentCollect "));
				Serial.print(RendererStatus.FragmentCollect);
				Serial.println(F("us"));
				Serial.print(F("\tFragmentSort "));
				Serial.print(RendererStatus.FragmentSort);
				Serial.println(F("us"));

				if (RendererStatus.RasterizeWait >= 0)
				{
					Serial.print(F("\tRasterWait: "));
					Serial.print(RendererStatus.RasterizeWait);
					Serial.println(F("us"));
				}
#else
#endif
				Serial.print(F("\tRaster("));
				Serial.print(RendererStatus.FragmentsDrawn);
				Serial.print(F(") "));
				Serial.print(RendererStatus.Rasterize);
				Serial.println(F("us"));

				Serial.print(F("\tRender Total: "));
				Serial.print(RendererStatus.GetRenderDuration());
				Serial.println(F("us"));

				Serial.println();
				Serial.println();
			}


			return true;
		}
	};
}

#endif