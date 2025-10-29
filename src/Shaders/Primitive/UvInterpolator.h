#ifndef _INTEGER_WORLD_SHADERS_PRIMITIVE_UV_INTERPOLATOR_SAMPLER_h
#define _INTEGER_WORLD_SHADERS_PRIMITIVE_UV_INTERPOLATOR_SAMPLER_h

#include "../../Framework/Model.h"

namespace IntegerWorld
{
	namespace PrimitiveShaders
	{
		namespace UvInterpolation
		{
			// Bias nudges each per-vertex UV toward the texel center.
			// Compensates for integer fixed-point truncation.
			static constexpr auto Bias = 1;
		}

		/// <summary>
		/// Computes the interpolated texture coordinates (UV) for a given pixel position 
		/// inside the current triangle using barycentric fractions and the per-vertex UVs. 
		/// The result is biased toward texel centers and clamped to non-negative values.
		/// </summary>
		class UvInterpolatorFast
		{
		private:


		private:
			coordinate_t UvA{};
			coordinate_t UvB{};
			coordinate_t UvC{};

		public:
			UvInterpolatorFast() {}

			template<typename mesh_fragment_t>
			void SetFragmentData(const mesh_fragment_t& fragment)
			{
				UvA = fragment.uvA;
				UvB = fragment.uvB;
				UvC = fragment.uvC;
			}

			coordinate_t GetUv(const ufraction8_t fractionA, const ufraction8_t fractionB, const ufraction8_t fractionC) const
			{
				return coordinate_t{
					MaxValue<int16_t>(0, Fraction<int16_t>(fractionA, UvA.x + UvInterpolation::Bias)
						+ Fraction<int16_t>(fractionB, UvB.x + UvInterpolation::Bias)
						+ Fraction<int16_t>(fractionC, UvC.x + UvInterpolation::Bias)),
					MaxValue<int16_t>(0, Fraction<int16_t>(fractionA, UvA.y + UvInterpolation::Bias)
						+ Fraction<int16_t>(fractionB, UvB.y + UvInterpolation::Bias)
						+ Fraction<int16_t>(fractionC, UvC.y + UvInterpolation::Bias)) };
			}

			coordinate_t GetUv(const ufraction16_t fractionA, const ufraction16_t fractionB, const ufraction16_t fractionC) const
			{
				return coordinate_t{
					MaxValue<int16_t>(0, Fraction<int16_t>(fractionA, UvA.x + UvInterpolation::Bias)
						+ Fraction<int16_t>(fractionB, UvB.x + UvInterpolation::Bias)
						+ Fraction<int16_t>(fractionC, UvC.x + UvInterpolation::Bias)),
					MaxValue<int16_t>(0, Fraction<int16_t>(fractionA, UvA.y + UvInterpolation::Bias)
						+ Fraction<int16_t>(fractionB, UvB.y + UvInterpolation::Bias)
						+ Fraction<int16_t>(fractionC, UvC.y + UvInterpolation::Bias)) };
			}
		};

		class UvInterpolatorPerspectiveCorrect
		{
		private:
			coordinate_t UvA{};
			coordinate_t UvB{};
			coordinate_t UvC{};

			// Reciprocals in Q16.
			uint32_t Qa{};
			uint32_t Qb{};
			uint32_t Qc{};

		public:
			UvInterpolatorPerspectiveCorrect() {}

			template<typename mesh_fragment_t>
			void SetFragmentData(const mesh_fragment_t& fragment)
			{
				UvA = fragment.uvA;
				UvB = fragment.uvB;
				UvC = fragment.uvC;

				// Clamp depths to at least 1 to avoid divide-by-zero.
				const int16_t zA = MaxValue<int16_t>(1, fragment.vertexA.z);
				const int16_t zB = MaxValue<int16_t>(1, fragment.vertexB.z);
				const int16_t zC = MaxValue<int16_t>(1, fragment.vertexC.z);

				// Reciprocal in Q16.
				Qa = (static_cast<uint32_t>(1u) << 16) / zA;
				Qb = (static_cast<uint32_t>(1u) << 16) / zB;
				Qc = (static_cast<uint32_t>(1u) << 16) / zC;
			}

			coordinate_t GetUv(const ufraction16_t fractionA, const ufraction16_t fractionB, const ufraction16_t fractionC) const
			{
				const uint32_t denom = static_cast<uint32_t>(fractionA) * Qa + static_cast<uint32_t>(fractionB) * Qb + static_cast<uint32_t>(fractionC) * Qc;

				if (denom == 0)
				{
					// Fallback: linear (no perspective).
					return coordinate_t{
					MaxValue<int16_t>(0, Fraction<int16_t>(fractionA, UvA.x + UvInterpolation::Bias)
						+ Fraction<int16_t>(fractionB, UvB.x + UvInterpolation::Bias)
						+ Fraction<int16_t>(fractionC, UvC.x + UvInterpolation::Bias)),
					MaxValue<int16_t>(0, Fraction<int16_t>(fractionA, UvA.y + UvInterpolation::Bias)
						+ Fraction<int16_t>(fractionB, UvB.y + UvInterpolation::Bias)
						+ Fraction<int16_t>(fractionC, UvC.y + UvInterpolation::Bias)) };
				}

				// Numerator sums: f * (uv * q).
				const uint32_t numU = static_cast<uint32_t>(fractionA) * (static_cast<int32_t>(UvA.x) * Qa)
					+ static_cast<uint32_t>(fractionB) * (static_cast<int32_t>(UvB.x) * Qb)
					+ static_cast<uint32_t>(fractionC) * (static_cast<int32_t>(UvC.x) * Qc);

				const uint32_t numV = static_cast<uint32_t>(fractionA) * (static_cast<int32_t>(UvA.y) * Qa)
					+ static_cast<uint32_t>(fractionB) * (static_cast<int32_t>(UvB.y) * Qb)
					+ static_cast<uint32_t>(fractionC) * (static_cast<int32_t>(UvC.y) * Qc);

				// Result in texture coordinate units (round).
				return coordinate_t{
					static_cast<int16_t>(MaxValue<int32_t>(0, static_cast<int32_t>((numU + SignedRightShift(denom, 1)) / denom))),
					static_cast<int16_t>(MaxValue<int32_t>(0, static_cast<int32_t>((numV + SignedRightShift(denom, 1)) / denom))) };
			}
		};
	}
}
#endif