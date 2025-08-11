#ifndef _INTEGER_WORLD_ENGINE_RENDER_TASK_h
#define _INTEGER_WORLD_ENGINE_RENDER_TASK_h

//#define INTEGER_WORLD_PERFORMANCE_DEBUG // Enable full render information.

#define _TASK_OO_CALLBACKS
#include <TSchedulerDeclarations.hpp>

#include "../Framework/Interface.h"
#include "../Framework/Viewport.h"
#include "../Framework/FragmentManager.h"

namespace IntegerWorld
{
	/// <summary>
	/// Abstract base class for render tasks that manage a fixed-size array of renderable objects.
	/// Provides basic object management (add/clear) and tracks the number of objects.
	/// </summary>
	/// <typeparam name="MaxObjectCount">Maximum number of objects this task can manage.</typeparam>
	template<uint16_t MaxObjectCount>
	class AbstractObjectRenderTask : public IEngineRenderer, public TS::Task
	{
	private:
		using RenderObjectType = IRenderObject*;

	protected:
		uint16_t ObjectCount = 0; // Tracks the current number of objects.
		RenderObjectType Objects[MaxObjectCount]{}; // The fixed-size storage for objects.

	public:
		/// <summary>
		/// Constructor. Registers the task with the provided scheduler.
		/// </summary>
		AbstractObjectRenderTask(TS::Scheduler& scheduler)
			: IEngineRenderer()
			, TS::Task(TASK_IMMEDIATE, TASK_FOREVER, &scheduler, false)
		{
		}

		/// <summary>
		/// Removes all objects from the task.
		/// </summary>
		virtual void ClearObjects()
		{
			ObjectCount = 0;
		}

		/// <summary>
		/// Adds a render object to the task if there is space.
		/// </summary>
		/// <param name="renderObject">Pointer to the object to add.</param>
		/// <returns>True if added, false if full or null.</returns>
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

	/// <summary>
	/// Main engine render task. Manages the rendering pipeline for a set of objects,
	/// including all major stages: vertex shading, primitive shading, camera transform,
	/// projection, fragment collection, sorting, and rasterization.
	/// </summary>
	/// <typeparam name="MaxObjectCount">Maximum number of renderable objects.</typeparam>
	/// <typeparam name="MaxOrderedPrimitives">Maximum number of ordered primitives/fragments.</typeparam>
	/// <typeparam name="BatchSize">Number of items processed per callback iteration.</typeparam>
	template<uint16_t MaxObjectCount, uint16_t MaxOrderedPrimitives, uint16_t BatchSize = 1>
	class EngineRenderTask : public AbstractObjectRenderTask<MaxObjectCount>
	{
	private:
		using Base = AbstractObjectRenderTask<MaxObjectCount>;

	protected:
		using Base::Objects;
		using Base::ObjectCount;

	private:
		/// <summary>
		/// Internal state machine for the rendering pipeline.
		/// </summary>
		enum class StateEnum : uint8_t
		{
			Disabled,           // Task is disabled.
			EngineStart,        // Initial state, prepares the surface.
			CycleStart,         // Prepares for a new frame.
			VertexShade,        // Vertex shading stage.
			PrimitiveWorldShade,// World-space primitive shading.
			CameraTransform,    // Applies camera transformation.
			ScreenProject,      // Projects primitives to screen space.
			PrimitiveScreenShade,// Screen-space primitive shading.
			FragmentCollect,    // Collects fragments for rasterization.
			FragmentSort,       // Sorts fragments for correct rendering order.
			WaitForSurface,     // Waits for the output surface to be ready.
			Rasterize           // Rasterizes fragments to the surface.
		};

	private:
		ordered_fragment_t OrderedPrimitives[MaxOrderedPrimitives]{}; // Storage for ordered fragments.

	private:
		// Viewport projector for screen-space transformations and clipping.
		ViewportProjector ViewProjector{};

		// Camera state and transformation for the current frame.
		camera_state_t CameraControls{};
		camera_transform_t ReverseCameraTransform{};
		StateEnum State = StateEnum::Disabled; // Current pipeline state.

		uint16_t ObjectIndex = 0; // Index of the current object being processed.
		uint16_t ItemIndex = 0;   // Index of the current item within the object.

	private:
		uint32_t MeasureStart = 0; // Used for performance timing.
#if defined(INTEGER_WORLD_PERFORMANCE_DEBUG)
		render_debug_status_struct Status{};     // Detailed render status for debugging.
		render_debug_status_struct StatusCopy{}; // Copy of status for reporting.
#else
		render_status_struct Status{};           // Minimal render status.
		render_status_struct StatusCopy{};       // Copy of status for reporting.
#endif
	private:
		SurfacedWindowRasterizer Rasterizer; // Handles rasterization to the output surface.

		OrderedFragmentManager<MaxOrderedPrimitives> FragmentManager; // Manages fragment collection and sorting.

	public:
		/// <summary>
		/// Constructor. Initializes the render task with a scheduler and output surface.
		/// </summary>
		EngineRenderTask(TS::Scheduler& scheduler, IOutputSurface& surface)
			: Base(scheduler)
			, Rasterizer(surface)
			, FragmentManager(OrderedPrimitives)
		{
		}

#if defined(INTEGER_WORLD_PERFORMANCE_DEBUG)
		/// <summary>
		/// Gets the full renderer status (debug version).
		/// </summary>
		void GetRendererStatus(render_debug_status_struct& rendererStatus) final
		{
			rendererStatus = StatusCopy;
		}

		/// <summary>
		/// Gets the minimal renderer status (debug version).
		/// </summary>
		void GetRendererStatus(render_status_struct& rendererStatus) final
		{
			rendererStatus.Rasterize = StatusCopy.Rasterize;
			rendererStatus.Render = StatusCopy.GetRenderDuration();
			rendererStatus.FragmentsDrawn = StatusCopy.FragmentsDrawn;
		}
#else
		/// <summary>
		/// Gets the minimal renderer status.
		/// </summary>
		void GetRendererStatus(render_status_struct& rendererStatus) final
		{
			rendererStatus = StatusCopy;
		}
#endif

		/// <summary>
		/// Starts the render task.
		/// </summary>
		void Start()
		{
			SetEnabled(true);
		}

		/// <summary>
		/// Stops the render task.
		/// </summary>
		void Stop()
		{
			SetEnabled(false);
		}

		/// <summary>
		/// Returns a pointer to the camera controls for external manipulation.
		/// </summary>
		camera_state_t* GetCameraControls() final
		{
			return &CameraControls;
		}

		/// <summary>
		/// Sets the field of view for the viewport projector.
		/// </summary>
		void SetFov(const ufraction16_t fovFraction) final
		{
			ViewProjector.SetFov(fovFraction);
		}

		/// <summary>
		/// Main callback for the task scheduler. Advances the rendering pipeline state machine.
		/// Processes a batch of items per call, progressing through all pipeline stages.
		/// </summary>
		bool Callback() final
		{
			switch (State)
			{
			case StateEnum::EngineStart:
				// Try to start the output surface. If successful, move to next stage.
				if (Rasterizer.StartSurface())
				{
					State = StateEnum::CycleStart;
				}
				else
				{
					State = StateEnum::Disabled;
				}
				break;
			case StateEnum::CycleStart:
				// Prepare for a new frame: reset timers, update dimensions, clear fragments.
				MeasureStart = micros();
				Status.Clear();
				Rasterizer.UpdateDimensions();
				ViewProjector.SetDimensions(Rasterizer.Width(), Rasterizer.Height());

				FragmentManager.Clear();
				ObjectIndex = 0;
				ItemIndex = 0;
				if (ObjectCount > 0)
				{
					// Prepare camera transform for the frame.
					ReverseCameraTransform.Translation.x = -CameraControls.Position.x;
					ReverseCameraTransform.Translation.y = -CameraControls.Position.y;
					ReverseCameraTransform.Translation.z = -CameraControls.Position.z;

					CalculateTransformRotation(ReverseCameraTransform,
						ANGLE_RANGE - CameraControls.Rotation.x,
						ANGLE_RANGE - CameraControls.Rotation.y,
						ANGLE_RANGE - CameraControls.Rotation.z
					);

					ReverseCameraTransform.ViewDistance = ViewProjector.GetViewDistance();
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
				// Perform vertex shading for each object in batches.
				MeasureStart = micros();
				for (uint_fast16_t i = 0; i < BatchSize; i++)
				{
					if (Objects[ObjectIndex]->VertexShade(ItemIndex))
					{
#if defined(INTEGER_WORLD_PERFORMANCE_DEBUG)
						Status.VertexShades += ItemIndex;
#endif
						ItemIndex = 0;
						ObjectIndex++;
						if (ObjectIndex >= ObjectCount)
						{
							ItemIndex = 0;
							ObjectIndex = 0;
							State = StateEnum::PrimitiveWorldShade;
							break;
						}
					}
					else
					{
						ItemIndex++;
					}
				}
#if defined(INTEGER_WORLD_PERFORMANCE_DEBUG)
				Status.VertexShade += micros() - MeasureStart;
#else
				Status.Render += micros() - MeasureStart;
#endif
				break;
			case StateEnum::PrimitiveWorldShade:
				// Perform world-space primitive shading for each object in batches.
				MeasureStart = micros();
				for (uint_fast16_t i = 0; i < BatchSize; i++)
				{
					if (Objects[ObjectIndex]->PrimitiveWorldShade(ItemIndex))
					{
#if defined(INTEGER_WORLD_PERFORMANCE_DEBUG)
						Status.WorldShades += ItemIndex;
#endif
						ItemIndex = 0;
						ObjectIndex++;
						if (ObjectIndex >= ObjectCount)
						{
							ItemIndex = 0;
							ObjectIndex = 0;
							State = StateEnum::CameraTransform;
							break;
						}
					}
					else
					{
						ItemIndex++;
					}
				}
#if defined(INTEGER_WORLD_PERFORMANCE_DEBUG)
				Status.WorldShade += micros() - MeasureStart;
#else
				Status.Render += micros() - MeasureStart;
#endif
				break;
			case StateEnum::CameraTransform:
				// Apply camera transformation to all objects in batches.
				MeasureStart = micros();
				for (uint_fast16_t i = 0; i < BatchSize; i++)
				{
					if (Objects[ObjectIndex]->CameraTransform(ReverseCameraTransform, ItemIndex))
					{
#if defined(INTEGER_WORLD_PERFORMANCE_DEBUG)
						Status.CameraTransforms += ItemIndex;
#endif
						ItemIndex = 0;
						ObjectIndex++;
						if (ObjectIndex >= ObjectCount)
						{
							ItemIndex = 0;
							ObjectIndex = 0;
							State = StateEnum::ScreenProject;
							break;
						}
					}
					else
					{
						ItemIndex++;
					}
				}
#if defined(INTEGER_WORLD_PERFORMANCE_DEBUG)
				Status.CameraTransform += micros() - MeasureStart;
#else
				Status.Render += micros() - MeasureStart;
#endif
				break;
			case StateEnum::ScreenProject:
				// Project all primitives to screen space in batches.
				MeasureStart = micros();
				for (uint_fast16_t i = 0; i < BatchSize; i++)
				{
					if (Objects[ObjectIndex]->ScreenProject(ViewProjector, ItemIndex))
					{
#if defined(INTEGER_WORLD_PERFORMANCE_DEBUG)
						Status.ScreenProjects += ItemIndex;
#endif
						ItemIndex = 0;
						ObjectIndex++;
						if (ObjectIndex >= ObjectCount)
						{
							ItemIndex = 0;
							ObjectIndex = 0;
							State = StateEnum::PrimitiveScreenShade;
							break;
						}
					}
					else
					{
						ItemIndex++;
					}
				}
#if defined(INTEGER_WORLD_PERFORMANCE_DEBUG)
				Status.ScreenProject += micros() - MeasureStart + 1;
#else
				Status.Render += micros() - MeasureStart;
#endif
				break;
			case StateEnum::PrimitiveScreenShade:
				// Perform screen-space primitive shading for all objects in batches.
				MeasureStart = micros();
				for (uint_fast16_t i = 0; i < BatchSize; i++)
				{
					if (Objects[ObjectIndex]->PrimitiveScreenShade(ItemIndex, Rasterizer.Width(), Rasterizer.Height()))
					{
#if defined(INTEGER_WORLD_PERFORMANCE_DEBUG)
						Status.ScreenShades += ItemIndex;
#endif
						ItemIndex = 0;
						ObjectIndex++;
						if (ObjectIndex >= ObjectCount)
						{
							ItemIndex = 0;
							ObjectIndex = 0;
							State = StateEnum::FragmentCollect;
							break;
						}
					}
					else
					{
						ItemIndex++;
					}
				}
#if defined(INTEGER_WORLD_PERFORMANCE_DEBUG)
				Status.ScreenShade += micros() - MeasureStart;
#else
				Status.Render += micros() - MeasureStart;
#endif
				break;
			case StateEnum::FragmentCollect:
				// Collect fragments from each object for rasterization.
				MeasureStart = micros();
				FragmentManager.PrepareForObject(ObjectIndex);
				Objects[ObjectIndex]->FragmentCollect((FragmentCollector&)FragmentManager);
				ObjectIndex++;
				if (ObjectIndex >= ObjectCount)
				{
					State = StateEnum::FragmentSort;
				}
#if defined(INTEGER_WORLD_PERFORMANCE_DEBUG)
				Status.FragmentCollect += micros() - MeasureStart;
#else
				Status.Render += micros() - MeasureStart;
#endif
				break;
			case StateEnum::FragmentSort:
				// Sort all collected fragments for correct rendering order.
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
				// Wait until the output surface is ready for drawing.
				if (Rasterizer.IsSurfaceReady())
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
				// Rasterize all sorted fragments to the output surface.
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
					Rasterizer.FlipSurface();
					State = StateEnum::CycleStart;
				}
				break;
			default:
				// If in an unknown state, disable the task.
				SetEnabled(false);
				break;
			}

			return true;
		}

		/// <summary>
		/// Enables or disables the render task and updates the state machine accordingly.
		/// </summary>
		/// <param name="enabled">True to enable, false to disable.</param>
		virtual void SetEnabled(const bool enabled)
		{
			if (enabled)
			{
				State = StateEnum::EngineStart;
				TS::Task::enable();
			}
			else
			{
				Rasterizer.StopSurface();
				State = StateEnum::Disabled;
				TS::Task::disable();
			}
		}

		/// <summary>
		/// Clears all objects and resets the pipeline if running.
		/// </summary>
		void ClearObjects() final
		{
			Base::ClearObjects();
			if (State != StateEnum::Disabled
				&& State != StateEnum::EngineStart)
			{
				State = StateEnum::CycleStart;
			}
		}

		/// <summary>
		/// Adds a render object and resets the pipeline if running.
		/// </summary>
		/// <param name="renderObject">Pointer to the object to add.</param>
		/// <returns>True if added and pipeline reset, false otherwise.</returns>
		bool AddObject(IRenderObject* renderObject) final
		{
			if (Base::AddObject(renderObject))
			{
				if (State != StateEnum::Disabled)
				{
					if (State != StateEnum::EngineStart)
					{
						State = StateEnum::CycleStart;

					}
					return true;
				}
			}

			return false;
		}
	};
}

#endif