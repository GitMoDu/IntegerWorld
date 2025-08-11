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

	struct IRenderObject
	{
		// Load and transforms vertices.
		virtual bool VertexShade(const uint16_t index) = 0;

		// Computes primitive properties (e.g., world position, normal).
		virtual bool PrimitiveWorldShade(const uint16_t index) = 0;

		// Apply camera transformation to each vertex.
		virtual bool CameraTransform(const camera_transform_t& transform, const uint16_t index) = 0;

		// Projects vertices to screen space.
		virtual bool ScreenProject(ViewportProjector& screenProjector, const uint16_t index) = 0;

		// Performs culling and sets up screen - space primitives.
		virtual bool PrimitiveScreenShade(const uint16_t index, const uint16_t boundsWidth, const uint16_t boundsHeight) = 0;

		// Collect z-indexed fragments for rasterization.
		virtual void FragmentCollect(FragmentCollector& fragmentCollector) = 0;

		// Rasterize z-ordered fragments to the screen.
		virtual void FragmentShade(WindowRasterizer& rasterizer, const uint16_t index) = 0;
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