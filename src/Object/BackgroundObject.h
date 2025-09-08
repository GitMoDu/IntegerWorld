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
		Rgb8::color_t Color = Rgb8::Color(UINT8_MAX / 8, UINT8_MAX / 8, UINT8_MAX / 8);

	private:
		const vertex16_t* VerticesSource;

	private:
		flat_background_fragment_t BackgroundFragment{};

	public:
		FlatBackgroundObject()
			: BaseObject()
		{
		}

		void ObjectShade(const frustum_t& frustum) {}
		bool WorldTransform(const uint16_t vertexIndex) { return true; }
		bool WorldShade(const frustum_t& frustum, const uint16_t primitiveIndex) { return true; }
		bool CameraTransform(const transform16_camera_t& transform, const uint16_t vertexIndex) { return true; }
		bool ScreenProject(ViewportProjector& screenProjector, const uint16_t vertexIndex) { return true; }

		bool ScreenShade(const uint16_t primitiveIndex) { return true; }

		void FragmentCollect(FragmentCollector& fragmentCollector)
		{
			fragmentCollector.AddFragment(0, MinValue(int32_t(VERTEX16_RANGE), int32_t(VERTEX16_RANGE) + ZOffset));
		}

		void FragmentShade(WindowRasterizer& rasterizer, const uint16_t primitiveIndex) final
		{
			BackgroundFragment.color = Color;
			BackgroundFragment.material = Material;
			if (FragmentShader != nullptr)
			{
				FragmentShader->FragmentShade(rasterizer, BackgroundFragment, SceneShader);
			}
		}

	protected:
		void GetFragment(point_normal_fragment_t& fragment, const uint16_t primitiveIndex)
		{
			fragment.color = Rgb8::WHITE;
			fragment.material = { 0, UFRACTION8_1X , 0,0 };
			fragment.normal = { 0, 0, 0 };
		}
	};
}
#endif