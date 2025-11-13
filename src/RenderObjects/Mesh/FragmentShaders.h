#ifndef _INTEGER_WORLD_OBJECT_RENDER_OBJECTS_MESH_FRAGMENT_SHADERS_h
#define _INTEGER_WORLD_OBJECT_RENDER_OBJECTS_MESH_FRAGMENT_SHADERS_h

#include "../AbstractObject.h"
#include "../../Shaders/Primitive/DepthSampler.h"
#include "PixelShaders.h"
#include "../Edge/PixelShaders.h"

namespace IntegerWorld
{
	namespace RenderObjects
	{
		namespace Mesh
		{
			namespace FragmentShaders
			{
				namespace Abstract
				{
					template<typename fragment_t,
						typename TextureSourceType,
						typename TexturePixelShaderType = PixelShaders::TextureUnlit<TextureSourceType>,
						pixel_blend_mode_t blendMode = pixel_blend_mode_t::Replace>
					class TemplateTextureShader : public IFragmentShader<fragment_t>
					{
					private:
						TexturePixelShaderType PixelShader;

					public:
						TemplateTextureShader(TextureSourceType& textureSource)
							: PixelShader(textureSource)
						{
						}

						void FragmentShade(WindowRasterizer& rasterizer, const fragment_t& fragment)
						{
							if (PixelShader.SetFragmentData(fragment))
							{
								rasterizer.RasterTriangle<blendMode>(fragment.vertexA, fragment.vertexB, fragment.vertexC, PixelShader);
							}
						}
					};
				}

				namespace TriangleShade
				{
					template<typename TextureSourceType,
						typename TexturePixelShaderType = PixelShaders::TextureUnlit<TextureSourceType>>
						using TemplateTextureShader = Abstract::TemplateTextureShader<mesh_triangle_fragment_t, TextureSourceType, TexturePixelShaderType>;

					struct FillShader : IFragmentShader<mesh_triangle_fragment_t>
					{
						void FragmentShade(WindowRasterizer& rasterizer, const mesh_triangle_fragment_t& fragment)
						{
							rasterizer.DrawTriangle(Rgb8::Color(fragment.red, fragment.green, fragment.blue),
								fragment.vertexA, fragment.vertexB, fragment.vertexC);
						}
					};

					struct FillZShader : IFragmentShader<mesh_triangle_fragment_t>
					{
						void FragmentShade(WindowRasterizer& rasterizer, const mesh_triangle_fragment_t& fragment)
						{
							const uint8_t gray = PrimitiveShaders::DepthSampler::ZDepth8(fragment.z);

							rasterizer.DrawTriangle(Rgb8::Color(gray, gray, gray),
								fragment.vertexA, fragment.vertexB, fragment.vertexC);
						}
					};

					class ZInterpolateShader : public IFragmentShader<mesh_triangle_fragment_t>
					{
					private:
						PixelShaders::ZInterpolate<mesh_triangle_fragment_t> PixelShader{};

					public:
						void FragmentShade(WindowRasterizer& rasterizer, const mesh_triangle_fragment_t& fragment)
						{
							if (PixelShader.SetFragmentData(fragment))
							{
								rasterizer.RasterTriangle(fragment.vertexA, fragment.vertexB, fragment.vertexC, PixelShader);
							}
						}
					};

					struct WireframeShader : IFragmentShader<mesh_triangle_fragment_t>
					{
						void FragmentShade(WindowRasterizer& rasterizer, const mesh_triangle_fragment_t& fragment)
						{
							const Rgb8::color_t color = Rgb8::Color(fragment.red, fragment.green, fragment.blue);
							rasterizer.DrawLine(color, fragment.vertexA, fragment.vertexB);
							rasterizer.DrawLine(color, fragment.vertexA, fragment.vertexC);
							rasterizer.DrawLine(color, fragment.vertexB, fragment.vertexC);
						}
					};
				}

				namespace VertexShade
				{
					template<typename TextureSourceType,
						typename TexturePixelShaderType = PixelShaders::TextureUnlit<TextureSourceType>,
						pixel_blend_mode_t blendMode = pixel_blend_mode_t::Replace>
					using TemplateTextureShader = Abstract::TemplateTextureShader<mesh_vertex_fragment_t, TextureSourceType, TexturePixelShaderType, blendMode>;

					template<typename TriangleSamplerType = PrimitiveShaders::TriangleAffineSampler,
						pixel_blend_mode_t blendMode = pixel_blend_mode_t::Replace>
					class ColorInterpolateShader : public IFragmentShader<mesh_vertex_fragment_t>
					{
					private:
						PixelShaders::VertexColorInterpolate<TriangleSamplerType> PixelShader{};

					public:
						void FragmentShade(WindowRasterizer& rasterizer, const mesh_vertex_fragment_t& fragment)
						{
							if (PixelShader.SetFragmentData(fragment))
							{
								rasterizer.RasterTriangle<blendMode>(fragment.vertexA, fragment.vertexB, fragment.vertexC, PixelShader);
							}
						}
					};

					template<typename TriangleSamplerType = PrimitiveShaders::TriangleAffineSampler>
					class ZInterpolateShader : public IFragmentShader<mesh_vertex_fragment_t>
					{
					private:
						PixelShaders::ZInterpolate<mesh_vertex_fragment_t, TriangleSamplerType> PixelShader{};

					public:
						void FragmentShade(WindowRasterizer& rasterizer, const mesh_vertex_fragment_t& fragment)
						{
							if (PixelShader.SetFragmentData(fragment))
							{
								rasterizer.RasterTriangle(fragment.vertexA, fragment.vertexB, fragment.vertexC, PixelShader);
							}
						}
					};

					class WireframeShader : public IFragmentShader<mesh_vertex_fragment_t>
					{
					private:
						Edge::PixelShaders::VertexColorInterpolate PixelShader{};

					public:
						void FragmentShade(WindowRasterizer& rasterizer, const mesh_vertex_fragment_t& fragment)
						{
							if (PixelShader.SetFragmentData(fragment.vertexA, fragment.vertexB,
								Rgb8::Color(fragment.redA, fragment.greenA, fragment.blueA),
								Rgb8::Color(fragment.redB, fragment.greenB, fragment.blueB)))
							{
								rasterizer.RasterLine<>(fragment.vertexA, fragment.vertexB, PixelShader);

								if (PixelShader.SetFragmentData(fragment.vertexA, fragment.vertexC,
									Rgb8::Color(fragment.redA, fragment.greenA, fragment.blueA),
									Rgb8::Color(fragment.redC, fragment.greenC, fragment.blueC)))
								{
									rasterizer.RasterLine<>(fragment.vertexA, fragment.vertexC, PixelShader);

									if (PixelShader.SetFragmentData(fragment.vertexB, fragment.vertexC,
										Rgb8::Color(fragment.redB, fragment.greenB, fragment.blueB),
										Rgb8::Color(fragment.redC, fragment.greenC, fragment.blueC)))
									{
										rasterizer.RasterLine<>(fragment.vertexB, fragment.vertexC, PixelShader);
									}
								}
							}
						}
					};
				}
			}
		}
	}
}
#endif