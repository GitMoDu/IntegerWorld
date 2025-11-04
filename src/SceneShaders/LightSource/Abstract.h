#ifndef _INTEGER_WORLD_LIGHTS_SHADERS_LIGHT_SOURCE_SHADER_ABSTRACT_h
#define _INTEGER_WORLD_LIGHTS_SHADERS_LIGHT_SOURCE_SHADER_ABSTRACT_h

#include "../Abstract.h"

namespace IntegerWorld
{
	namespace SceneShaders
	{
		namespace LightSource
		{
			namespace Lighting
			{
				/// <summary>
				/// Bit shifts in the 32-bit result of DotProduct16() when using vertex16_t inputs.
				/// Used to align dot product magnitude into ufraction16_t space.
				/// </summary>
				static constexpr uint8_t DOT_SHIFTS = GetBitShifts(VERTEX16_DOT);

				/// <summary>
				/// Bit shifts representing the fixed-point scale of UFraction16::FRACTION_1X.
				/// </summary>
				static constexpr uint8_t FRAC_SHIFTS = GetBitShifts(UFRACTION16_1X);

				/// <summary>
				/// Number of shifts required to convert a positive 32-bit dot product into ufraction16_t.
				/// Equals DOT_SHIFTS - FRAC_SHIFTS.
				/// </summary>
				static constexpr uint8_t DOT_CONVERT_SHIFTS = DOT_SHIFTS - FRAC_SHIFTS;

				/// <summary>
				/// Computes the Lambertian diffuse fraction between a surface normal and a (assumed normalized) illumination vector.
				/// </summary>
				/// <param name="normal">The surface normal vector (vertex16_t).</param>
				/// <param name="illuminationVector">The illumination (light) direction vector (normalized L vector)</param>
				/// <returns>A ufraction16_t representing the Lambert diffuse contribution.
				/// If the dot product of illuminationVector and normal is positive, returns the shifted DotProduct16 value (cast to ufraction16_t);
				/// otherwise returns 0.</returns>
				static ufraction16_t GetDiffuseFraction(const vertex16_t& normal, const vertex16_t& illuminationVector)
				{
					// Compute dot product of illumination vector and normal.
					const int32_t dotProduct = DotProduct16(illuminationVector, normal);

					// Return shifted dot product as ufraction16_t if positive; otherwise return 0.
					return (dotProduct > 0)
						? static_cast<ufraction16_t>((static_cast<uint32_t>(dotProduct) >> DOT_CONVERT_SHIFTS))
						: 0;
				}

				/// <summary>
				/// Computes a Blinn–Phong style specular fraction using normalized normal and half-vector inputs, producing a ufraction16_t in the range [0,1].
				/// The function narrows the highlight by repeated squaring (effectively x^8) and interpolates between a wide and narrow lobe according to the focus parameter.
				/// </summary>
				/// <param name="normal">Reference to the surface normal (vertex16_t). Expected to be normalized; used to compute the dot product with the half-vector.</param>
				/// <param name="halfVector">Reference to the half-vector (vertex16_t) between light and view directions. Expected to be normalized; used together with the normal for the specular term.</param>
				/// <param name="focus">ufraction8_t value that controls interpolation between the wide and narrow specular responses (acts as a weight). Interpreted as a fixed-point fraction in [0,1].</param>
				/// <returns>A ufraction16_t fixed-point specular fraction in the range [0,1]. Returns 0 when the dot product of normal and half-vector is non-positive; otherwise returns an interpolated specular response between wide and narrowed lobes.</returns>
				static ufraction16_t GetSpecularFraction(const vertex16_t& normal, const vertex16_t& halfVector, const ufraction8_t focus)
				{
					// Compute dot product of normal and half-vector.
					const int32_t dotProduct = DotProduct16(halfVector, normal);

					// Return 0 if dot product is non-positive.
					if (dotProduct <= 0)
						return 0;

					// Convert dot(N, H) to ufraction16_t in [0,1]
					uint32_t narrow = static_cast<uint32_t>(dotProduct) >> DOT_CONVERT_SHIFTS;
					const uint16_t wide = narrow;

					// Narrower lobe via repeated squaring.
					narrow = (narrow * narrow) >> FRAC_SHIFTS;
					narrow = (narrow * narrow) >> FRAC_SHIFTS;
					narrow = (narrow * narrow) >> FRAC_SHIFTS;
					narrow = static_cast<ufraction16_t>(MinValue<uint32_t>(UFRACTION16_1X, narrow));

					// Interpolate between wide and narrow specular responses based on focus.
					return Fraction(focus, narrow) + Fraction(static_cast<ufraction8_t>(UFRACTION8_1X - focus), wide);
				}

				/// <summary>
				/// Computes the spot cone fraction as ufraction16_t in [0,1] from a cone alignment dot product.
				/// FocusFactor increases the steepness of the cone (higher = narrower).
				/// focusFraction blends between a narrower and a wider cone response.
				/// </summary>
				/// <typeparam name="FocusFactor">Number of squaring passes to narrow the cone, default 3</typeparam>
				/// <param name="dotProduct">Positive alignment term (e.g., -dot(Direction, L))</param>
				/// <param name="focusFraction">Blend factor in [0,1] (ufraction16_t) between narrow and wide responses</param>
				/// <returns>Cone intensity as ufraction16_t in [0,1]</returns>
				template<uint8_t FocusFactor = 3>
				static ufraction16_t GetConeFraction(const vertex16_t& illuminationVector, const vertex16_t& lightDirection, const ufraction16_t focus)
				{
					// Cone alignment: larger is more inside the cone. Assumes IlluminationVector is the normalized L.
					const int32_t coneDot = -DotProduct16(lightDirection, illuminationVector);

					if (coneDot <= 0)
						return 0;

					const ufraction16_t wide = ufraction16_t(static_cast<uint32_t>(coneDot) >> DOT_CONVERT_SHIFTS);
					ufraction16_t narrow = wide;
					for (uint_fast8_t i = 0; i < FocusFactor; i++)
					{
						narrow = (uint32_t(narrow) * narrow) >> GetBitShifts(UFRACTION16_1X);
					}

					// Interpolate between wide and narrow cone responses.
					return Fraction(focus, narrow) + Fraction(static_cast<ufraction8_t>(UFRACTION8_1X - focus), wide);
				}

				/// <summary>
				/// Computes distance-based attenuation as ufraction16_t in [0,1] from a squared distance.
				/// Returns 1 inside RangeSquaredMin, 0 beyond RangeSquaredMax, and a linear falloff between.
				/// </summary>
				/// <param name="light">Light containing RangeSquaredMin/Max</param>
				/// <param name="squaredDistance">Squared distance from light to fragment</param>
				/// <returns>Proximity/attenuation as ufraction16_t in [0,1]</returns>
				static ufraction16_t GetProximityFraction(const light_source_t& light, const uint32_t squaredDistance)
				{
					if (squaredDistance >= light.RangeSquaredMax)
					{
						return 0;
					}
					else if (squaredDistance < light.RangeSquaredMin)
					{
						return UFRACTION16_1X;
					}
					else
					{
						return UFRACTION16_1X - UFraction16::GetScalar<uint32_t>(
							static_cast<uint32_t>(squaredDistance - light.RangeSquaredMin),
							static_cast<uint32_t>((light.RangeSquaredMax - light.RangeSquaredMin)));
					}
				}

				/// <summary>
				/// Convenience overload to compute attenuation from a vector (no normalization required).
				/// Computes squared length of the world-space vector, clamps to [RangeSquaredMin, RangeSquaredMax],
				/// then maps to ufraction16_t in [0,1].
				/// </summary>
				/// <param name="light">Light containing RangeSquaredMin/Max</param>
				/// <param name="vector">Vector from fragment to light (world space)</param>
				/// <returns>Proximity/attenuation as ufraction16_t in [0,1]</returns>
				static ufraction16_t GetProximityFraction(const light_source_t& light, const vertex16_t& vector)
				{
					const uint32_t squaredDistance = LimitValue<uint32_t>(static_cast<uint32_t>(
						(static_cast<int32_t>(vector.x) * vector.x) +
						(static_cast<int32_t>(vector.y) * vector.y) +
						(static_cast<int32_t>(vector.z) * vector.z)),
						light.RangeSquaredMin,
						light.RangeSquaredMax);

					return GetProximityFraction(light, squaredDistance);
				}
			}
		}
	}
}
#endif