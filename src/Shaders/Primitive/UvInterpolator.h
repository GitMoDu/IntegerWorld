#ifndef _INTEGER_WORLD_SHADERS_PRIMITIVE_UV_INTERPOLATOR_SAMPLER_h
#define _INTEGER_WORLD_SHADERS_PRIMITIVE_UV_INTERPOLATOR_SAMPLER_h

#include "../../Framework/Model.h"

namespace IntegerWorld
{
	namespace PrimitiveShaders
	{
		enum class UvInterpolationMode : uint8_t
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
			coordinate_t UvA{}, UvB{}, UvC{};

		public:
			UvInterpolator() {}

			template<typename mesh_fragment_t>
			void SetFragmentData(const mesh_fragment_t& fragment)
			{
				UvA = fragment.uvA;
				UvB = fragment.uvB;
				UvC = fragment.uvC;
			}

			coordinate_t UvFast(const ufraction8_t fractionA, const ufraction8_t fractionB, const ufraction8_t fractionC) const
			{
				return coordinate_t{
					MaxValue<int16_t>(0, Fraction<int16_t>(fractionA, UvA.x + Bias)
						+ Fraction<int16_t>(fractionB, UvB.x + Bias)
						+ Fraction<int16_t>(fractionC, UvC.x + Bias)),
					MaxValue<int16_t>(0, Fraction<int16_t>(fractionA, UvA.y + Bias)
						+ Fraction<int16_t>(fractionB, UvB.y + Bias)
						+ Fraction<int16_t>(fractionC, UvC.y + Bias)) };
			}

			coordinate_t UvFast(const ufraction16_t fractionA, const ufraction16_t fractionB, const ufraction16_t fractionC) const
			{
				return coordinate_t{
					MaxValue<int16_t>(0, Fraction<int16_t>(fractionA, UvA.x + Bias)
						+ Fraction<int16_t>(fractionB, UvB.x + Bias)
						+ Fraction<int16_t>(fractionC, UvC.x + Bias)),
					MaxValue<int16_t>(0, Fraction<int16_t>(fractionA, UvA.y + Bias)
						+ Fraction<int16_t>(fractionB, UvB.y + Bias)
						+ Fraction<int16_t>(fractionC, UvC.y + Bias)) };
			}

			coordinate_t UvAccurate(const ufraction8_t fractionA, const ufraction8_t fractionB, const ufraction8_t fractionC) const
			{
				const uint32_t sum = static_cast<uint32_t>(fractionA) + fractionB + fractionC;

				const int32_t numU = static_cast<int32_t>(UvA.x) * fractionA
					+ static_cast<int32_t>(UvB.x) * fractionB
					+ static_cast<int32_t>(UvC.x) * fractionC;

				const int32_t numV = static_cast<int32_t>(UvA.y) * fractionA
					+ static_cast<int32_t>(UvB.y) * fractionB
					+ static_cast<int32_t>(UvC.y) * fractionC;

				const int16_t u = static_cast<int16_t>(MaxValue<int32_t>(0, static_cast<int32_t>((numU + SignedRightShift(sum, 1)) / sum)));
				const int16_t v = static_cast<int16_t>(MaxValue<int32_t>(0, static_cast<int32_t>((numV + SignedRightShift(sum, 1)) / sum)));

				return coordinate_t{ u, v };
			}

			coordinate_t UvAccurate(const ufraction16_t fractionA, const ufraction16_t fractionB, const ufraction16_t fractionC) const
			{
				const uint32_t sum = static_cast<uint32_t>(fractionA) + fractionB + fractionC;

				const int32_t numU = static_cast<int32_t>(UvA.x) * fractionA
					+ static_cast<int32_t>(UvB.x) * fractionB
					+ static_cast<int32_t>(UvC.x) * fractionC;

				const int32_t numV = static_cast<int32_t>(UvA.y) * fractionA
					+ static_cast<int32_t>(UvB.y) * fractionB
					+ static_cast<int32_t>(UvC.y) * fractionC;

				const int16_t u = static_cast<int16_t>(MaxValue<int32_t>(0, static_cast<int32_t>((numU + SignedRightShift(sum, 1)) / sum)));
				const int16_t v = static_cast<int16_t>(MaxValue<int32_t>(0, static_cast<int32_t>((numV + SignedRightShift(sum, 1)) / sum)));

				return coordinate_t{ u, v };
			}
		};
	}
}
#endif