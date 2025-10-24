#ifndef _INTEGER_WORLD_RENDER_OBJECT_MESH_FRAGMENT_SHADERS_FUNCTORS_h
#define _INTEGER_WORLD_RENDER_OBJECT_MESH_FRAGMENT_SHADERS_FUNCTORS_h

#include "AbstractObject.h"
#include "../../Shaders/Primitive/DepthSampler.h"
#include "../../Shaders/Primitive/TriangleSampler.h"

namespace IntegerWorld
{
	namespace RenderObjects
	{
		namespace Mesh
		{
			namespace FragmentShaders
			{
				namespace Functors
				{
					struct ColorFunctor
					{
					private:
						PrimitiveShaders::TriangleSampler Sampler{};

					public:
						uint8_t Ra, Ga, Ba, Rb, Gb, Bb, Rc, Gc, Bc;

					public:
						bool SetFragmentData(const mesh_vertex_fragment_t& fragment)
						{
							if (Sampler.SetTriangle(fragment.vertexA, fragment.vertexB, fragment.vertexC))
							{
								Ra = fragment.redA;
								Ga = fragment.greenA;
								Ba = fragment.blueA;

								Rb = fragment.redB;
								Gb = fragment.greenB;
								Bb = fragment.blueB;

								Rc = fragment.redC;
								Gc = fragment.greenC;
								Bc = fragment.blueC;
								return true;
							}

							return false;
						}

						bool operator()(Rgb8::color_t& color, const int16_t x, const int16_t y)
						{
							const auto fractions = Sampler.U8Fractions(x, y);
							color = Rgb8::Color(
								MinValue<uint16_t>(UINT8_MAX, static_cast<uint16_t>(Fraction(fractions.FractionA, Ra)) + Fraction(fractions.FractionB, Rb) + Fraction(fractions.FractionC, Rc)),
								MinValue<uint16_t>(UINT8_MAX, static_cast<uint16_t>(Fraction(fractions.FractionA, Ga)) + Fraction(fractions.FractionB, Gb) + Fraction(fractions.FractionC, Gc)),
								MinValue<uint16_t>(UINT8_MAX, static_cast<uint16_t>(Fraction(fractions.FractionA, Ba)) + Fraction(fractions.FractionB, Bb) + Fraction(fractions.FractionC, Bc)));

							return true;
						}
					};

					template<typename fragment_t = mesh_triangle_fragment_t>
					struct ZFunctor
					{
					private:
						PrimitiveShaders::TriangleSampler Sampler{};

					public:
						int16_t Az, Bz, Cz;

					public:
						bool SetFragmentData(const fragment_t& fragment)
						{
							if (Sampler.SetTriangle(fragment.vertexA, fragment.vertexB, fragment.vertexC))
							{
								Az = fragment.vertexA.z;
								Bz = fragment.vertexB.z;
								Cz = fragment.vertexC.z;

								return true;
							}

							return false;
						}

						bool operator()(Rgb8::color_t& color, const int16_t x, const int16_t y)
						{
							const auto fractions = Sampler.U16Fractions(x, y);

							const int16_t z = LimitValue<int32_t, 0, INT16_MAX>(
								static_cast<int32_t>(Fraction(fractions.FractionA, Az))
								+ Fraction(fractions.FractionB, Bz)
								+ Fraction(fractions.FractionC, Cz));

							const uint8_t gray = PrimitiveShaders::DepthSampler::ZDepth8(z);

							color = Rgb8::Color(gray, gray, gray);

							return true;
						}
					};

					class AbstractTextureFunctor
					{
					protected:
						PrimitiveShaders::TriangleSampler Sampler{};

					protected:
						coordinate_t UvA{};
						coordinate_t UvB{};
						coordinate_t UvC{};

					protected:
						coordinate_t GetUv(const int16_t x, const int16_t y) const
						{
							const auto fractions = Sampler.U8Fractions(x, y);

							return coordinate_t{
								MaxValue<int16_t>(0, Fraction(fractions.FractionA, UvA.x)
									+ Fraction(fractions.FractionB, UvB.x)
									+ Fraction(fractions.FractionC, UvC.x)),
								MaxValue<int16_t>(0, Fraction(fractions.FractionA, UvA.y)
									+ Fraction(fractions.FractionB, UvB.y)
									+ Fraction(fractions.FractionC, UvC.y)) };
						}
					};

					template<typename TextureSourceType>
					class TextureFunctor : public AbstractTextureFunctor
					{
					private:
						using Base = AbstractTextureFunctor;

					private:
						TextureSourceType& TextureSource;

					public:
						TextureFunctor(TextureSourceType& textureSource)
							: Base()
							, TextureSource(textureSource)
						{
						}

						bool SetFragmentData(const mesh_triangle_fragment_t& fragment)
						{
							if (Sampler.SetTriangle(fragment.vertexA, fragment.vertexB, fragment.vertexC))
							{
								UvA = fragment.uvA;
								UvB = fragment.uvB;
								UvC = fragment.uvC;

								return true;
							}

							return false;
						}

						bool operator()(Rgb8::color_t& color, const int16_t x, const int16_t y)
						{
							const coordinate_t uv = Base::GetUv(x, y);
							color = TextureSource.GetTexel(uv.x, uv.y);

							return true;
						}
					};

					template<typename TextureSourceType, typename fragment_t = mesh_triangle_fragment_t>
					class TextureTriangleLitFunctor : public AbstractTextureFunctor
					{
					private:
						using Base = AbstractTextureFunctor;

					private:
						TextureSourceType& TextureSource;

					private:
						uint8_t R = 0;
						uint8_t G = 0;
						uint8_t B = 0;

					public:
						TextureTriangleLitFunctor(TextureSourceType& textureSource)
							: Base()
							, TextureSource(textureSource)
						{
						}

						bool SetFragmentData(const fragment_t& fragment)
						{
							if (Sampler.SetTriangle(fragment.vertexA, fragment.vertexB, fragment.vertexC))
							{
								UvA = fragment.uvA;
								UvB = fragment.uvB;
								UvC = fragment.uvC;

								R = fragment.red;
								G = fragment.green;
								B = fragment.blue;

								return true;
							}

							return false;
						}

						bool operator()(Rgb8::color_t& color, const int16_t x, const int16_t y)
						{
							{
								const coordinate_t uv = Base::GetUv(x, y);
								color = TextureSource.GetTexel(uv.x, uv.y);
							}

							color = Rgb8::Color(
								static_cast<uint8_t>(MinValue<uint16_t>(UINT8_MAX, (static_cast<uint16_t>(Rgb8::Red(color)) * R) >> 8)),
								static_cast<uint8_t>(MinValue<uint16_t>(UINT8_MAX, (static_cast<uint16_t>(Rgb8::Green(color)) * G) >> 8)),
								static_cast<uint8_t>(MinValue<uint16_t>(UINT8_MAX, (static_cast<uint16_t>(Rgb8::Blue(color)) * B) >> 8)));

							return true;
						}
					};


					template<typename TextureSourceType>
					struct TextureVertexLitFunctor
					{
					private:
						PrimitiveShaders::TriangleSampler Sampler{};

					private:
						TextureSourceType& TextureSource;

					private:
						coordinate_t UvA{};
						coordinate_t UvB{};
						coordinate_t UvC{};

					private:
						uint8_t Ra, Ga, Ba, Rb, Gb, Bb, Rc, Gc, Bc;

					public:
						TextureVertexLitFunctor(TextureSourceType& textureSource)
							: TextureSource(textureSource)
						{
						}

						bool SetFragmentData(const mesh_vertex_fragment_t& fragment)
						{
							if (Sampler.SetTriangle(fragment.vertexA, fragment.vertexB, fragment.vertexC))
							{
								UvA = fragment.uvA;
								UvB = fragment.uvB;
								UvC = fragment.uvC;

								Ra = fragment.redA;
								Ga = fragment.greenA;
								Ba = fragment.blueA;

								Rb = fragment.redB;
								Gb = fragment.greenB;
								Bb = fragment.blueB;

								Rc = fragment.redC;
								Gc = fragment.greenC;
								Bc = fragment.blueC;

								return true;
							}

							return false;
						}

						bool operator()(Rgb8::color_t& color, const int16_t x, const int16_t y)
						{
							const auto fractions = Sampler.U16Fractions(x, y);

							uint8_t r;
							uint8_t g;
							uint8_t b;
							{
								const uint16_t u = MaxValue<int16_t>(0, Fraction(fractions.FractionA, UvA.x)
									+ Fraction(fractions.FractionB, UvB.x)
									+ Fraction(fractions.FractionC, UvC.x));
								const uint16_t v = MaxValue<int16_t>(0, Fraction(fractions.FractionA, UvA.y)
									+ Fraction(fractions.FractionB, UvB.y)
									+ Fraction(fractions.FractionC, UvC.y));

								const Rgb8::color_t texel = TextureSource.GetTexel(u, v);

								r = Rgb8::Red(texel);
								g = Rgb8::Green(texel);
								b = Rgb8::Blue(texel);
							}

							r = MinValue<uint16_t>(UINT8_MAX,
								((static_cast<uint16_t>(Fraction(fractions.FractionA, Ra))
									+ Fraction(fractions.FractionB, Rb)
									+ Fraction(fractions.FractionC, Rc)) * r) >> 8);

							g = MinValue<uint16_t>(UINT8_MAX,
								((static_cast<uint16_t>(Fraction(fractions.FractionA, Ga))
									+ Fraction(fractions.FractionB, Gb)
									+ Fraction(fractions.FractionC, Gc)) * g) >> 8);

							b = MinValue<uint16_t>(UINT8_MAX,
								((static_cast<uint16_t>(Fraction(fractions.FractionA, Ba))
									+ Fraction(fractions.FractionB, Bb)
									+ Fraction(fractions.FractionC, Bc)) * b) >> 8);

							color = Rgb8::Color(r, g, b);

							return true;
						}
					};
				}
			}
		}
	}
}
#endif