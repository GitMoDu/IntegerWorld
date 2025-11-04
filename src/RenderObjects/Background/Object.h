#ifndef _INTEGER_WORLD_RENDER_OBJECTS_BACKGROUND_OBJECT_h
#define _INTEGER_WORLD_RENDER_OBJECTS_BACKGROUND_OBJECT_h

#include "../AbstractObject.h"

namespace IntegerWorld
{
	namespace RenderObjects
	{
		namespace Background
		{
			class FillObject : public IRenderObject
			{
			public:
				IFragmentShader<color_fragment_t>* FragmentShader = nullptr;

				Rgb8::color_t Color = Rgb8::BLACK;
				int16_t ZOffset = 0;

			public:
				FillObject() : IRenderObject() {}

				void ObjectShade(const frustum_t& frustum) {}
				bool VertexShade(const uint16_t vertexIndex) { return true; }
				bool WorldTransform(const uint16_t vertexIndex) { return true; }
				bool WorldShade(const frustum_t& frustum, const uint16_t primitiveIndex) { return true; }
				bool CameraTransform(const transform16_camera_t& transform, const uint16_t vertexIndex) { return true; }
				bool ScreenProject(ViewportProjector& screenProjector, const uint16_t vertexIndex) { return true; }
				bool ScreenShade(const uint16_t primitiveIndex) { return true; }

				void FragmentCollect(FragmentCollector& fragmentCollector)
				{
					fragmentCollector.AddFragment(0, MinValue<int32_t>(VERTEX16_RANGE, static_cast<int32_t>(VERTEX16_RANGE) + ZOffset));
				}

				void FragmentShade(WindowRasterizer& rasterizer, const uint16_t primitiveIndex) final
				{
					if (primitiveIndex == 0)
					{
						const color_fragment_t backgroundFragment{
							Rgb8::Red(Color),
							Rgb8::Green(Color),
							Rgb8::Blue(Color),
							static_cast<int16_t>(MinValue<int32_t>(VERTEX16_RANGE, static_cast<int32_t>(VERTEX16_RANGE) + ZOffset)) };

						if (FragmentShader != nullptr)
						{
							FragmentShader->FragmentShade(rasterizer, backgroundFragment);
						}
					}
				}
			};
		}
	}
}
#endif