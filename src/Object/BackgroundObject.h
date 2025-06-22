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

		int16_t GetZPosition() const final
		{
			return VERTEX16_RANGE + ZOffset;
		}

		bool FragmentShade(WindowRasterizer& rasterizer, const uint16_t index) final
		{
			if (FragmentShader != nullptr)
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

			return true;
		}

	protected:
		virtual void GetFragment(point_fragment_t& fragment, const uint16_t index)
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