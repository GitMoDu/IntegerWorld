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
		render_status_struct RendererStatus{};

	public:
		PerformanceLogTask(TS::Scheduler& scheduler, IEngineRenderer& engine)
			: TS::Task(LogPeriodMillis, TASK_FOREVER, &scheduler, true), Engine(engine)
		{
		}

	public:
		bool Callback() final
		{
			Engine.GetRendererStatus(RendererStatus);

			Serial.println();
			Serial.println(F("Integer World Renderer"));
			Serial.print(F("\tVertexShade("));
			Serial.print(RendererStatus.VertexShades);
			Serial.print(F(") "));
			Serial.print(RendererStatus.VertexShadeDuration);
			Serial.println(F("us"));
			Serial.print(F("\tPrimitiveWorldShade("));
			Serial.print(RendererStatus.PrimitiveWorldShades);
			Serial.print(F(") "));
			Serial.print(RendererStatus.PrimitiveWorldShadeDuration);
			Serial.println(F("us"));
			Serial.print(F("\tCameraTransform("));
			Serial.print(RendererStatus.CameraTransforms);
			Serial.print(F(") "));
			Serial.print(RendererStatus.CameraTransformDuration);
			Serial.println(F("us"));
			Serial.print(F("\tScreenProject("));
			Serial.print(RendererStatus.ScreenProjects);
			Serial.print(F(") "));
			Serial.print(RendererStatus.ScreenProjectDuration);
			Serial.println(F("us"));
			Serial.print(F("\tPrimitiveScreenShade("));
			Serial.print(RendererStatus.PrimitiveScreenShades);
			Serial.print(F(") "));
			Serial.print(RendererStatus.PrimitiveScreenShadeDuration);
			Serial.println(F("us"));



			Serial.print(F("\tEngineSort "));
			Serial.print(RendererStatus.EngineSortDuration);
			Serial.println(F("us"));

			Serial.print(F("\tItemSort("));
			Serial.print(RendererStatus.ItemSorts);
			Serial.print(F(") "));
			Serial.print(RendererStatus.ItemSortDuration);
			Serial.println(F("us"));

			if (RendererStatus.RasterizeWaitDuration >= 0)
			{
				Serial.print(F("\tRasterWait: "));
				Serial.print(RendererStatus.RasterizeWaitDuration);
				Serial.println(F("us"));
			}

			Serial.print(F("\tRaster("));
			Serial.print(RendererStatus.Rasterizes);
			Serial.print(F(") "));
			Serial.print(RendererStatus.RasterizeDuration);
			Serial.println(F("us"));

			Serial.print(F("\tRender Total: "));
			Serial.print(RendererStatus.GetRenderDuration());
			Serial.println(F("us"));

			Serial.println();
			Serial.println();


			return true;
		}
	};
}

#endif