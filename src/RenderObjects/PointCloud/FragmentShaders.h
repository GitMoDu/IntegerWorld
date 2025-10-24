#ifndef _INTEGER_WORLD_RENDER_OBJECTS_POINT_CLOUD_FRAGMENT_SHADERS_h
#define _INTEGER_WORLD_RENDER_OBJECTS_POINT_CLOUD_FRAGMENT_SHADERS_h

#include "../AbstractObject.h"
#include "../../Shaders/Primitive/DepthSampler.h"

namespace IntegerWorld
{
	namespace RenderObjects
	{
		namespace PointCloud
		{
			namespace FragmentShaders
			{
				struct PointShader : IFragmentShader<point_cloud_fragment_t>
				{
					void FragmentShade(WindowRasterizer& rasterizer, const point_cloud_fragment_t& fragment)
					{
						rasterizer.DrawRectangle(Rgb8::Color(fragment.red, fragment.green, fragment.blue),
							fragment.x - 1, fragment.y - 1,
							fragment.x + 1, fragment.y + 1
						);
					}
				};

				struct PointZShader : IFragmentShader<point_cloud_fragment_t>
				{
					void FragmentShade(WindowRasterizer& rasterizer, const point_cloud_fragment_t& fragment)
					{
						const uint8_t gray = PrimitiveShaders::DepthSampler::ZDepth8(fragment.z);
						rasterizer.DrawRectangle(Rgb8::Color(gray, gray, gray),
							fragment.x - 1, fragment.y - 1,
							fragment.x + 1, fragment.y + 1
						);
					}
				};
			}
		}
	}
}
#endif