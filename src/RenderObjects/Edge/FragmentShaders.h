#ifndef _INTEGER_WORLD_OBJECT_RENDER_OBJECTS_EDGE_FRAGMENT_SHADERS_h
#define _INTEGER_WORLD_OBJECT_RENDER_OBJECTS_EDGE_FRAGMENT_SHADERS_h

#include "../AbstractObject.h"
#include "../../Shaders/Primitive/DepthSampler.h"
#include "PixelShaders.h"

namespace IntegerWorld
{
	namespace RenderObjects
	{
		namespace Edge
		{
			namespace FragmentShaders
			{
				namespace LineShade
				{
					struct FillShader : IFragmentShader<edge_line_fragment_t>
					{
						void FragmentShade(WindowRasterizer& rasterizer, const edge_line_fragment_t& fragment)
						{
							rasterizer.DrawLine(Rgb8::Color(fragment.red, fragment.green, fragment.blue),
								fragment.vertexA, fragment.vertexB);
						}
					};

					struct ColorInterpolateShader : IFragmentShader<edge_line_fragment_t>
					{
						void FragmentShade(WindowRasterizer& rasterizer, const edge_line_fragment_t& fragment)
						{

						}
					};

					class ZShader : public IFragmentShader<edge_line_fragment_t>
					{
					public:
						void FragmentShade(WindowRasterizer& rasterizer, const edge_line_fragment_t& fragment)
						{
							const uint8_t gray = PrimitiveShaders::DepthSampler::ZDepth8(fragment.z);
							rasterizer.DrawLine(Rgb8::Color(gray, gray, gray),
								fragment.vertexA, fragment.vertexB);
						}
					};

					class ZInterpolateShader : public IFragmentShader<edge_line_fragment_t>
					{
					private:
						PixelShaders::ZInterpolate<edge_line_fragment_t> PixelShader{};

					public:
						void FragmentShade(WindowRasterizer& rasterizer, const edge_line_fragment_t& fragment)
						{
							PixelShader.SetFragmentData(fragment);
							rasterizer.RasterLine(fragment.vertexA, fragment.vertexB, PixelShader);
						}
					};
				}

				namespace VertexShade
				{
					template<pixel_blend_mode_t blendMode = pixel_blend_mode_t::Replace>
					class ColorInterpolateShader : public IFragmentShader<edge_vertex_fragment_t>
					{
					private:
						PixelShaders::VertexColorInterpolate PixelShader{};

					public:
						void FragmentShade(WindowRasterizer& rasterizer, const edge_vertex_fragment_t& fragment)
						{
							if (PixelShader.SetFragmentData(
								fragment.vertexA,
								fragment.vertexB,
								Rgb8::Color(fragment.redA, fragment.greenA, fragment.blueA),
								Rgb8::Color(fragment.redB, fragment.greenB, fragment.blueB)))
							{
								rasterizer.RasterLine(fragment.vertexA, fragment.vertexB, PixelShader);
							}
						}
					};

					class ZInterpolateShader : public IFragmentShader<edge_vertex_fragment_t>
					{
					private:
						PixelShaders::ZInterpolate<edge_vertex_fragment_t> PixelShader{};

					public:
						void FragmentShade(WindowRasterizer& rasterizer, const edge_vertex_fragment_t& fragment)
						{
							PixelShader.SetFragmentData(fragment);
							rasterizer.RasterLine(fragment.vertexA, fragment.vertexB, PixelShader);
						}
					};
				}
			}
		}
	}
}
#endif