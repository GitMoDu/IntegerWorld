#ifndef _INTEGER_WORLD_SHADERS_PRIMITIVE_TRIANGLE_SAMPLER_h
#define _INTEGER_WORLD_SHADERS_PRIMITIVE_TRIANGLE_SAMPLER_h

#include "../../Framework/Model.h"

namespace IntegerWorld
{
	namespace PrimitiveShaders
	{
		struct TriangleWeights
		{
			int16_t WeightA;
			int16_t WeightB;
			int16_t WeightC;
		};

		struct TriangleU8Fractions
		{
			ufraction8_t FractionA;
			ufraction8_t FractionB;
			ufraction8_t FractionC;
		};

		struct TriangleU16Fractions
		{
			ufraction16_t FractionA;
			ufraction16_t FractionB;
			ufraction16_t FractionC;
		};

		/// <summary>
		/// Screen-space barycentric sampler for a single triangle.
		/// Precomputes edge-function coefficients and a reduced 16-bit area so
		/// weights and fixed-point fractions can be evaluated efficiently for any pixel.
		/// </summary>
		/// <remarks>
		/// - Call SetTriangle(a, b, c) to cache edge deltas and normalization.
		/// - If the input triangle has negative area, vertices B and C are treated as swapped (see IsSwapped()).
		/// - Internal coefficients and area are uniformly reduced by a power-of-two to fit 16-bit while
		///   preserving proportionality and preventing overflow on large triangles.
		/// - WeightA/WeightB/WeightC return clamped integer weights in [0, ReducedArea].
		/// - U8Fraction*/U16Fraction* return normalized fixed-point fractions in [0, 1].
		/// - ReducedArea corresponds to twice the geometric area after reduction.
		/// </remarks>
		class TriangleSampler
		{
		private:
			int16_t Cx = 0;
			int16_t Cy = 0;

		private:
			int16_t BmCy = 0;
			int16_t CmBx = 0;
			int16_t CmAy = 0;
			int16_t AmCx = 0;

		private:
			int16_t ReducedArea = 0;

			/// <summary>
			/// If the triangle has negative area, vertices B and C are swapped.
			/// </summary>
			bool Swapped = false;

		public:
			/// <summary>
			/// Precomputes and caches triangle properties from a fragment's vertices for barycentric interpolation and rasterization.
			/// Detects degenerate triangles, may reorder vertices to ensure consistent winding.
			/// </summary>
			/// <typeparam name="fragment_t">The fragment type. Must provide accessible members vertexA, vertexB, and vertexC, each with x and y coordinate fields.</typeparam>
			/// <param name="fragment">A fragment object that provides three vertices named vertexA, vertexB, and vertexC. Each vertex must expose x and y coordinates (numeric types convertible to int32_t).</param>
			/// <returns>true if the fragment defines a non-degenerate triangle and triangle data was computed and cached; false if the triangle area is zero (degenerate) and no data was set.</returns>
			template<typename fragment_t>
			bool SetFragmentData(const fragment_t& fragment)
			{
				// Compute denominator (twice the area of the triangle)
				int32_t triangleArea = static_cast<int32_t>(fragment.vertexB.y - fragment.vertexC.y) * (fragment.vertexA.x - fragment.vertexC.x)
					+ static_cast<int32_t>(fragment.vertexC.x - fragment.vertexB.x) * (fragment.vertexA.y - fragment.vertexC.y);

				if (triangleArea == 0)
				{
					return false;
				}
				else // Pre-calculate intermediates and cache triangle properties.
				{
					// If area is negative, swap B and C to maintain consistent winding.
					Swapped = triangleArea < 0;
					if (Swapped)
					{
						triangleArea = -triangleArea;

						// Swap B and C
						BmCy = fragment.vertexC.y - fragment.vertexB.y;
						CmBx = fragment.vertexB.x - fragment.vertexC.x;
						CmAy = fragment.vertexB.y - fragment.vertexA.y;
						AmCx = fragment.vertexA.x - fragment.vertexB.x;

						Cx = fragment.vertexB.x;
						Cy = fragment.vertexB.y;
					}
					else
					{
						BmCy = fragment.vertexB.y - fragment.vertexC.y;
						CmBx = fragment.vertexC.x - fragment.vertexB.x;
						CmAy = fragment.vertexC.y - fragment.vertexA.y;
						AmCx = fragment.vertexA.x - fragment.vertexC.x;

						Cx = fragment.vertexC.x;
						Cy = fragment.vertexC.y;
					}

					// Reduce area and weights to fit in 16-bit for faster barycentric calculations.
					uint8_t reduceShifts = 0;
					while (triangleArea > static_cast<int32_t>(INT16_MAX))
					{
						triangleArea >>= 1;
						reduceShifts++;
					}
					ReducedArea = triangleArea;
					BmCy = SignedRightShift(BmCy, reduceShifts);
					CmBx = SignedRightShift(CmBx, reduceShifts);
					CmAy = SignedRightShift(CmAy, reduceShifts);
					AmCx = SignedRightShift(AmCx, reduceShifts);

					return true;
				}
			}

			/// <summary>
			/// Order unaware weight and fraction calculations.
			/// </summary>
		protected:
			int16_t WeightB(const int16_t x, const int16_t y) const
			{
				return LimitValue<int32_t>((static_cast<int32_t>(CmAy) * (x - Cx)) + (static_cast<int32_t>(AmCx) * (y - Cy)), 0, ReducedArea);
			}

			int16_t WeightC(const int16_t weightA, const int16_t weightB) const
			{
				return ReducedArea - weightA - weightB;
			}

			ufraction8_t U8FractionB(const int16_t x, const int16_t y) const
			{
				return UFraction8::GetScalar(WeightB(x, y), ReducedArea);
			}

			ufraction8_t U8FractionC(const ufraction8_t fraction0, const ufraction8_t fraction1) const
			{
				return LimitValue<int16_t, 0, UFRACTION8_1X>(int16_t(UFRACTION8_1X) - fraction0 - fraction1);
			}

			ufraction16_t U16FractionB(const int16_t x, const int16_t y) const
			{
				return UFraction16::GetScalar(WeightB(x, y), ReducedArea);
			}

			ufraction16_t U16FractionC(const ufraction16_t fractionA, const ufraction16_t fractionB) const
			{
				return LimitValue<int32_t, 0, UFRACTION16_1X>(int32_t(UFRACTION16_1X) - fractionA - fractionB);
			}

			/// <summary>
			/// Order compliant weight and fraction calculations.
			/// </summary>
		public:
			int16_t WeightA(const int16_t x, const int16_t y) const
			{
				return LimitValue<int32_t>((static_cast<int32_t>(BmCy) * (x - Cx)) + (static_cast<int32_t>(CmBx) * (y - Cy)), 0, ReducedArea);
			}

			ufraction8_t U8FractionA(const int16_t x, const int16_t y) const
			{
				return UFraction8::GetScalar(WeightA(x, y), ReducedArea);
			}

			ufraction16_t U16FractionA(const int16_t x, const int16_t y) const
			{
				return UFraction16::GetScalar(WeightA(x, y), ReducedArea);
			}

			TriangleWeights Weights(const int16_t x, const int16_t y) const
			{
				const int16_t xmCx = x - Cx;
				const int16_t ymCy = y - Cy;
				const int16_t wA = LimitValue<int32_t>((static_cast<int32_t>(BmCy) * xmCx) + (static_cast<int32_t>(CmBx) * ymCy), 0, ReducedArea);
				const int16_t wB = LimitValue<int32_t>((static_cast<int32_t>(CmAy) * xmCx) + (static_cast<int32_t>(AmCx) * ymCy), 0, ReducedArea);
				const int16_t wC = ReducedArea - MinValue<int32_t>(static_cast<int32_t>(wA) + wB, ReducedArea);

				if (Swapped)
					return TriangleWeights{ wA, wC, wB };
				else
					return TriangleWeights{ wA, wB, wC };
			}

			TriangleU8Fractions U8Fractions(const int16_t x, const int16_t y) const
			{
				const int16_t xmCx = x - Cx;
				const int16_t ymCy = y - Cy;
				const int16_t wA = LimitValue<int32_t>((static_cast<int32_t>(BmCy) * xmCx) + (static_cast<int32_t>(CmBx) * ymCy), 0, ReducedArea);
				const int16_t wB = LimitValue<int32_t>((static_cast<int32_t>(CmAy) * xmCx) + (static_cast<int32_t>(AmCx) * ymCy), 0, ReducedArea);

				const ufraction8_t fA = UFraction8::GetScalar(wA, ReducedArea);
				const ufraction8_t fB = UFraction8::GetScalar(wB, ReducedArea);
				const ufraction8_t fC = LimitValue<int16_t, 0, UFRACTION8_1X>(static_cast<int16_t>(UFRACTION8_1X) - fA - fB);

				if (Swapped)
					return TriangleU8Fractions{ fA, fC, fB };
				else
					return TriangleU8Fractions{ fA, fB, fC };
			}

			TriangleU16Fractions U16Fractions(const int16_t x, const int16_t y) const
			{
				const int16_t xmCx = x - Cx;
				const int16_t ymCy = y - Cy;
				const int16_t wA = LimitValue<int32_t>((static_cast<int32_t>(BmCy) * xmCx) + (static_cast<int32_t>(CmBx) * ymCy), 0, ReducedArea);
				const int16_t wB = LimitValue<int32_t>((static_cast<int32_t>(CmAy) * xmCx) + (static_cast<int32_t>(AmCx) * ymCy), 0, ReducedArea);

				const ufraction16_t fA = UFraction16::GetScalar(wA, ReducedArea);
				const ufraction16_t fB = UFraction16::GetScalar(wB, ReducedArea);
				const ufraction16_t fC = LimitValue<int32_t, 0, UFRACTION16_1X>(static_cast<int32_t>(UFRACTION16_1X) - fA - fB);

				if (Swapped)
					return TriangleU16Fractions{ fA, fC, fB };
				else
					return TriangleU16Fractions{ fA, fB, fC };
			}
		};
	}
}
#endif