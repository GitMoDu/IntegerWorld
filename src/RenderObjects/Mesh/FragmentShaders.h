#ifndef _INTEGER_WORLD_OBJECT_RENDER_OBJECTS_MESH_FRAGMENT_SHADERS_h
#define _INTEGER_WORLD_OBJECT_RENDER_OBJECTS_MESH_FRAGMENT_SHADERS_h

#include "../AbstractObject.h"
#include "../../Shaders/Primitive/DepthSampler.h"
#include "Functors.h"
#include "../Edge/Functors.h"

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
						typename TextureFunctorType = Functors::Texture::UnlitFunctor<TextureSourceType>>
						class TemplateTextureShader : public IFragmentShader<fragment_t>
					{
					private:
						TextureFunctorType Functor;

					public:
						TemplateTextureShader(TextureSourceType& textureSource)
							: Functor(textureSource)
						{
						}

						void FragmentShade(WindowRasterizer& rasterizer, const fragment_t& fragment)
						{
							if (Functor.SetFragmentData(fragment))
							{
								rasterizer.RasterTriangle(fragment.vertexA, fragment.vertexB, fragment.vertexC, Functor);
							}
						}
					};
				}


				namespace TriangleShade
				{
					template<typename TextureSourceType,
						typename TextureFunctorType = Functors::Texture::UnlitFunctor<TextureSourceType>>
						using TemplateTextureShader = Abstract::TemplateTextureShader<mesh_triangle_fragment_t, TextureSourceType, TextureFunctorType>;

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
						Functors::ZFunctor<mesh_triangle_fragment_t> Functor{};

					public:
						void FragmentShade(WindowRasterizer& rasterizer, const mesh_triangle_fragment_t& fragment)
						{
							if (Functor.SetFragmentData(fragment))
							{
								rasterizer.RasterTriangle(fragment.vertexA, fragment.vertexB, fragment.vertexC, Functor);
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
						typename TextureFunctorType = Functors::Texture::UnlitFunctor<TextureSourceType>>
						using TemplateTextureShader = Abstract::TemplateTextureShader<mesh_vertex_fragment_t, TextureSourceType, TextureFunctorType>;

					class ColorInterpolateShader : public IFragmentShader<mesh_vertex_fragment_t>
					{
					private:
						Functors::ColorFunctor Functor{};

					public:
						void FragmentShade(WindowRasterizer& rasterizer, const mesh_vertex_fragment_t& fragment)
						{
							if (Functor.SetFragmentData(fragment))
							{
								rasterizer.RasterTriangle(fragment.vertexA, fragment.vertexB, fragment.vertexC, Functor);
							}
						}
					};

					class ZInterpolateShader : public IFragmentShader<mesh_vertex_fragment_t>
					{
					private:
						Functors::ZFunctor<mesh_vertex_fragment_t> Functor{};

					public:
						void FragmentShade(WindowRasterizer& rasterizer, const mesh_vertex_fragment_t& fragment)
						{
							if (Functor.SetFragmentData(fragment))
							{
								rasterizer.RasterTriangle(fragment.vertexA, fragment.vertexB, fragment.vertexC, Functor);
							}
						}
					};

					class WireframeShader : public IFragmentShader<mesh_vertex_fragment_t>
					{
					private:
						Edge::FragmentShaders::Functors::ColorFunctor Functor{};

					public:
						void FragmentShade(WindowRasterizer& rasterizer, const mesh_vertex_fragment_t& fragment)
						{
							if (Functor.SetFragmentData(fragment.vertexA, fragment.vertexB,
								Rgb8::Color(fragment.redA, fragment.greenA, fragment.blueA),
								Rgb8::Color(fragment.redB, fragment.greenB, fragment.blueB)))
							{
								rasterizer.RasterLine(fragment.vertexA, fragment.vertexB, Functor);

								if (Functor.SetFragmentData(fragment.vertexA, fragment.vertexC,
									Rgb8::Color(fragment.redA, fragment.greenA, fragment.blueA),
									Rgb8::Color(fragment.redC, fragment.greenC, fragment.blueC)))
								{
									rasterizer.RasterLine(fragment.vertexA, fragment.vertexC, Functor);

									if (Functor.SetFragmentData(fragment.vertexB, fragment.vertexC,
										Rgb8::Color(fragment.redB, fragment.greenB, fragment.blueB),
										Rgb8::Color(fragment.redC, fragment.greenC, fragment.blueC)))
									{
										rasterizer.RasterLine(fragment.vertexB, fragment.vertexC, Functor);
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