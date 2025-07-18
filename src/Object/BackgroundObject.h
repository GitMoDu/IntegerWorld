#ifndef _INTEGER_WORLD_BACKGROUND_OBJECT_h
#define _INTEGER_WORLD_BACKGROUND_OBJECT_h

#include "AbstractObject.h"

namespace IntegerWorld
{
	class FlatBackgroundObject : public AbstractObject
	{
		using BaseObject = AbstractObject;

	public:
		using BaseObject::SceneShader;

	public:
		IFragmentShader<flat_background_fragment_t>* FragmentShader = nullptr;

		int16_t ZOffset = 0;
		material_t Material{ UFRACTION8_1X, 0, 0, 0 };
		color_fraction16_t Color{ UFRACTION16_1X / 8, UFRACTION16_1X / 8, UFRACTION16_1X / 8 };

	private:
		const vertex16_t* VerticesSource;

	private:
		flat_background_fragment_t BackgroundFragment{};

	public:
		FlatBackgroundObject()
			: BaseObject()
		{
		}

		bool VertexShade(const uint16_t index) { return true; }
		bool PrimitiveWorldShade(const uint16_t index) { return true; }
		bool CameraTransform(const transform32_rotate_translate_t& transform, const uint16_t index) { return true; }
		bool ScreenProject(ViewportProjector& screenProjector, const uint16_t index) { return true; }

		bool PrimitiveScreenShade(const uint16_t index, const uint16_t boundsWidth, const uint16_t boundsHeight) { return true; }

		void FragmentCollect(FragmentCollector& fragmentCollector)
		{
			if (FragmentShader != nullptr)
			{
				fragmentCollector.AddFragment(0, MinValue(int32_t(VERTEX16_RANGE), int32_t(VERTEX16_RANGE) + ZOffset));
			}
		}

		void FragmentShade(WindowRasterizer& rasterizer, const uint16_t index) final
		{
			BackgroundFragment.color = Color;
			BackgroundFragment.material = Material;
			if (SceneShader != nullptr)
			{
				FragmentShader->FragmentShade(rasterizer, BackgroundFragment, SceneShader);
			}
			else
			{
				FragmentShader->FragmentShade(rasterizer, BackgroundFragment);
			}
		}

	protected:
		void GetFragment(point_fragment_t& fragment, const uint16_t index)
		{
			fragment.color.r = UFRACTION16_1X;
			fragment.color.g = UFRACTION16_1X;
			fragment.color.b = UFRACTION16_1X;
			fragment.material.Emissive = 0;
			fragment.material.Diffuse = UFRACTION8_1X;
			fragment.material.Specular = 0;
			fragment.material.Metallic = 0;
		}
	};
}
#endif