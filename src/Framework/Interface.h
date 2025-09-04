#ifndef _INTEGER_WORLD_INTERFACE_h
#define _INTEGER_WORLD_INTERFACE_h

#include "Model.h"
#include "IOutputSurface.h"
#include "WindowRasterizer.h"
#include "Viewport.h"
#include "FragmentManager.h"


namespace IntegerWorld
{
	/// <summary>
	/// SceneShader shader takes in a material and fragment properties, to shade a provided color.
	/// </summary>
	struct ISceneShader
	{
		virtual void Shade(Rgb8::color_t& color, const material_t& material) = 0;
		virtual void Shade(Rgb8::color_t& color, const material_t& material, const world_position_shade_t& shade) = 0;
		virtual void Shade(Rgb8::color_t& color, const material_t& material, const world_position_normal_shade_t& shade) = 0;
	};

	struct ILightSource
	{
		virtual void GetLightColor(Rgb8::color_t& color) = 0;
		virtual void GetLighting(Rgb8::color_t& lightColor, ufraction16_t& diffuse, ufraction16_t& specular, const world_position_shade_t& shade) = 0;
		virtual void GetLighting(Rgb8::color_t& lightColor, ufraction16_t& diffuse, ufraction16_t& specular, const world_position_normal_shade_t& shade) = 0;
	};

	template<typename fragment_t>
	struct IFragmentShader
	{
		// Shade with scene shader.
		virtual void FragmentShade(WindowRasterizer& rasterizer, const fragment_t& fragment, ISceneShader* sceneShader) = 0;

		// Shade without scene shader.
		virtual void FragmentShade(WindowRasterizer& rasterizer, const fragment_t& fragment) = 0;
	};


	/// <summary>
	/// Rendering pipeline contract for render objects.
	///
	/// IRenderObject breaks rendering for an object into small, well-defined stages.
	/// EngineRenderTask drives those stages in stage-major order each frame: the engine steps
	/// one pipeline stage across all objects (and their per-stage indices) before advancing
	/// to the next stage. This interleaving reduces peak working set and enables a single,
	/// global fragment collector that gathers z-ordered primitives from every object.
	///
	/// Typical stage order invoked by the engine:
	///  - ObjectShade (once per object)
	///  - VertexShade (per-vertex, iterated for each object)
	///  - WorldTransform (per-vertex, iterated for each object)
	///  - WorldShade (per-primitive, iterated for each object)
	///  - CameraTransform (per-vertex, iterated for each object)
	///  - ScreenProject (per-vertex, iterated for each object)
	///  - ScreenShade (per-primitive, iterated for each object)
	///  - FragmentCollect (called once per object; fragments are added to the global collector)
	///  - FragmentSort / Rasterize (global across objects)
	///
	/// Important behavioral notes:
	/// - Objects perform their own culling internally (world- and screen-space) and only emit fragments
	///   for visible primitives. The engine does not perform per-primitive culling centrally.
	/// - Emitted fragments will be called to rasterize in z-order.
	///
	/// Culling responsibilities (clarified):
	/// - WorldShade performs world-space per-primitive culling (distance/frustum) and computes world-space properties for lighting.
	/// - ScreenShade performs screen-space clipping/culling and finalizes screen-space primitive properties used by rasterization.
	/// - ScreenProject computes per-vertex screen-space positions and depth for z-sorting. Implementations may skip work for vertices
	///   known to be unused by any potentially visible primitive (based on earlier culling), but any data required for FragmentCollect
	///   must be reliably available and stable for the remainder of the frame.
	///
	/// Fragment lifecycle and stability:
	/// - FragmentCollector stores only ObjectIndex, FragmentIndex (primitiveIndex), and Z. All other per-primitive data needed
	///   for rasterization must be cached inside the object.
	/// - The primitiveIndex recorded during FragmentCollect must remain valid and refer to the same primitive data until
	///   FragmentShade is invoked later in the same frame. Do not reorder, free, or invalidate that backing storage mid-frame.
	///
	/// Unified return convention for indexed stages (VertexShade, WorldTransform, WorldShade,
	/// CameraTransform, ScreenProject, ScreenShade):
	/// - Return true when the object has finished processing all indices for that stage (the engine
	///   will then advance to the next object for that stage).
	/// - Return false to indicate there are more indices to process; the engine will call the same
	///   stage again for the same object with the next index.
	///
	/// Threading / performance notes:
	/// - Methods are called from the single EngineRenderTask render thread; no internal locking is expected.
	/// - Avoid long computations in hot paths. Cache computed data in object-local buffers.
	/// </summary>
	struct IRenderObject
	{
		/// <summary>
		/// One-per-frame object preparation.
		/// Called at the start of each frame.
		/// Responsibilities:
		/// - Compute object-to-world transform (scale/rotate/translate).
		/// - Apply transform to object position.
		/// - Ensure required vertex/source data is loaded.
		/// - Compute LOD, bounding box and primitive flags.
		/// - Perform any object-level frustum culling.
		/// - Defer per-vertex work to indexed VertexShade/WorldTransform.
		/// </summary>
		/// <param name="frustum">Current view frustum (world-space).</param>
		virtual void ObjectShade(const frustum_t& frustum) = 0;

		/// <summary>
		/// Per-vertex animation / morph step. (Per-vertex stage)
		/// Called repeatedly before world transform.
		/// Responsibilities:
		/// - Apply skeletal/morph/animated offsets into the vertex buffer.
		/// Return semantics:
		/// - Return true when the object has finished processing all indices for this stage (engine will advance to the next object).
		/// - Return false to indicate there are more indices to process (engine will call this method again with the next index).
		/// </summary>
		/// <param name="vertexIndex">Vertex index.</param>
		/// <returns>True if finished; false to continue.</returns>
		virtual bool VertexShade(const uint16_t vertexIndex) = 0;

		/// <summary>
		/// Convert a vertex from local-space to world-space. (Per-vertex stage)
		/// Called repeatedly after VertexShade and before WorldShade.
		/// Responsibilities:
		/// - Apply object scale/rotation/translation from local-space.
		/// Return semantics:
		/// - Return true when the object has finished processing all indices for this stage (engine will advance to the next object).
		/// - Return false to indicate there are more indices to process (engine will call this method again with the next index).
		/// </summary>
		/// <param name="vertexIndex">Vertex index.</param>
		/// <returns>True if finished; false to continue.</returns>
		virtual bool WorldTransform(const uint16_t vertexIndex) = 0;

		/// <summary>
		/// Compute world-space properties and perform per-primitive world culling. (Per-primitive stage)
		/// Called repeatedly after WorldTransform and before CameraTransform.
		/// Responsibilities:
		/// - Compute properties needed for lighting (world position, normal).
		/// - Perform any internal per-primitive frustum/distance culling (world-space).
		/// Return semantics:
		/// - Return true when the object has finished processing all indices for this stage (engine will advance to the next object).
		/// - Return false to indicate there are more indices to process (engine will call this method again with the next index).
		/// </summary>
		/// <param name="frustum">Pre-computed view frustum for world-space checks.</param>
		/// <param name="primitiveIndex">Primitive index.</param>
		/// <returns>True if finished; false to continue.</returns>
		virtual bool WorldShade(const frustum_t& frustum, const uint16_t primitiveIndex) = 0;

		/// <summary>
		/// Transform world-space vertex into camera-space. (Per-vertex stage)
		/// Called repeatedly after WorldShade and before ScreenProject.
		/// Responsibilities:
		/// - Apply camera transform from world-space.
		/// Return semantics:
		/// - Return true when the object has finished processing all indices for this stage (engine will advance to the next object).
		/// - Return false to indicate there are more indices to process (engine will call this method again with the next index).
		/// </summary>
		/// <param name="transform">Pre-computed camera transform (cos/sin and translation).</param>
		/// <param name="vertexIndex">Vertex index.</param>
		/// <returns>True if finished; false to continue.</returns>
		virtual bool CameraTransform(const transform16_camera_t& transform, const uint16_t vertexIndex) = 0;

		/// <summary>
		/// Project camera-space into screen-space using the given ViewportProjector. (Per-vertex stage)
		/// Responsibilities:
		/// - Apply camera viewport transform.
		/// - Compute per-vertex screen-space positions and depth for z-sorting.
		/// - Implementations may skip work for vertices known to be unused by any potentially visible primitive,
		///   but any screen-space/depth data required by later stages (e.g., for FragmentCollect) must be available and stable.
		/// Return semantics:
		/// - Return true when the object has finished processing all indices for this stage (engine will advance to the next object).
		/// - Return false to indicate there are more indices to process (engine will call this method again with the next index).
		/// </summary>
		/// <param name="screenProjector">Projector configured for current viewport/FOV.</param>
		/// <param name="vertexIndex">Vertex index.</param>
		/// <returns>True if finished; false to continue.</returns>
		virtual bool ScreenProject(ViewportProjector& screenProjector, const uint16_t vertexIndex) = 0;

		/// <summary>
		/// Compute and cache final screen-space properties. (Per-primitive stage)
		/// Called after projection and before FragmentCollect.
		/// Responsibilities:
		/// - Compute properties needed by fragment shaders (primitive center, screen-space normal, etc.).
		/// - Perform off-screen clipping/culling and mark culled primitives accordingly (screen-space).
		/// - Cache any screen-space primitive data required later by FragmentShade.
		/// Return semantics:
		/// - Return true when the object has finished processing all indices for this stage (engine will advance to the next object).
		/// - Return false to indicate there are more indices to process (engine will call this method again with the next index).
		/// </summary>
		/// <param name="primitiveIndex">Primitive index.</param>
		/// <returns>True if finished; false to continue.</returns>
		virtual bool ScreenShade(const uint16_t primitiveIndex) = 0;

		/// <summary>
		/// Emit the object's primitives as z-ordered fragment entries to the provided FragmentCollector.
		/// Called once per object after all vertex and primitive processing is complete.
		/// Responsibilities:
		/// - Add z-ordered entries to fragment collector for each visible primitive.
		/// - The FragmentCollector only stores ObjectIndex, FragmentIndex (primitiveIndex), and Z.
		///   Implementations must ensure the recorded primitiveIndex remains valid and its backing data
		///   stays alive and un-reordered until rasterization completes for the frame.
		/// </summary>
		/// <param name="fragmentCollector">Collector to receive ordered_fragment_t entries.</param>
		virtual void FragmentCollect(FragmentCollector& fragmentCollector) = 0;

		/// <summary>
		/// Rasterize a previously collected fragment using the active fragment shader and rasterizer.
		/// Responsibilities:
		/// - Populate fragment with cached primitive properties.
		/// - Invoke IFragmentShader with ISceneShader to draw fragment, passing along the rasterizer.
		/// - Index corresponds into the object's fragment/primitive list and matches the primitiveIndex
		///   value previously emitted by FragmentCollect.
		/// - The object must not have reordered or invalidated the primitive storage since FragmentCollect.
		/// </summary>
		/// <param name="rasterizer">Rasterizer bound to the output surface.</param>
		/// <param name="primitiveIndex">Fragment index, same as primitive index as emitted by FragmentCollect.</param>
		virtual void FragmentShade(WindowRasterizer& rasterizer, const uint16_t primitiveIndex) = 0;
	};


	struct IEngineRenderer
	{
		virtual bool AddObject(IRenderObject* renderObject) = 0;
		virtual void ClearObjects() = 0;

		virtual camera_state_t* GetCameraControls() = 0;
		virtual void SetFov(const ufraction16_t fovFraction) = 0;
		virtual void GetRendererStatus(render_status_struct& rendererStatus) = 0;
#if defined(INTEGER_WORLD_PERFORMANCE_DEBUG)
		virtual void GetRendererStatus(render_debug_status_struct& rendererStatus) = 0;
#endif
	};
}
#endif