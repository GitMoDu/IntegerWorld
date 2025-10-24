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
			: TS::Task(LogPeriodMillis, TASK_FOREVER, &scheduler, false), Engine(engine)
		{
		}

	public:
		void Start()
		{
			TS::Task::enable();
		}

		void Stop()
		{
			TS::Task::disable();
		}

		bool Callback() final
		{
			Engine.GetRendererStatus(RendererStatus);

			if (RendererStatus.GetRenderDuration() > 0)
			{
				Serial.println();
				Serial.print(F("Integer World Log\t"));
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

				Serial.print(F("\tRaster("));
				Serial.print(RendererStatus.FragmentsDrawn);
				Serial.print(F(")\t"));
				Serial.print(RendererStatus.Rasterize);
				Serial.println(F("us"));

				Serial.print(F("\tRender Total\t"));
				Serial.print(RendererStatus.GetRenderDuration());
				Serial.println(F("us"));

#if defined(INTEGER_WORLD_PERFORMANCE_DEBUG)
				Serial.print(F("\tFrameSetup\t"));
				Serial.print(RendererStatus.FramePreparation);
				Serial.println(F("us"));
				Serial.print(F("\tObjectShade\t"));
				Serial.print(RendererStatus.ObjectShade);
				Serial.println(F("us"));
				Serial.print(F("\tVertexShade\t"));
				Serial.print(RendererStatus.VertexShade);
				Serial.println(F("us"));
				Serial.print(F("\tWorldTransform\t"));
				Serial.print(RendererStatus.WorldTransform);
				Serial.println(F("us"));
				Serial.print(F("\tWorldShade\t"));
				Serial.print(RendererStatus.WorldShade);
				Serial.println(F("us"));
				Serial.print(F("\tCameraTransform\t"));
				Serial.print(RendererStatus.CameraTransform);
				Serial.println(F("us"));
				Serial.print(F("\tScreenProject\t"));
				Serial.print(RendererStatus.ScreenProject);
				Serial.println(F("us"));
				Serial.print(F("\tScreenShade\t"));
				Serial.print(RendererStatus.ScreenShade);
				Serial.println(F("us"));
				Serial.print(F("\tFragmentCollect\t"));
				Serial.print(RendererStatus.FragmentCollect);
				Serial.println(F("us"));
				Serial.print(F("\tFragmentSort\t"));
				Serial.print(RendererStatus.FragmentSort);
				Serial.println(F("us"));
				Serial.print(F("\tRasterWait\t"));
				Serial.print(RendererStatus.RasterizeWait);
				Serial.println(F("us"));
#endif
				Serial.println();
			}


			return true;
		}
	};
}

#endif