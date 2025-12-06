#ifndef _INTEGER_WORLD_SHADERS_PRIMITIVE_UV_INTERPOLATOR_SAMPLER_h
#define _INTEGER_WORLD_SHADERS_PRIMITIVE_UV_INTERPOLATOR_SAMPLER_h

#include "../../Framework/Model.h"

namespace IntegerWorld
{
	namespace PrimitiveShaders
	{
		enum class UvInterpolationModeEnum : uint8_t
		{
			Fast,
			Accurate
		};

		class UvInterpolator
		{
		protected:
			// Bias nudges each per-vertex UV toward the texel center.
			// Compensates for integer fixed-point truncation.
			static constexpr auto Bias = 1;

		private:
			uv_t UvA{}, UvB{}, UvC{};

		public:
			UvInterpolator() {}

			template<typename mesh_fragment_t>
			void SetFragmentData(const mesh_fragment_t& fragment)
			{
				UvA = fragment.uvA;
				UvB = fragment.uvB;
				UvC = fragment.uvC;
			}

			uv_t UvFast(const ufraction16_t fractionA, const ufraction16_t fractionB, const ufraction16_t fractionC) const
			{
				return uv_t{
					static_cast<uint8_t>(
						Fraction<uint16_t>(fractionA, static_cast<uint16_t>(UvA.x) + Bias) +
						Fraction<uint16_t>(fractionB, static_cast<uint16_t>(UvB.x) + Bias) +
						Fraction<uint16_t>(fractionC, static_cast<uint16_t>(UvC.x) + Bias)),
					static_cast<uint8_t>(
						Fraction<uint16_t>(fractionA, static_cast<uint16_t>(UvA.y) + Bias) +
						Fraction<uint16_t>(fractionB, static_cast<uint16_t>(UvB.y) + Bias) +
						Fraction<uint16_t>(fractionC, static_cast<uint16_t>(UvC.y) + Bias)) };
			}

			uv_t UvAccurate(const ufraction16_t fractionA, const ufraction16_t fractionB, const ufraction16_t fractionC) const
			{
				const uint16_t sum = MaxValue<uint16_t>(1, static_cast<uint16_t>(fractionA) + fractionB + fractionC);

				const uint32_t numU = static_cast<uint32_t>(UvA.x) * fractionA
					+ static_cast<uint32_t>(UvB.x) * fractionB
					+ static_cast<uint32_t>(UvC.x) * fractionC
					+ Bias;

				const uint32_t numV = static_cast<uint32_t>(UvA.y) * fractionA
					+ static_cast<uint32_t>(UvB.y) * fractionB
					+ static_cast<uint32_t>(UvC.y) * fractionC
					+ Bias;

				return uv_t{
					static_cast<uint8_t>(numU / sum),
					static_cast<uint8_t>(numV / sum) };
			}
		};
	}
}
#endif