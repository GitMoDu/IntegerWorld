#ifndef _INTEGER_WORLD_ABSTRACT_LIGHTS_SHADER_h
#define _INTEGER_WORLD_ABSTRACT_LIGHTS_SHADER_h

#include "Model.h"

namespace IntegerWorld
{
	/// <summary>
	/// Lighting helpers and common math used by scene shaders that implement multiple light types.
	///
	/// This class extends AbstractSceneShader with reusable calculations for:
	/// - Directional lights (infinite distance, no attenuation)
	/// - Point lights (position + range-based attenuation)
	/// - Spot lights (position + cone + range-based attenuation)
	/// - Camera-style lights (derived shaders may position at the camera and attenuate by view depth)
	///
	/// Fixed-point fractions:
	/// - All lighting weights/attenuations returned by this class are ufraction16_t fixed-point values in [0,1].
	/// - Typical material scalars (Diffuse/Specular/Metallic) are ufraction8_t.
	///
	/// It provides fixed-point friendly implementations of:
	/// - Diffuse weighting (Lambert) using dot(N, L) where L = IlluminationVector (normalized)
	/// - Specular weighting (Blinn–Phong) using dot(N, H)^p where H = normalize(L + V)
	/// - Distance-based attenuation between RangeSquaredMin and RangeSquaredMax
	/// - Cone falloff/softness for spot lights with a tunable focus
	///
	/// Notes and conventions:
	/// - All vertex16_t vectors used in dot products must be normalized to the VERTEX16 range.
	/// - IlluminationVector must be set and normalized by derived shaders before calling Get*Weights().
	/// - For spot and directional lights, light.Direction must be normalized (use light.SetDirectionVector()).
	/// - GetWeightsLambertBlinnPhong() does not apply distance or cone attenuation and does not scale by
	///   material or light color — it only returns ufraction16_t diffuse/specular fractions in [0,1].
	/// - Specular requires a normalized half-vector H. Computing H typically uses a normalized view vector V
	///   from the fragment toward the camera in the derived shader.
	///
	/// Typical usage in a derived shader:
	/// 1) StartShade(color)
	/// 2) For each light:
	///    - Compute IlluminationVector (direction from point toward the light) and normalize it
	///    - Optionally compute H = normalize(L + V) if specular is desired
	///    - Query Lambert/Blinn–Phong weights via GetWeightsLambertBlinnPhong(...)
	///    - Apply attenuation via GetProximityFraction(...) and, for spots, GetConeFraction(...)
	///    - Scale by material/light terms and ColorMix(...)
	/// 3) color = EndShade()
	/// </summary>
	class AbstractLightsShader : public AbstractSceneShader
	{
	protected:
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

	private:
		/// <summary>
		/// Optional array of lights available to the shader (not used by helpers here).
		/// Derived shaders may set and iterate these when implementing Shade().
		/// </summary>
		const light_source_t* Lights = nullptr;

		/// <summary>
		/// Number of entries in Lights (not used by helpers here).
		/// </summary>
		uint8_t LightCount = 0;

	protected:
		/// <summary>
		/// Illumination vector L: direction from the shaded point toward the light (normalized).
		/// Must be provided/updated by derived shaders before calling Get*Weights().
		/// </summary>
		vertex32_t IlluminationVector{ 0, 0, 0 };

	public:
		AbstractLightsShader() : AbstractSceneShader() {}

	protected:
		/// <summary>
		/// Computes diffuse/specular fractions using normalized inputs:
		/// - Diffuse: Lambert with L = IlluminationVector
		/// - Specular: Blinn–Phong with provided H = normalize(L + V)
		/// Outputs:
		/// - diffuse/specular: ufraction16_t fractions in [0,1]. No attenuation or material/light scaling is applied.
		/// </summary>
		void GetWeightsLambertBlinnPhong(const vertex16_t& normal, const vertex32_t& halfVector, ufraction16_t& diffuse, ufraction16_t& specular)
		{
			// Diffuse: Lambert. Assumes IlluminationVector is the normalized L vector.
			const int32_t lightDot = DotProduct16(IlluminationVector, normal);
			if (lightDot > 0)
			{
				diffuse = ufraction16_t(static_cast<uint32_t>(lightDot) >> DOT_CONVERT_SHIFTS);
			}

			// Specular: Blinn–Phong. Assumes halfVector is normalized H = normalize(L + V).
			specular = GetSpecularFraction(normal, halfVector);
		}

		/// <summary>
		/// Spot cone intensity in [0,1] as ufraction16_t. Requires normalized inputs:
		/// - lightDirection: the light’s axis (normalized)
		/// - IlluminationVector: L (normalized, set by the caller)
		/// Uses a tunable focus to sharpen the cone response.
		/// </summary>
		ufraction16_t GetConeFraction(const vertex16_t& lightDirection, const ufraction16_t focusFraction) const
		{
			// Cone alignment: larger is more inside the cone. Assumes IlluminationVector is the normalized L.
			const int32_t coneDot = -DotProduct16(lightDirection, IlluminationVector);

			if (coneDot > 0)
			{
				return TemplateConeFraction<>(coneDot, focusFraction);
			}
			else
			{
				return 0;
			}
		}

	protected:
		/// <summary>
		/// Blinn–Phong specular term using normalized N and H.
		/// Approximates the exponent by squaring to sharpen the highlight.
		/// Returns ufraction16_t in [0,1].
		/// </summary>
		static ufraction16_t GetSpecularFraction(const vertex16_t& normal, const vertex32_t& halfVector)
		{
			// Specular strength ~ max(dot(N, H), 0)^p (p approximated via squaring in fixed-point).
			const int32_t dotProduct = DotProduct16(normal, halfVector);

			uint16_t specular = 0;
			if (dotProduct > 0)
			{
				// Scale dot, square and downscale to sharpen highlight.
				specular = ufraction16_t(uint32_t(dotProduct) >> (DOT_CONVERT_SHIFTS - 1));
				specular = (uint32_t(specular) * specular) >> (GetBitShifts(UFRACTION16_1X) + 2);
			}

			return static_cast<ufraction16_t>(specular);
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
		static ufraction16_t TemplateConeFraction(const int32_t dotProduct, const ufraction16_t focusFraction)
		{
			const ufraction16_t wideFraction = ufraction16_t(static_cast<uint32_t>(dotProduct) >> DOT_CONVERT_SHIFTS);
			ufraction16_t narrowFraction = wideFraction;
			for (uint_fast8_t i = 0; i < FocusFactor; i++)
			{
				narrowFraction = (uint32_t(narrowFraction) * narrowFraction) >> GetBitShifts(UFRACTION16_1X);
			}

			// Interpolate between wide and narrow cone responses.
			return Interpolate(focusFraction, narrowFraction, wideFraction);
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
		static ufraction16_t GetProximityFraction(const light_source_t& light, const vertex32_t& vector)
		{
			const uint32_t squaredDistance = LimitValue<uint32_t>(static_cast<uint32_t>(
				(int32_t(vector.x) * vector.x) +
				(int32_t(vector.y) * vector.y) +
				(int32_t(vector.z) * vector.z)),
				light.RangeSquaredMin,
				light.RangeSquaredMax);

			return GetProximityFraction(light, squaredDistance);
		}
	};
}
#endif