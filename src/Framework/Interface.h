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
		virtual void Shade(color_fraction16_t& color, const material_t& material) {}
		virtual void Shade(color_fraction16_t& color, const material_t& material, const world_position_shade_t& shade) {}
		virtual void Shade(color_fraction16_t& color, const material_t& material, const world_position_normal_shade_t& shade) {}
	};

	struct ILightSource
	{
		virtual void GetLighting(color_fraction16_t& lightColor, ufraction16_t& diffuse, ufraction16_t& specular, const world_position_shade_t& shade) {}
		virtual void GetLighting(color_fraction16_t& lightColor, ufraction16_t& diffuse, ufraction16_t& specular, const world_position_normal_shade_t& shade) {}
	};

	template<typename fragment_t>
	struct IFragmentShader
	{
		// Shade with scene shader.
		virtual void FragmentShade(WindowRasterizer& rasterizer, const fragment_t& fragment, ISceneShader* sceneShader) {}

		// Shade without scene shader.
		virtual void FragmentShade(WindowRasterizer& rasterizer, const fragment_t& fragment) {}
	};

	struct IRenderObject
	{
		virtual bool VertexShade(const uint16_t index) { return true; }
		virtual bool PrimitiveWorldShade(const uint16_t index) { return true; }
		virtual bool CameraTransform(const transform32_rotate_translate_t& transform, const uint16_t index) { return true; }
		virtual bool ScreenProject(ViewportProjector& screenProjector, const uint16_t index) { return true; }
		virtual bool PrimitiveScreenShade(const uint16_t index, const uint16_t boundsWidth, const uint16_t boundsHeight) { return true; }
		virtual void FragmentCollect(FragmentCollector& fragmentCollector) {}
		virtual void FragmentShade(WindowRasterizer& rasterizer, const uint16_t index) { }
	};



	struct IEngineRenderer
	{
		virtual bool AddObject(IRenderObject* renderObject) { return false; }
		virtual void ClearObjects() {}

		virtual camera_state_t* GetCameraControls() { return nullptr; }
		virtual void SetFov(const ufraction16_t fovFraction) {}
		virtual void GetRendererStatus(render_status_struct& rendererStatus) {}
		virtual void GetRendererStatus(render_debug_status_struct& rendererStatus) {}
	};
}
#endif