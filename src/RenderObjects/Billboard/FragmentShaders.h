#ifndef _INTEGER_WORLD_RENDER_OBJECTS_BILLBOARD_FRAGMENT_SHADERS_h
#define _INTEGER_WORLD_RENDER_OBJECTS_BILLBOARD_FRAGMENT_SHADERS_h

#include "../AbstractObject.h"
#include "../../Shaders/Primitive/DepthSampler.h"

namespace IntegerWorld
{
	namespace RenderObjects
	{
		namespace Billboard
		{
			namespace FragmentShaders
			{
				struct PlaceholderShader final : IFragmentShader<billboard_fragment_t>
				{
				public:
					void FragmentShade(WindowRasterizer& rasterizer, const billboard_fragment_t& fragment)
					{
						const Rgb8::color_t color = Rgb8::Color(
							fragment.red,
							fragment.green,
							fragment.blue
						);

						// Rectangle edges
						rasterizer.DrawLine(color, fragment.topLeftX, fragment.topLeftY, fragment.bottomRightX, fragment.topLeftY);
						rasterizer.DrawLine(color, fragment.bottomRightX, fragment.topLeftY, fragment.bottomRightX, fragment.bottomRightY);
						rasterizer.DrawLine(color, fragment.bottomRightX, fragment.bottomRightY, fragment.topLeftX, fragment.bottomRightY);
						rasterizer.DrawLine(color, fragment.topLeftX, fragment.bottomRightY, fragment.topLeftX, fragment.topLeftY);

						// Diagonals
						rasterizer.DrawLine(color, fragment.topLeftX, fragment.topLeftY, fragment.bottomRightX, fragment.bottomRightY);
						rasterizer.DrawLine(color, fragment.bottomRightX, fragment.topLeftY, fragment.topLeftX, fragment.bottomRightY);
					}
				};

				struct InterpolateZShader final : IFragmentShader<billboard_fragment_t>
				{
					void FragmentShade(WindowRasterizer& rasterizer, const billboard_fragment_t& fragment)
					{
						const uint8_t gray = PrimitiveShaders::DepthSampler::ZDepth8(fragment.z);
						// Rectangle edges
						rasterizer.DrawLine(Rgb8::Color(gray, gray, gray), fragment.topLeftX, fragment.topLeftY, fragment.bottomRightX, fragment.topLeftY);
						rasterizer.DrawLine(Rgb8::Color(gray, gray, gray), fragment.bottomRightX, fragment.topLeftY, fragment.bottomRightX, fragment.bottomRightY);
						rasterizer.DrawLine(Rgb8::Color(gray, gray, gray), fragment.bottomRightX, fragment.bottomRightY, fragment.topLeftX, fragment.bottomRightY);
						rasterizer.DrawLine(Rgb8::Color(gray, gray, gray), fragment.topLeftX, fragment.bottomRightY, fragment.topLeftX, fragment.topLeftY);
						// Diagonals
						rasterizer.DrawLine(Rgb8::Color(gray, gray, gray), fragment.topLeftX, fragment.topLeftY, fragment.bottomRightX, fragment.bottomRightY);
						rasterizer.DrawLine(Rgb8::Color(gray, gray, gray), fragment.bottomRightX, fragment.topLeftY, fragment.topLeftX, fragment.bottomRightY);
					}
				};

				struct FillShader final : IFragmentShader<billboard_fragment_t>
				{
					void FragmentShade(WindowRasterizer& rasterizer, const billboard_fragment_t& fragment)
					{
						rasterizer.DrawRectangle(Rgb8::Color(
							fragment.red,
							fragment.green,
							fragment.blue),
							fragment.topLeftX, fragment.topLeftY,
							fragment.bottomRightX, fragment.bottomRightY);
					}
				};

				struct FillZShader final : IFragmentShader<billboard_fragment_t>
				{
					void FragmentShade(WindowRasterizer& rasterizer, const billboard_fragment_t& fragment)
					{
						const uint8_t gray = PrimitiveShaders::DepthSampler::ZDepth8(fragment.z);
						rasterizer.DrawRectangle(Rgb8::Color(gray, gray, gray),
							fragment.topLeftX, fragment.topLeftY,
							fragment.bottomRightX, fragment.bottomRightY);
					}
				};
			}
		}
	}
}
#endif