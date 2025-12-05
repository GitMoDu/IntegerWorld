#ifndef _INTEGER_WORLD_RENDER_OBJECT_MESH_PIXEL_SHADERS_h
#define _INTEGER_WORLD_RENDER_OBJECT_MESH_PIXEL_SHADERS_h

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
			namespace PixelShaders
			{
				template<typename TriangleSamplerType = PrimitiveShaders::TriangleAffineSampler>
				class VertexColorInterpolate
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

					Rgb8::color_t operator()(const int16_t x, const int16_t y)
					{
						const auto fractions = Sampler.GetWeights(x, y).GetFractions();

						return Rgb8::Color(
							MinValue<uint16_t>(UINT8_MAX, static_cast<uint16_t>(Fraction(fractions.FractionA, Ra)) + Fraction(fractions.FractionB, Rb) + Fraction(fractions.FractionC, Rc)),
							MinValue<uint16_t>(UINT8_MAX, static_cast<uint16_t>(Fraction(fractions.FractionA, Ga)) + Fraction(fractions.FractionB, Gb) + Fraction(fractions.FractionC, Gc)),
							MinValue<uint16_t>(UINT8_MAX, static_cast<uint16_t>(Fraction(fractions.FractionA, Ba)) + Fraction(fractions.FractionB, Bb) + Fraction(fractions.FractionC, Bc)));
					}
				};

				template<typename fragment_t = mesh_triangle_fragment_t,
					typename TriangleSamplerType = PrimitiveShaders::TriangleAffineSampler>
				class ZInterpolate
				{
				private:
					TriangleSamplerType Sampler{};

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

					Rgb8::color_t operator()(const int16_t x, const int16_t y)
					{
						const auto fractions = Sampler.GetWeights(x, y).GetFractions();

						const int16_t z = LimitValue<int32_t, 0, INT16_MAX>(
							static_cast<int32_t>(Fraction(fractions.FractionA, Az))
							+ Fraction(fractions.FractionB, Bz)
							+ Fraction(fractions.FractionC, Cz));

						const uint8_t gray = PrimitiveShaders::DepthSampler::ZDepth8(z);

						return Rgb8::Color(gray, gray, gray);
					}
				};

				template<typename TextureSourceType,
					typename TriangleSamplerType = PrimitiveShaders::TriangleAffineSampler,
					PrimitiveShaders::UvInterpolationModeEnum uvInterpolationMode = PrimitiveShaders::UvInterpolationModeEnum::Fast>
				class TextureUnlit
				{
				private:
					TriangleSamplerType Sampler{};
					PrimitiveShaders::UvInterpolator UvInterpolator{};
					TextureSourceType& TextureSource;

				public:
					TextureUnlit(TextureSourceType& textureSource)
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

					Rgb8::color_t operator()(const int16_t x, const int16_t y)
					{
						const auto fractions = Sampler.GetWeights(x, y).GetFractions();
						coordinate_t uv;
						switch (uvInterpolationMode)
						{
						case PrimitiveShaders::UvInterpolationModeEnum::Fast:
							uv = UvInterpolator.UvFast(fractions.FractionA, fractions.FractionB, fractions.FractionC);
							break;
						case PrimitiveShaders::UvInterpolationModeEnum::Accurate:
						default:
							uv = UvInterpolator.UvAccurate(fractions.FractionA, fractions.FractionB, fractions.FractionC);
							break;
						}

						return TextureSource.GetTexel(uv.x, uv.y);
					}
				};

				template<typename TextureSourceType,
					typename TriangleSamplerType = PrimitiveShaders::TriangleAffineSampler,
					PrimitiveShaders::UvInterpolationModeEnum uvInterpolationMode = PrimitiveShaders::UvInterpolationModeEnum::Fast>
				class TextureTriangleLit
				{
				private:
					TriangleSamplerType Sampler{};
					PrimitiveShaders::UvInterpolator UvInterpolator{};
					uint8_t R{}, G{}, B{};
					TextureSourceType& TextureSource;

				public:
					TextureTriangleLit(TextureSourceType& textureSource)
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

					Rgb8::color_t operator()(const int16_t x, const int16_t y)
					{
						const auto fractions = Sampler.GetWeights(x, y).GetFractions();

						coordinate_t uv;
						switch (uvInterpolationMode)
						{
						case PrimitiveShaders::UvInterpolationModeEnum::Fast:
							uv = UvInterpolator.UvFast(fractions.FractionA, fractions.FractionB, fractions.FractionC);
							break;
						case PrimitiveShaders::UvInterpolationModeEnum::Accurate:
						default:
							uv = UvInterpolator.UvAccurate(fractions.FractionA, fractions.FractionB, fractions.FractionC);
							break;
						}

						uint8_t a;
						uint8_t r;
						uint8_t g;
						uint8_t b;
						{
							const Rgb8::color_t texel = TextureSource.GetTexel(uv.x, uv.y);
							a = Rgb8::Alpha(texel);
							r = Rgb8::Red(texel);
							g = Rgb8::Green(texel);
							b = Rgb8::Blue(texel);
						}

						return Rgb8::Color(a,
							MinValue<uint16_t>(UINT8_MAX, (static_cast<uint16_t>(r) * R) >> 8),
							MinValue<uint16_t>(UINT8_MAX, (static_cast<uint16_t>(g) * G) >> 8),
							MinValue<uint16_t>(UINT8_MAX, (static_cast<uint16_t>(b) * B) >> 8));
					}
				};

				template<typename TextureSourceType,
					typename TriangleSamplerType = PrimitiveShaders::TrianglePerspectiveCorrectSampler,
					PrimitiveShaders::UvInterpolationModeEnum uvInterpolationMode = PrimitiveShaders::UvInterpolationModeEnum::Fast>
				class TextureVertexLit
				{
				private:
					TriangleSamplerType Sampler{};
					PrimitiveShaders::UvInterpolator UvInterpolator{};
					uint8_t Ra{}, Ga{}, Ba{}, Rb{}, Gb{}, Bb{}, Rc{}, Gc{}, Bc{};
					TextureSourceType& TextureSource;

				public:
					TextureVertexLit(TextureSourceType& textureSource)
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

					Rgb8::color_t operator()(const int16_t x, const int16_t y)
					{
						const auto fractions = Sampler.GetWeights(x, y).GetFractions();

						uint8_t a;
						uint8_t r;
						uint8_t g;
						uint8_t b;
						{
							coordinate_t uv;
							switch (uvInterpolationMode)
							{
							case PrimitiveShaders::UvInterpolationModeEnum::Fast:
								uv = UvInterpolator.UvFast(fractions.FractionA, fractions.FractionB, fractions.FractionC);
								break;
							case PrimitiveShaders::UvInterpolationModeEnum::Accurate:
							default:
								uv = UvInterpolator.UvAccurate(fractions.FractionA, fractions.FractionB, fractions.FractionC);
								break;
							}

							const Rgb8::color_t texel = TextureSource.GetTexel(uv.x, uv.y);
							a = Rgb8::Alpha(texel);
							r = Rgb8::Red(texel);
							g = Rgb8::Green(texel);
							b = Rgb8::Blue(texel);
						}

						return Rgb8::Color(a,
							static_cast<uint8_t>(MinValue<uint16_t>(UINT8_MAX,
								((static_cast<uint16_t>(Fraction(fractions.FractionA, Ra))
									+ Fraction(fractions.FractionB, Rb)
									+ Fraction(fractions.FractionC, Rc)) * r) >> 8)),
							static_cast<uint8_t>(MinValue<uint16_t>(UINT8_MAX,
								((static_cast<uint16_t>(Fraction(fractions.FractionA, Ga))
									+ Fraction(fractions.FractionB, Gb)
									+ Fraction(fractions.FractionC, Gc)) * g) >> 8)),
							static_cast<uint8_t>(MinValue<uint16_t>(UINT8_MAX,
								((static_cast<uint16_t>(Fraction(fractions.FractionA, Ba))
									+ Fraction(fractions.FractionB, Bb)
									+ Fraction(fractions.FractionC, Bc)) * b) >> 8)));
					}
				};
			}
		}
	}
}
#endif