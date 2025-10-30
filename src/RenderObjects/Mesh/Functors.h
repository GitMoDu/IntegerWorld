#ifndef _INTEGER_WORLD_RENDER_OBJECT_MESH_FRAGMENT_SHADERS_FUNCTORS_h
#define _INTEGER_WORLD_RENDER_OBJECT_MESH_FRAGMENT_SHADERS_FUNCTORS_h

#include "AbstractObject.h"
#include "../../Shaders/Primitive/DepthSampler.h"
#include "../../Shaders/Primitive/TriangleSampler.h"
#include "../../Shaders/Primitive/UvInterpolator.h"

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
					template<typename TriangleSamplerType = PrimitiveShaders::TrianglePerspectiveCorrectSampler>
					struct ColorFunctor
					{
					private:
						TriangleSamplerType Sampler{};

					public:
						uint8_t Ra{}, Ga{}, Ba{}, Rb{}, Gb{}, Bb{}, Rc{}, Gc{}, Bc{};

					public:
						bool SetFragmentData(const mesh_vertex_fragment_t& fragment)
						{
							if (Sampler.SetFragmentData(fragment))
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
							const auto fractions = Sampler.Fractions(x, y);
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
						PrimitiveShaders::TriangleAffineSampler Sampler{};

					public:
						int16_t Az, Bz, Cz;

					public:
						bool SetFragmentData(const fragment_t& fragment)
						{
							if (Sampler.SetFragmentData(fragment))
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
							const auto fractions = Sampler.Fractions(x, y);

							const int16_t z = LimitValue<int32_t, 0, INT16_MAX>(
								static_cast<int32_t>(Fraction(fractions.FractionA, Az))
								+ Fraction(fractions.FractionB, Bz)
								+ Fraction(fractions.FractionC, Cz));

							const uint8_t gray = PrimitiveShaders::DepthSampler::ZDepth8(z);

							color = Rgb8::Color(gray, gray, gray);

							return true;
						}
					};

					namespace Texture
					{
						template<typename TextureSourceType,
							typename TriangleSamplerType = PrimitiveShaders::TriangleAffineSampler>
						class UnlitFunctor
						{
						private:
							TriangleSamplerType Sampler{};
							PrimitiveShaders::UvInterpolator UvInterpolator{};

						private:
							TextureSourceType& TextureSource;

						public:
							UnlitFunctor(TextureSourceType& textureSource)
								: TextureSource(textureSource)
							{
							}

							template<typename mesh_fragment_t>
							bool SetFragmentData(const mesh_fragment_t& fragment)
							{
								if (Sampler.SetFragmentData(fragment))
								{
									UvInterpolator.SetFragmentData(fragment);

									return true;
								}

								return false;
							}

							bool operator()(Rgb8::color_t& color, const int16_t x, const int16_t y)
							{
								const auto fractions = Sampler.Fractions(x, y);
								const coordinate_t uv = UvInterpolator.GetUv(fractions.FractionA, fractions.FractionB, fractions.FractionC);
								color = TextureSource.GetTexel(uv.x, uv.y);

								return true;
							}
						};

						template<typename TextureSourceType,
							typename TriangleSamplerType = PrimitiveShaders::TriangleAffineSampler,
							PrimitiveShaders::UvInterpolationMode uvInterpolationMode = PrimitiveShaders::UvInterpolationMode::Fast>
						class TriangleLitFunctor
						{
						private:
							TriangleSamplerType Sampler{};
							PrimitiveShaders::UvInterpolator UvInterpolator{};

						private:
							TextureSourceType& TextureSource;

						private:
							uint8_t R{}, G{}, B{};

						public:
							TriangleLitFunctor(TextureSourceType& textureSource)
								: TextureSource(textureSource)
							{
							}

							bool SetFragmentData(const mesh_triangle_fragment_t& fragment)
							{
								if (Sampler.SetFragmentData(fragment))
								{
									UvInterpolator.SetFragmentData(fragment);

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
									const auto fractions = Sampler.Fractions(x, y);
									coordinate_t uv;
									switch (uvInterpolationMode)
									{
									case PrimitiveShaders::UvInterpolationMode::Fast:
										uv = UvInterpolator.UvFast(fractions.FractionA, fractions.FractionB, fractions.FractionC);
										break;
									case PrimitiveShaders::UvInterpolationMode::Accurate:
									default:
										uv = UvInterpolator.UvAccurate(fractions.FractionA, fractions.FractionB, fractions.FractionC);
										break;
									}
									color = TextureSource.GetTexel(uv.x, uv.y);
								}

								color = Rgb8::Color(
									static_cast<uint8_t>(MinValue<uint16_t>(UINT8_MAX, (static_cast<uint16_t>(Rgb8::Red(color)) * R) >> 8)),
									static_cast<uint8_t>(MinValue<uint16_t>(UINT8_MAX, (static_cast<uint16_t>(Rgb8::Green(color)) * G) >> 8)),
									static_cast<uint8_t>(MinValue<uint16_t>(UINT8_MAX, (static_cast<uint16_t>(Rgb8::Blue(color)) * B) >> 8)));

								return true;
							}
						};

						template<typename TextureSourceType,
							typename TriangleSamplerType = PrimitiveShaders::TrianglePerspectiveCorrectSampler,
							PrimitiveShaders::UvInterpolationMode uvInterpolationMode = PrimitiveShaders::UvInterpolationMode::Fast>
						class VertexLitFunctor
						{
						private:
							TriangleSamplerType Sampler{};
							PrimitiveShaders::UvInterpolator UvInterpolator{};

						private:
							TextureSourceType& TextureSource;

						private:
							uint8_t Ra{}, Ga{}, Ba{}, Rb{}, Gb{}, Bb{}, Rc{}, Gc{}, Bc{};

						public:
							VertexLitFunctor(TextureSourceType& textureSource)
								: TextureSource(textureSource)
							{
							}

							bool SetFragmentData(const mesh_vertex_fragment_t& fragment)
							{
								if (Sampler.SetFragmentData(fragment))
								{
									UvInterpolator.SetFragmentData(fragment);
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
								const auto fractions = Sampler.Fractions(x, y);

								coordinate_t uv;
								switch (uvInterpolationMode)
								{
								case PrimitiveShaders::UvInterpolationMode::Fast:
									uv = UvInterpolator.UvFast(fractions.FractionA, fractions.FractionB, fractions.FractionC);
									break;
								case PrimitiveShaders::UvInterpolationMode::Accurate:
								default:
									uv = UvInterpolator.UvAccurate(fractions.FractionA, fractions.FractionB, fractions.FractionC);
									break;
								}

								uint8_t r;
								uint8_t g;
								uint8_t b;
								{
									const Rgb8::color_t texel = TextureSource.GetTexel(uv.x, uv.y);
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
}
#endif