#ifndef _INTEGER_WORLD_SHADERS_PRIMITIVE_TRIANGLE_SAMPLER_h
#define _INTEGER_WORLD_SHADERS_PRIMITIVE_TRIANGLE_SAMPLER_h

#include "../../Framework/Model.h"

namespace IntegerWorld
{
	namespace PrimitiveShaders
	{
		struct triangle_sample_fractions_t
		{
			ufraction16_t FractionA;
			ufraction16_t FractionB;
			ufraction16_t FractionC;
		};

		struct triangle_sample_weights_t
		{
			int16_t WeightA;
			int16_t WeightB;
			int16_t WeightC;

			triangle_sample_fractions_t GetFractions() const
			{
				//const uint16_t totalWeight = MinValue<uint32_t>(UINT16_MAX, static_cast<uint32_t>(WeightA) + WeightB + WeightC);
				const uint16_t totalWeight = static_cast<uint16_t>(WeightA) + WeightB + WeightC;

				// Weights are already normalized to fit in int16_t and properly perspective-corrected.
				const ufraction16_t fA = UFraction16::GetScalar<uint16_t>(WeightA, totalWeight);
				const ufraction16_t fB = UFraction16::GetScalar<uint16_t>(WeightB, totalWeight);
				const ufraction16_t fC = LimitValue<int32_t, 0, UFRACTION16_1X>(static_cast<int32_t>(UFRACTION16_1X) - fA - fB);

				return triangle_sample_fractions_t{ fA, fB, fC };
			}
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

				uint16_t ReducedArea = 0;
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
					// Twice the signed area using C as pivot:
					// area2 = (B - C) x (A - C)
					int32_t area2 =
						static_cast<int32_t>(fragment.vertexB.y - fragment.vertexC.y) * (fragment.vertexA.x - fragment.vertexC.x)
						- static_cast<int32_t>(fragment.vertexB.x - fragment.vertexC.x) * (fragment.vertexA.y - fragment.vertexC.y);

					// Normalize orientation: ensure positive area by swapping B<->C logically.
					Swapped = (area2 < 0);
					if (Swapped) area2 = -area2;

					if (area2 < 1)
						return false;

					const vertex16_t& a = fragment.vertexA;
					const vertex16_t& b = Swapped ? fragment.vertexC : fragment.vertexB;
					const vertex16_t& c = Swapped ? fragment.vertexB : fragment.vertexC;

					// Precompute edge-function coefficients with C as pivot.
					BmCy = b.y - c.y;
					CmBx = c.x - b.x;
					CmAy = c.y - a.y;
					AmCx = a.x - c.x;

					// Reduce to fit in int16 while preserving ratios for area.
					while (area2 > INT16_MAX)
					{
						area2 >>= 1; // Areas is always positive here.
						BmCy = SignedRightShift(BmCy, 1);
						CmBx = SignedRightShift(CmBx, 1);
						CmAy = SignedRightShift(CmAy, 1);
						AmCx = SignedRightShift(AmCx, 1);
					}

					if (area2 < 1)
						return false;

					ReducedArea = static_cast<uint16_t>(area2);

					// Always store C as the pivot for fast evaluation.
					Cx = c.x;
					Cy = c.y;

					return true;
				}
			};
		}

		/// <summary>
		/// Linear (affine) triangle sampler.
		/// Provides standard barycentric weights without perspective correction.
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

			/// <summary>
			/// Compute sample weights for the triangle's vertices at a given sample position. 
			/// </summary>
			/// <param name="x">Sample x coordinate (int16_t) in the same coordinate space used by the triangle.</param>
			/// <param name="y">Sample y coordinate (int16_t) in the same coordinate space used by the triangle.</param>
			/// <returns>A triangle_sample_weights_t containing three non-negative weights (int16_t) corresponding to the triangle vertices A, B, and C. The weights are computed from internal area calculations, mapped back to the original vertex order if a swap occurred, and clamped to a minimum of 0.</returns>
			triangle_sample_weights_t GetWeights(const int16_t x, const int16_t y) const
			{
				int16_t wA{}, wB{}, wC{};
				{
					const int16_t xmCx = x - Cx;
					const int16_t ymCy = y - Cy;

					wA = (static_cast<int32_t>(BmCy) * xmCx) + (static_cast<int32_t>(CmBx) * ymCy);
					wB = (static_cast<int32_t>(CmAy) * xmCx) + (static_cast<int32_t>(AmCx) * ymCy);
					wC = static_cast<int16_t>(ReducedArea) - wA - wB;

					// Map back to original vertex order if we swapped.
					if (Swapped)
					{
						const uint16_t temp = wB;
						wB = wC;
						wC = temp;
					}
				}

				return triangle_sample_weights_t{
					MaxValue<int16_t>(wA, 0),
					MaxValue<int16_t>(wB, 0),
					MaxValue<int16_t>(wC, 0) };
			}
		};

		/// <summary>
		/// Perspective-correct triangle sampler.
		/// Stores per-vertex depth reciprocals and provides perspective-correct barycentric weights.
		/// </summary>
		class TrianglePerspectiveCorrectSampler : public Abstract::AbstractSampler
		{
		private:
			// Q format parameters for perspective correction.
			static constexpr uint8_t Qbits = 24;
			static constexpr uint32_t Qscale = static_cast<uint32_t>(1) << Qbits;

			// Perspective-correction reciprocals (Qi = (1<<qBits)/z_i).
			uint16_t Qa{}, Qb{}, Qc{};

		public:
			TrianglePerspectiveCorrectSampler() : Abstract::AbstractSampler() {}

			/// <summary>
			/// Configure perspective-correct interpolation for the current triangle.
			/// Returns false if the triangle is degenerate (SetTriangle fails).
			/// </summary>
			/// <typeparam name="fragment_t">The fragment type. Must be acceptable to Abstract::AbstractSampler::SetTriangle and provide vertexA, vertexB, and vertexC with a numeric z component (used in MaxValue<int16_t> calls).</typeparam>
			/// <param name="fragment">A constant reference to the fragment (typically a triangle) to set. The fragment must provide vertexA, vertexB, and vertexC members; each vertex must expose a numeric z component used for depth-based Q computation.</param>
			/// <returns>true if the fragment was accepted by the base SetTriangle call and internal Q values (Qa, Qb, Qc) were computed; false if the base SetTriangle call failed.</returns>
			template<typename fragment_t>
			bool SetFragmentData(const fragment_t& fragment)
			{
				if (!Abstract::AbstractSampler::SetTriangle(fragment))
					return false;

				Qa = Qscale / MaxValue<int16_t>(1, fragment.vertexA.z);
				Qb = Qscale / MaxValue<int16_t>(1, fragment.vertexB.z);
				Qc = Qscale / MaxValue<int16_t>(1, fragment.vertexC.z);

				return true;
			}

			/// <summary>
			/// Compute normalized sample weights for a point (x, y) with respect to a triangle.
			/// </summary>
			/// <param name="x">The x coordinate of the sample point (int16_t) in the same coordinate space as the triangle vertices.</param>
			/// <param name="y">The y coordinate of the sample point (int16_t) in the same coordinate space as the triangle vertices.</param>
			/// <returns>A triangle_sample_weights_t holding three int16_t weights for vertices A, B and C (in the original vertex order).
			/// Each weight is non-negative and scaled down if necessary so the total fits within INT16_MAX while preserving relative ratios.</returns>
			triangle_sample_weights_t GetWeights(const int16_t x, const int16_t y) const
			{
				uint32_t nA{}, nB{}, nC{};
				{
					int16_t wA{}, wB{}, wC{};
					{
						const int16_t xmCx = x - Cx;
						const int16_t ymCy = y - Cy;

						wA = (static_cast<int32_t>(BmCy) * xmCx) + (static_cast<int32_t>(CmBx) * ymCy);
						wB = (static_cast<int32_t>(CmAy) * xmCx) + (static_cast<int32_t>(AmCx) * ymCy);
						wC = static_cast<int16_t>(ReducedArea) - wA - wB;

						wA = MaxValue<int16_t>(wA, 0);
						wB = MaxValue<int16_t>(wB, 0);
						wC = MaxValue<int16_t>(wC, 0);

						// Map back to original vertex order if we swapped.
						if (Swapped)
						{
							const uint16_t temp = wB;
							wB = wC;
							wC = temp;
						}
					}

					nA = static_cast<uint32_t>(wA) * Qa;
					nB = static_cast<uint32_t>(wB) * Qb;
					nC = static_cast<uint32_t>(wC) * Qc;
				}

				// Normalize to fit in int16_t if needed, preserving ratios.
				while ((nA + nB + nC) > INT16_MAX)
				{
					nA >>= 1;
					nB >>= 1;
					nC >>= 1;
				}

				return triangle_sample_weights_t{
					static_cast<int16_t>(nA),
					static_cast<int16_t>(nB),
					static_cast<int16_t>(nC) };
			}
		};
	}
}
#endif