#ifndef _INTEGER_WORLD_ENGINE_RENDER_TASK_h
#define _INTEGER_WORLD_ENGINE_RENDER_TASK_h

//#define INTEGER_WORLD_PERFORMANCE_DEBUG // Enabled full render information.

#define _TASK_OO_CALLBACKS
#include <TSchedulerDeclarations.hpp>

#include "../Framework/Interface.h"
#include "../Framework/Viewport.h"
#include "../Framework/FragmentManager.h"

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
	};


	template<uint16_t MaxObjectCount, uint16_t MaxOrderedPrimitives>
	class EngineRenderTask : public AbstractObjectRenderTask<MaxObjectCount>
	{
	private:
		using Base = AbstractObjectRenderTask<MaxObjectCount>;

	protected:
		using Base::Objects;
		using Base::ObjectCount;

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
			FragmentCollect,
			FragmentSort,
			WaitForSurface,
			Rasterize
		};

	private:
		ordered_fragment_t OrderedPrimitives[MaxOrderedPrimitives]{};

	private:
		// Viewport clipping planes and window for rasterizing.
		ViewportProjector ViewProjector{};


		camera_state_t CameraControls{};
		transform32_rotate_translate_t ReverseCameraTransform{};
		StateEnum State = StateEnum::Disabled;

		uint16_t ObjectIndex = 0;
		uint16_t ItemIndex = 0;

	private:
		uint32_t MeasureStart = 0;
#if defined(INTEGER_WORLD_PERFORMANCE_DEBUG)
		render_debug_status_struct Status{};
		render_debug_status_struct StatusCopy{};
#else
		render_status_struct Status{};
		render_status_struct StatusCopy{};
#endif
	private:
		WindowRasterizer Rasterizer;
		OrderedFragmentManager FragmentManager;


	public:
		EngineRenderTask(TS::Scheduler& scheduler, IOutputSurface* surface, const bool startFullscreen = true)
			: Base(scheduler)
			, Rasterizer(surface, startFullscreen)
			, FragmentManager(OrderedPrimitives, MaxOrderedPrimitives)
		{
		}

#if defined(INTEGER_WORLD_PERFORMANCE_DEBUG)
		virtual void GetRendererStatus(render_debug_status_struct& rendererStatus) final
		{
			rendererStatus = StatusCopy;
		}

		virtual void GetRendererStatus(render_status_struct& rendererStatus) final
		{
			rendererStatus.Rasterize = StatusCopy.Rasterize;
			rendererStatus.Render = StatusCopy.GetRenderDuration();
			rendererStatus.FragmentsDrawn = StatusCopy.FragmentsDrawn;
		}
#else
		virtual void GetRendererStatus(render_status_struct& rendererStatus) final
		{
			rendererStatus = StatusCopy;
		}
#endif

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
				}
				break;
			case StateEnum::CycleStart:
				MeasureStart = micros();

				Status.Clear();
				ViewProjector.SetDimensions(Rasterizer.Width(), Rasterizer.Height());
				FragmentManager.Clear();
				ObjectIndex = 0;
				ItemIndex = 0;
				if (ObjectCount > 0)
				{
					ReverseCameraTransform.Translation.x = -CameraControls.Position.x;
					ReverseCameraTransform.Translation.y = -CameraControls.Position.y;
					ReverseCameraTransform.Translation.z = -CameraControls.Position.z;

					CalculateTransformRotation(ReverseCameraTransform,
						ANGLE_RANGE - CameraControls.Rotation.x,
						ANGLE_RANGE - CameraControls.Rotation.y,
						ANGLE_RANGE - CameraControls.Rotation.z);
					State = StateEnum::VertexShade;
				}
				else
				{
					State = StateEnum::WaitForSurface;
				}
#if defined(INTEGER_WORLD_PERFORMANCE_DEBUG)
				Status.FramePreparation += micros() - MeasureStart;
#else
				Status.Render += micros() - MeasureStart;
#endif
				break;
			case StateEnum::VertexShade:
#if defined(INTEGER_WORLD_PERFORMANCE_DEBUG)
				Status.VertexShades++;
#endif
				MeasureStart = micros();
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
#if defined(INTEGER_WORLD_PERFORMANCE_DEBUG)
				Status.VertexShade += micros() - MeasureStart;
#else
				Status.Render += micros() - MeasureStart;
#endif
				break;
			case StateEnum::PrimitiveWorldShade:
#if defined(INTEGER_WORLD_PERFORMANCE_DEBUG)
				Status.WorldShades++;
#endif
				MeasureStart = micros();
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
#if defined(INTEGER_WORLD_PERFORMANCE_DEBUG)
				Status.WorldShade += micros() - MeasureStart;
#else
				Status.Render += micros() - MeasureStart;
#endif
				break;
			case StateEnum::CameraTransform:
#if defined(INTEGER_WORLD_PERFORMANCE_DEBUG)
				Status.CameraTransforms++;
#endif
				MeasureStart = micros();
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
#if defined(INTEGER_WORLD_PERFORMANCE_DEBUG)
				Status.CameraTransform += micros() - MeasureStart;
#else
				Status.Render += micros() - MeasureStart;
#endif
				break;
			case StateEnum::ScreenProject:
#if defined(INTEGER_WORLD_PERFORMANCE_DEBUG)
				Status.ScreenProjects++;
#endif
				MeasureStart = micros();
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
#if defined(INTEGER_WORLD_PERFORMANCE_DEBUG)
				Status.ScreenProject += micros() - MeasureStart + 1;
#else
				Status.Render += micros() - MeasureStart;
#endif
				break;
			case StateEnum::PrimitiveScreenShade:
#if defined(INTEGER_WORLD_PERFORMANCE_DEBUG)
				Status.ScreenShades++;
#endif
				MeasureStart = micros();
				if (Objects[ObjectIndex]->PrimitiveScreenShade(ItemIndex, Rasterizer.Width(), Rasterizer.Height()))
				{
					ItemIndex = 0;
					ObjectIndex++;
					if (ObjectIndex >= ObjectCount)
					{
						ItemIndex = 0;
						ObjectIndex = 0;
						State = StateEnum::FragmentCollect;
					}
				}
				else
				{
					ItemIndex++;
				}
#if defined(INTEGER_WORLD_PERFORMANCE_DEBUG)
				Status.ScreenShade += micros() - MeasureStart;
#else
				Status.Render += micros() - MeasureStart;
#endif
				break;
			case StateEnum::FragmentCollect:
				MeasureStart = micros();
				FragmentManager.PrepareForObject(ObjectIndex);
				Objects[ObjectIndex]->FragmentCollect((FragmentCollector&)FragmentManager);
				ObjectIndex++;
				if (ObjectIndex >= ObjectCount)
				{
					State = StateEnum::FragmentSort;
				}
#if defined(INTEGER_WORLD_PERFORMANCE_DEBUG)
				Status.FragmentSort += micros() - MeasureStart;
#else
				Status.Render += micros() - MeasureStart;
#endif
				break;
			case StateEnum::FragmentSort:
				MeasureStart = micros();
				FragmentManager.Sort();
				Status.FragmentsDrawn = FragmentManager.Count();
				State = StateEnum::WaitForSurface;
#if defined(INTEGER_WORLD_PERFORMANCE_DEBUG)
				Status.FragmentSort = micros() - MeasureStart;
#else
				Status.Render += micros() - MeasureStart;
#endif
				MeasureStart = micros();
				break;
			case StateEnum::WaitForSurface:
#if defined(INTEGER_WORLD_PERFORMANCE_DEBUG)
				MeasureStart = micros();
#endif
				// Poll layer to check if we can draw.
				if (Rasterizer.Surface->IsSurfaceReady())
				{
					ObjectIndex = 0;
					ItemIndex = 0;
					State = StateEnum::Rasterize;
				}
#if defined(INTEGER_WORLD_PERFORMANCE_DEBUG)
				else
				{
					Status.RasterizeWait += micros() - MeasureStart;
				}
#endif
				break;
			case StateEnum::Rasterize:
				MeasureStart = micros();
				if (ItemIndex < FragmentManager.Count())
				{
					Objects[OrderedPrimitives[ItemIndex].ObjectIndex]->FragmentShade(Rasterizer,
						OrderedPrimitives[ItemIndex].FragmentIndex);
					ItemIndex++;
					Status.Rasterize += micros() - MeasureStart;
				}
				else
				{
					StatusCopy = Status;
					Rasterizer.Surface->FlipSurface();
					State = StateEnum::CycleStart;
				}
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