#ifndef _INTEGER_WORLD_ABSTRACT_SCENE_SHADER_h
#define _INTEGER_WORLD_ABSTRACT_SCENE_SHADER_h

#include "../Framework/Interface.h"

namespace IntegerWorld
{
	/// <summary>
	/// Lightweight base for scene shader implementations that compute lighting and effects.
	///
	/// This class provides a minimal per-fragment lifecycle and RGB accumulation helpers:
	/// - StartShade(): initialize per-fragment state from an input color
	/// - ColorMix(): add one or more lighting contributions with saturating arithmetic
	/// - EndShade(): finalize and return the accumulated shaded color
	///
	/// It does not implement ISceneShader::Shade() — derived shaders must implement
	/// the Shade() overloads and orchestrate the lifecycle above.
	/// 
	/// Usage pattern in a derived shader:
	/// 1) StartShade(color)            // capture base/albedo and reset accumulators
	/// 2) For each light: compute its RGB contribution and ColorMix(r, g, b)
	///    - Modulate contributions by material terms (Diffuse/Specular/etc.) and Fragment color as needed
	/// 3) color = EndShade()           // write back the final shaded color
	///
	/// Notes:
	/// - FragmentR/G/B store the base fragment color. EndShade() does not implicitly
	///   blend with FragmentR/G/B; derived shaders must apply any desired base-color modulation explicitly.
	/// - ColorMix() uses saturating adds (0..255) per component to avoid overflow when accumulating lights.
	/// </summary>
	class AbstractSceneShader : public ISceneShader
	{
	protected:
		/// <summary>
		/// Components of the base fragment color (material/texture color)
		/// </summary>
		Rgb8::component_t FragmentR{};
		Rgb8::component_t FragmentG{};
		Rgb8::component_t FragmentB{};

		/// <summary>
		/// Components of the accumulated shading result (lighting contribution)
		/// </summary>
		Rgb8::component_t ShadeR{};
		Rgb8::component_t ShadeG{};
		Rgb8::component_t ShadeB{};

	public:
		AbstractSceneShader() : ISceneShader() {}

	protected:
		/// <summary>
		/// Initializes the per-fragment state with the given input color.
		/// Captures the color into FragmentR/G/B and clears the accumulated ShadeR/G/B to black.
		/// Call once at the start of each Shade(...) implementation.
		/// </summary>
		/// <param name="color">Input color for the fragment (e.g., texture or primitive color)</param>
		void StartShade(const Rgb8::color_t& color)
		{
			// Prepare fragment base.
			FragmentR = Rgb8::Red(color);
			FragmentG = Rgb8::Green(color);
			FragmentB = Rgb8::Blue(color);

			// Start with black accumulation.
			ShadeR = 0;
			ShadeG = 0;
			ShadeB = 0;
		}

		/// <summary>
		/// Adds an RGB contribution to the current shading result using saturating (clamped) addition.
		/// Use this to accumulate per-light or per-effect colors after applying scaling/modulation.
		/// </summary>
		/// <param name="rMix">Red contribution to add (0..255)</param>
		/// <param name="gMix">Green contribution to add (0..255)</param>
		/// <param name="bMix">Blue contribution to add (0..255)</param>
		void ColorMix(const Rgb8::component_t rMix, const Rgb8::component_t gMix, const Rgb8::component_t bMix)
		{
			ShadeR = static_cast<uint8_t>(MinValue<uint16_t>(Rgb8::COMPONENT_MAX, uint16_t(ShadeR) + rMix));
			ShadeG = static_cast<uint8_t>(MinValue<uint16_t>(Rgb8::COMPONENT_MAX, uint16_t(ShadeG) + gMix));
			ShadeB = static_cast<uint8_t>(MinValue<uint16_t>(Rgb8::COMPONENT_MAX, uint16_t(ShadeB) + bMix));
		}

		/// <summary>
		/// Finalizes the shading process and returns the accumulated color.
		/// Does not implicitly combine with the original FragmentR/G/B; the return value is whatever
		/// was accumulated via ColorMix().
		/// </summary>
		/// <returns>The final RGB color produced by the accumulation pipeline</returns>
		Rgb8::color_t EndShade() const
		{
			return Rgb8::Color(ShadeR, ShadeG, ShadeB);
		}
	};
}
#endif