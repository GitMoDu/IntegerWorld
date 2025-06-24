#ifndef _INTEGER_WORLD_ENGINE_RENDER_TASK_h
#define _INTEGER_WORLD_ENGINE_RENDER_TASK_h

#define _TASK_OO_CALLBACKS
#include <TSchedulerDeclarations.hpp>

#include "../Framework/Interface.h"
#include "../Framework/Viewport.h"

namespace IntegerWorld
{
	template<uint16_t MaxObjectCount>
	class AbstractObjectRenderTask : public IEngineRenderer, public TS::Task
	{
	private:
		using RenderObjectType = IRenderObject*;

	protected:
		uint16_t ObjectCount = 0; // Tracks the current number of objects.
		RenderObjectType Objects[MaxObjectCount]{}; // The fixed-size storage for objects.

	public:
		AbstractObjectRenderTask(TS::Scheduler& scheduler)
			: IEngineRenderer()
			, TS::Task(TASK_IMMEDIATE, TASK_FOREVER, &scheduler, false)
		{
		}

		virtual void ClearObjects()
		{
			ObjectCount = 0;
		}

		virtual bool AddObject(IRenderObject* renderObject)
		{
			if (renderObject != nullptr && ObjectCount < MaxObjectCount)
			{
				Objects[ObjectCount] = renderObject;
				ObjectCount++;

				return true;
			}
			else
			{
				return false;
			}
		}

	protected:
		void SortObjects()
		{
			// Insertion sort using the Compare method
			for (uint16_t i = 1; i < ObjectCount; ++i)
			{
				RenderObjectType item = Objects[i];
				uint16_t j = i;

				// Move elements of Items[0..i-1] that are greater than key
				// to one position ahead of their current position
				for (; j > 0; --j)
				{
					if (!(Objects[j - 1]->GetZPosition() < item->GetZPosition()))
					{
						break;
					}
					Objects[j] = Objects[j - 1];
				}
				Objects[j] = item;
			}
		}
	};

	template<uint16_t MaxObjectCount>
	class EngineRenderTask : public AbstractObjectRenderTask<MaxObjectCount>
	{
	private:
		using Base = AbstractObjectRenderTask<MaxObjectCount>;

	protected:
		using Base::Objects;
		using Base::ObjectCount;
		using Base::SortObjects;

	private:
		enum class StateEnum : uint8_t
		{
			Disabled,
			EngineStart,
			CycleStart,
			VertexShade,
			PrimitiveWorldShade,
			CameraTransform,
			ScreenProject,
			PrimitiveScreenShade,
			PrimitiveSort,
			ObjectSort,
			WaitForSurface,
			Rasterize
		};

	private:
		// Viewport clipping planes and window for rasterizing.
		ViewportProjector ViewProjector{};

	private:
		camera_state_t CameraControls{};
		transform32_rotate_translate_t ReverseCameraTransform{};
		StateEnum State = StateEnum::Disabled;

		uint16_t ObjectIndex = 0;
		uint16_t ItemIndex = 0;

#if defined(INTEGER_WORLD_PERFORMANCE_LOG)
		uint32_t MeasureStart = 0;
		render_status_struct Status{};
		render_status_struct StatusCopy{};
#endif
	private:
		WindowRasterizer Rasterizer;


	public:
		EngineRenderTask(TS::Scheduler& scheduler, IOutputSurface* surface, const bool startFullscreen = true)
			: Base(scheduler)
			, Rasterizer(surface, startFullscreen)
		{
		}

#if defined(INTEGER_WORLD_PERFORMANCE_LOG)
		virtual void GetRendererStatus(render_status_struct& rendererStatus) final
		{
			rendererStatus = StatusCopy;
		}
#endif

		void SetSize(const int16_t width, const int16_t height)
		{
			Rasterizer.SetSize(width, height);
		}

		void Start()
		{
			SetEnabled(true);
		}

		void Stop()
		{
			SetEnabled(false);
		}

		virtual camera_state_t* GetCameraControls()
		{
			return &CameraControls;
		}

		virtual void SetFov(const ufraction16_t fovFraction)
		{
			ViewProjector.SetFov(fovFraction);
		}

		bool Callback() final
		{
			switch (State)
			{
			case StateEnum::EngineStart:
				if (Rasterizer.Surface != nullptr)
				{
					Rasterizer.Surface->StartSurface();
					State = StateEnum::CycleStart;
				}
				else
				{
					State = StateEnum::Disabled;
					//Serial.println(F("Unable to start render engine"));
				}
				break;
			case StateEnum::CycleStart:
#if defined(INTEGER_WORLD_PERFORMANCE_LOG)
				StatusCopy = Status;
				Status.Clear();
				MeasureStart = micros();
#endif
				ViewProjector.SetDimensions(Rasterizer.Width(), Rasterizer.Height());
				ObjectIndex = 0;
				ItemIndex = 0;
				if (ObjectCount > 0)
				{
					ReverseCameraTransform.Translation.x = CameraControls.Position.x;
					ReverseCameraTransform.Translation.y = CameraControls.Position.y;
					ReverseCameraTransform.Translation.z = CameraControls.Position.z;

					CalculateTransformRotation(ReverseCameraTransform,
						ANGLE_RANGE - CameraControls.Rotation.x,
						ANGLE_RANGE - CameraControls.Rotation.y,
						ANGLE_RANGE - CameraControls.Rotation.z);
					State = StateEnum::VertexShade;
				}
				else
				{
					State = StateEnum::Rasterize;
				}
#if defined(INTEGER_WORLD_PERFORMANCE_LOG)
				Status.EngineCameraDuration += micros() - MeasureStart;
#endif
				break;
			case StateEnum::VertexShade:
#if defined(INTEGER_WORLD_PERFORMANCE_LOG)
				Status.VertexShades++;
				MeasureStart = micros();
#endif
				if (Objects[ObjectIndex]->VertexShade(ItemIndex))
				{
					ItemIndex = 0;
					ObjectIndex++;
					if (ObjectIndex >= ObjectCount)
					{
						ItemIndex = 0;
						ObjectIndex = 0;
						State = StateEnum::PrimitiveWorldShade;
					}
				}
				else
				{
					ItemIndex++;
				}
#if defined(INTEGER_WORLD_PERFORMANCE_LOG)
				Status.VertexShadeDuration += micros() - MeasureStart;
#endif
				break;
			case StateEnum::PrimitiveWorldShade:
#if defined(INTEGER_WORLD_PERFORMANCE_LOG)
				Status.PrimitiveWorldShades++;
				MeasureStart = micros();
#endif
				if (Objects[ObjectIndex]->PrimitiveWorldShade(ItemIndex))
				{
					ItemIndex = 0;
					ObjectIndex++;
					if (ObjectIndex >= ObjectCount)
					{
						ItemIndex = 0;
						ObjectIndex = 0;
						State = StateEnum::CameraTransform;
					}
				}
				else
				{
					ItemIndex++;
				}
#if defined(INTEGER_WORLD_PERFORMANCE_LOG)
				Status.PrimitiveWorldShadeDuration += micros() - MeasureStart;
#endif
				break;
			case StateEnum::CameraTransform:
#if defined(INTEGER_WORLD_PERFORMANCE_LOG)
				Status.CameraTransforms++;
				MeasureStart = micros();
#endif
				if (Objects[ObjectIndex]->CameraTransform(ReverseCameraTransform, ItemIndex))
				{
					ItemIndex = 0;
					ObjectIndex++;
					if (ObjectIndex >= ObjectCount)
					{
						ItemIndex = 0;
						ObjectIndex = 0;
						State = StateEnum::ScreenProject;
					}
				}
				else
				{
					ItemIndex++;
				}
#if defined(INTEGER_WORLD_PERFORMANCE_LOG)
				Status.CameraTransformDuration += micros() - MeasureStart;
#endif
				break;
			case StateEnum::ScreenProject:
#if defined(INTEGER_WORLD_PERFORMANCE_LOG)
				Status.ScreenProjects++;
				MeasureStart = micros();
#endif
				if (Objects[ObjectIndex]->ScreenProject(ViewProjector, ItemIndex))
				{
					ItemIndex = 0;
					ObjectIndex++;
					if (ObjectIndex >= ObjectCount)
					{
						ItemIndex = 0;
						ObjectIndex = 0;
						State = StateEnum::PrimitiveScreenShade;
					}
				}
				else
				{
					ItemIndex++;
				}
#if defined(INTEGER_WORLD_PERFORMANCE_LOG)
				Status.ScreenProjectDuration += micros() - MeasureStart + 1;
#endif
				break;
			case StateEnum::PrimitiveScreenShade:
#if defined(INTEGER_WORLD_PERFORMANCE_LOG)
				Status.PrimitiveScreenShades++;
				MeasureStart = micros();
#endif

				if (Objects[ObjectIndex]->PrimitiveScreenShade(Rasterizer.Width(), Rasterizer.Height(), ItemIndex))
				{
					ItemIndex = 0;
					ObjectIndex++;
					if (ObjectIndex >= ObjectCount)
					{
						ItemIndex = 0;
						ObjectIndex = 0;
						State = StateEnum::PrimitiveSort;
					}
				}
				else
				{
					ItemIndex++;
				}
#if defined(INTEGER_WORLD_PERFORMANCE_LOG)
				Status.PrimitiveScreenShadeDuration += micros() - MeasureStart;
#endif
				break;
			case StateEnum::PrimitiveSort:
#if defined(INTEGER_WORLD_PERFORMANCE_LOG)
				Status.ItemSorts++;
				MeasureStart = micros();
#endif
				Objects[ObjectIndex]->PrimitiveSort();
				ObjectIndex++;
				if (ObjectIndex >= ObjectCount)
				{
					ObjectIndex = 0;
					State = StateEnum::ObjectSort;
				}
#if defined(INTEGER_WORLD_PERFORMANCE_LOG)
				Status.ItemSortDuration += micros() - MeasureStart;
#endif
				break;
			case StateEnum::ObjectSort: // Sort object render by camera distance.
#if defined(INTEGER_WORLD_PERFORMANCE_LOG)
				MeasureStart = micros();
#endif
				SortObjects();
				ObjectIndex = 0;
				ItemIndex = 0;
				State = StateEnum::WaitForSurface;
#if defined(INTEGER_WORLD_PERFORMANCE_LOG)
				Status.EngineSortDuration = micros() - MeasureStart;
				Status.RasterizeWaitDuration = 0;
				MeasureStart = micros();
#endif
				break;
			case StateEnum::WaitForSurface:
#if defined(INTEGER_WORLD_PERFORMANCE_LOG)
				MeasureStart = micros();
#endif
				// Poll layer to check if we can draw.
				if (Rasterizer.Surface->IsSurfaceReady())
				{
					State = StateEnum::Rasterize;
				}
#if defined(INTEGER_WORLD_PERFORMANCE_LOG)
				else
				{
					Status.RasterizeWaitDuration += micros() - MeasureStart;
				}
#endif
				break;
			case StateEnum::Rasterize:
#if defined(INTEGER_WORLD_PERFORMANCE_LOG)
				Status.Rasterizes++;
				MeasureStart = micros();
#endif
				if (ObjectCount > 0)
				{
					if (Objects[ObjectIndex]->FragmentShade(Rasterizer, ItemIndex))
					{
						ItemIndex = 0;
						ObjectIndex++;
						if (ObjectIndex >= ObjectCount)
						{
							Rasterizer.Surface->FlipSurface();
							State = StateEnum::CycleStart;
						}
					}
					else
					{
						ItemIndex++;
					}
				}
				else
				{
					Rasterizer.Surface->FlipSurface();
					State = StateEnum::CycleStart;
				}
#if defined(INTEGER_WORLD_PERFORMANCE_LOG)
				Status.RasterizeDuration += micros() - MeasureStart;
#endif
				break;
			default:
				SetEnabled(false);
				break;
			}

			return true;
		}

		virtual void SetEnabled(const bool enabled)
		{
			if (enabled)
			{
				State = StateEnum::EngineStart;
				TS::Task::enable();
			}
			else
			{
				if (Rasterizer.Surface != nullptr)
				{
					Rasterizer.Surface->StopSurface();
				}
				State = StateEnum::Disabled;
				TS::Task::disable();
			}
		}

		virtual void ClearObjects()
		{
			Base::ClearObjects();
			if (State != StateEnum::Disabled
				&& State != StateEnum::EngineStart)
			{
				State = StateEnum::CycleStart;
			}
		}

		virtual bool AddObject(IRenderObject* renderObject)
		{
			if (Base::AddObject(renderObject))
			{
				if (State != StateEnum::Disabled
					&& State != StateEnum::EngineStart)
				{
					State = StateEnum::CycleStart;

					return true;
				}
			}

			return false;
		}
	};
}

#endif