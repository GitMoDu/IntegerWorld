#ifndef _INTEGER_WORLD_SHADERS_PRIMITIVE_TRIANGLE_SAMPLER_h
#define _INTEGER_WORLD_SHADERS_PRIMITIVE_TRIANGLE_SAMPLER_h

#include "../../Framework/Model.h"

namespace IntegerWorld
{
	namespace PrimitiveShaders
	{
		struct triangle_sample_weights_t
		{
			int16_t WeightA;
			int16_t WeightB;
			int16_t WeightC;
		};

		struct triangle_sample_fractions_t
		{
			ufraction16_t FractionA;
			ufraction16_t FractionB;
			ufraction16_t FractionC;
		};

		namespace Abstract
		{
			/// <summary>
			/// Base class for screen-space barycentric sampling of a single triangle.
			/// Precomputes edge-function coefficients and a reduced 16-bit area so
			/// weights and fixed-point fractions can be evaluated efficiently for any pixel.
			/// </summary>
			class AbstractSampler
			{
			protected:
				int16_t Cx = 0;
				int16_t Cy = 0;

				int16_t BmCy = 0;
				int16_t CmBx = 0;
				int16_t CmAy = 0;
				int16_t AmCx = 0;

				int16_t ReducedArea = 0;

				/// <summary>
				/// If the triangle has negative area, vertices B and C are swapped.
				/// </summary>
				bool Swapped = false;

			protected:
				/// <summary>
				/// Set triangle properties with the three vertices. 
				/// Precomputes values for efficient barycentric calculations.
				/// Ensures the triangle has non-zero area and maintains consistent winding order. 
				/// </summary>
				template<typename fragment_t>
				bool SetTriangle(const fragment_t& fragment)
				{
					// Compute denominator (twice the area of the triangle)
					int32_t triangleArea = static_cast<int32_t>(fragment.vertexB.y - fragment.vertexC.y) * (fragment.vertexA.x - fragment.vertexC.x) + static_cast<int32_t>(fragment.vertexC.x - fragment.vertexB.x) * (fragment.vertexA.y - fragment.vertexC.y);

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
							BmCy = fragment.vertexC.y - fragment.vertexB.y;
							CmBx = fragment.vertexB.x - fragment.vertexC.x;
							CmAy = fragment.vertexB.y - fragment.vertexA.y;
							AmCx = fragment.vertexA.x - fragment.vertexB.x;

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
						ReducedArea = static_cast<int16_t>(triangleArea);
						BmCy = SignedRightShift(BmCy, reduceShifts);
						CmBx = SignedRightShift(CmBx, reduceShifts);
						CmAy = SignedRightShift(CmAy, reduceShifts);
						AmCx = SignedRightShift(AmCx, reduceShifts);

						return true;
					}
				}

			public:
				triangle_sample_weights_t Weights(const int16_t x, const int16_t y) const
				{
					const int16_t xmCx = x - Cx;
					const int16_t ymCy = y - Cy;
					const int16_t wA = LimitValue<int32_t>((static_cast<int32_t>(BmCy) * xmCx) + (static_cast<int32_t>(CmBx) * ymCy), 0, ReducedArea);
					const int16_t wB = LimitValue<int32_t>((static_cast<int32_t>(CmAy) * xmCx) + (static_cast<int32_t>(AmCx) * ymCy), 0, ReducedArea);
					const int16_t wC = ReducedArea - MinValue<int32_t>(static_cast<int32_t>(wA) + wB, ReducedArea);

					if (Swapped)
						return triangle_sample_weights_t{ wA, wC, wB };
					else
						return triangle_sample_weights_t{ wA, wB, wC };
				}
			};
		}


		/// <summary>
		/// Linear (affine) triangle sampler.
		/// Provides standard barycentric interpolation without perspective correction.
		/// </summary>
		class TriangleAffineSampler : public Abstract::AbstractSampler
		{
		public:
			TriangleAffineSampler() : Abstract::AbstractSampler() {}

		public:
			template<typename fragment_t>
			bool SetFragmentData(const fragment_t& fragment)
			{
				return SetTriangle(fragment);
			}

			triangle_sample_fractions_t Fractions(const int16_t x, const int16_t y) const
			{
				const int16_t xmCx = x - Cx;
				const int16_t ymCy = y - Cy;
				const int16_t wA = LimitValue<int32_t>((static_cast<int32_t>(BmCy) * xmCx) + (static_cast<int32_t>(CmBx) * ymCy), 0, ReducedArea);
				const int16_t wB = LimitValue<int32_t>((static_cast<int32_t>(CmAy) * xmCx) + (static_cast<int32_t>(AmCx) * ymCy), 0, ReducedArea);

				const ufraction16_t fA = UFraction16::GetScalar(wA, ReducedArea);
				const ufraction16_t fB = UFraction16::GetScalar(wB, ReducedArea);
				const ufraction16_t fC = LimitValue<int32_t, 0, UFRACTION16_1X>(static_cast<int32_t>(UFRACTION16_1X) - fA - fB);

				if (Swapped)
					return triangle_sample_fractions_t{ fA, fC, fB };
				else
					return triangle_sample_fractions_t{ fA, fB, fC };
			}
		};

		/// <summary>
		/// Perspective-correct triangle sampler.
		/// Stores per-vertex depth reciprocals and provides perspective-correct barycentric interpolation.
		/// </summary>
		class TrianglePerspectiveCorrectSampler : public Abstract::AbstractSampler
		{
		private:
			// Q format parameters for perspective correction.
			static constexpr uint8_t Qbits = 24;
			static constexpr uint32_t Qscale = (static_cast<uint32_t>(1) << Qbits);

			// Perspective-correction reciprocals (Qi = (1<<Qbits)/z_i).
			uint16_t Qa{}, Qb{}, Qc{};

		public:
			TrianglePerspectiveCorrectSampler() : Abstract::AbstractSampler() {}

			/// <summary>
			/// Configure perspective-correct interpolation for the current triangle.
			/// Stores per-vertex reciprocals Qi = (1<<qBits)/z_i using clamped z>=1.
			/// Returns false if the triangle is degenerate (SetTriangle fails).
			/// </summary>
			/// <typeparam name="fragment_t">The fragment type. Must be acceptable to Abstract::AbstractSampler::SetTriangle and provide vertexA, vertexB, and vertexC with a numeric z component (used in MaxValue<int16_t> calls).</typeparam>
			/// <param name="fragment">A constant reference to the fragment (typically a triangle) to set. The fragment must provide vertexA, vertexB, and vertexC members; each vertex must expose a numeric z component used for depth-based Q computation.</param>
			/// <returns>true if the fragment was accepted by the base SetTriangle call and internal Q values (Qa, Qb, Qc) were computed; false if the base SetTriangle call failed.</returns>
			template<typename fragment_t>
			bool SetFragmentData(const fragment_t& fragment)
			{
				//return SetTriangle(fragment.vertexA, fragment.vertexB, fragment.vertexC);
				if (!Abstract::AbstractSampler::SetTriangle(fragment))
					return false;

				Qa = Qscale / MaxValue<int16_t>(1, fragment.vertexA.z);
				Qb = Qscale / MaxValue<int16_t>(1, fragment.vertexB.z);
				Qc = Qscale / MaxValue<int16_t>(1, fragment.vertexC.z);

				return true;
			}

			/// <summary>
			/// Perspective-correct "weights" (numerators) n_i = w_i * Q_i.
			/// Sum of returned weights is the perspective-correct denominator.
			/// </summary>
			triangle_sample_weights_t Weights(const int16_t x, const int16_t y) const
			{
				const int16_t xmCx = x - Cx;
				const int16_t ymCy = y - Cy;
				const int16_t wA = LimitValue<int32_t>((static_cast<int32_t>(BmCy) * xmCx) + (static_cast<int32_t>(CmBx) * ymCy), 0, ReducedArea);
				const int16_t wB = LimitValue<int32_t>((static_cast<int32_t>(CmAy) * xmCx) + (static_cast<int32_t>(AmCx) * ymCy), 0, ReducedArea);
				const int16_t wC = ReducedArea - MinValue<int32_t>(static_cast<int32_t>(wA) + wB, ReducedArea);

				int32_t nA = static_cast<int32_t>(wA) * Qa;
				int32_t nB, nC;

				// Map internal weights to original vertices before applying Qi.
				if (Swapped)
				{
					nB = static_cast<int32_t>(wC) * Qb;
					nC = static_cast<int32_t>(wB) * Qc;
				}
				else
				{
					nB = static_cast<int32_t>(wB) * Qb;
					nC = static_cast<int32_t>(wC) * Qc;
				}

				// Normalize to fit in int16_t if needed, preserving ratios.
				while (AbsValue(nA + nB + nC) > INT16_MAX)
				{
					nA = SignedRightShift(nA, 1);
					nB = SignedRightShift(nB, 1);
					nC = SignedRightShift(nC, 1);
				}

				return triangle_sample_weights_t{
					static_cast<int16_t>(nA),
					static_cast<int16_t>(nB),
					static_cast<int16_t>(nC)
				};
			}

			triangle_sample_fractions_t Fractions(const int16_t x, const int16_t y) const
			{
				const triangle_sample_weights_t weights = Weights(x, y);
				const int16_t denom = weights.WeightA + weights.WeightB + weights.WeightC;

				if (denom == 0)
				{
					// Fallback to linear if denominator is zero
					const int16_t xmCx = x - Cx;
					const int16_t ymCy = y - Cy;
					const int16_t wA = LimitValue<int32_t>((static_cast<int32_t>(BmCy) * xmCx) + (static_cast<int32_t>(CmBx) * ymCy), 0, ReducedArea);
					const int16_t wB = LimitValue<int32_t>((static_cast<int32_t>(CmAy) * xmCx) + (static_cast<int32_t>(AmCx) * ymCy), 0, ReducedArea);

					const ufraction16_t fA = UFraction16::GetScalar(wA, ReducedArea);
					const ufraction16_t fB = UFraction16::GetScalar(wB, ReducedArea);
					const ufraction16_t fC = LimitValue<int32_t, 0, UFRACTION16_1X>(static_cast<int32_t>(UFRACTION16_1X) - fA - fB);

					if (Swapped)
						return triangle_sample_fractions_t{ fA, fC, fB };
					else
						return triangle_sample_fractions_t{ fA, fB, fC };
				}
				else
				{
					// Weights are already normalized to fit in int16_t and properly perspective-corrected
					const ufraction16_t fA = static_cast<ufraction16_t>(LimitValue<int32_t, 0, UFRACTION16_1X>(
						(static_cast<int32_t>(weights.WeightA) * static_cast<int32_t>(UFRACTION16_1X) + SignedRightShift(denom, 1)) / denom));
					const ufraction16_t fB = static_cast<ufraction16_t>(LimitValue<int32_t, 0, UFRACTION16_1X>(
						(static_cast<int32_t>(weights.WeightB) * static_cast<int32_t>(UFRACTION16_1X) + SignedRightShift(denom, 1)) / denom));
					const ufraction16_t fC = LimitValue<int32_t, 0, UFRACTION16_1X>(static_cast<int32_t>(UFRACTION16_1X) - fA - fB);

					return triangle_sample_fractions_t{ fA, fB, fC };
				}
			}
		};
	}
}
#endif