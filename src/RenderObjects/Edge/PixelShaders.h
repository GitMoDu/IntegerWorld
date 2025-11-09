#ifndef _INTEGER_WORLD_RENDER_OBJECT_EDGE_PIXEL_SHADERS_h
#define _INTEGER_WORLD_RENDER_OBJECT_EDGE_PIXEL_SHADERS_h

#include "AbstractObject.h"
#include "../../Shaders/Primitive/DepthSampler.h"
#include "../../Shaders/Primitive/LineSampler.h"

namespace IntegerWorld
{
	namespace RenderObjects
	{
		namespace Edge
		{
			namespace PixelShaders
			{
				template<typename fragment_t = edge_line_fragment_t>
				class ZInterpolate
				{
				private:
					PrimitiveShaders::LineSampler Sampler{};
					int16_t Az = 0;
					int16_t BmAZ = 0;

				public:
					bool SetFragmentData(const fragment_t& fragment)
					{
						if (Sampler.SetLine(fragment.vertexA, fragment.vertexB))
						{
							Az = fragment.vertexA.z;
							BmAZ = fragment.vertexB.z - fragment.vertexA.z;
							return true;
						}
						return false;
					}

					Rgb8::color_t operator()(const int16_t x, const int16_t y)
					{
						int16_t z;
						{
							const auto fraction = Sampler.U16Fraction(x, y);
							z = LimitValue<int32_t, INT16_MIN, INT16_MAX>(static_cast<int32_t>(Az) + Fraction(fraction, BmAZ));
						}
						const uint8_t gray = PrimitiveShaders::DepthSampler::ZDepth8(z);

						return Rgb8::Color(gray, gray, gray);
					}
				};

				class VertexColorInterpolate
				{
				private:
					PrimitiveShaders::LineSampler Sampler{};
					uint8_t Ra{}, Ga{}, Ba{};
					int8_t Rbma{}, Gbma{}, Bbma{};

				public:
					bool SetFragmentData(const vertex16_t& a, const vertex16_t& b, const Rgb8::color_t colorA, const Rgb8::color_t colorB)
					{
						if (Sampler.SetLine(a, b))
						{
							Ra = Rgb8::Red(colorA);
							Ga = Rgb8::Green(colorA);
							Ba = Rgb8::Blue(colorA);
							Rbma = LimitValue<int16_t, INT8_MIN, INT8_MAX>(static_cast<int16_t>(Rgb8::Red(colorB)) - Ra);
							Gbma = LimitValue<int16_t, INT8_MIN, INT8_MAX>(static_cast<int16_t>(Rgb8::Green(colorB)) - Ga);
							Bbma = LimitValue<int16_t, INT8_MIN, INT8_MAX>(static_cast<int16_t>(Rgb8::Blue(colorB)) - Ba);

							return true;
						}
						return false;
					}

					Rgb8::color_t operator()(const int16_t x, const int16_t y)
					{
						const auto fraction = Sampler.U16Fraction(x, y);

						return Rgb8::Color(
							LimitValue<int16_t, 0, UINT8_MAX>(static_cast<int16_t>(Ra) + Fraction(fraction, Rbma)),
							LimitValue<int16_t, 0, UINT8_MAX>(static_cast<int16_t>(Ga) + Fraction(fraction, Gbma)),
							LimitValue<int16_t, 0, UINT8_MAX>(static_cast<int16_t>(Ba) + Fraction(fraction, Bbma)));
					}
				};
			}
		}
	}
}
#endif