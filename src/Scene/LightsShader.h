#ifndef _INTEGER_WORLD_LIGHTS_SHADER_h
#define _INTEGER_WORLD_LIGHTS_SHADER_h

#include "../Framework/Interface.h"

namespace IntegerWorld
{
	template<uint8_t MaxLightCount = 3>
	class LightsShader : public ISceneShader
	{
	private:
		using LightItemType = ILightSource*;

	private:
		LightItemType Lights[MaxLightCount]{};
		uint8_t LightCount = 0;

	public:
		Rgb8::color_t AmbientLight{};

	private:
		Rgb8::color_t Tinted{};
		Rgb8::color_t LightColor{};
		Rgb8::color_t FragmentColor{};


	public:
		bool AmbientLightEnabled = true;
		bool EmissiveLightEnabled = true;
		bool DiffuseLightEnabled = true;
		bool SpecularLightEnabled = true;

	public:
		LightsShader() : ISceneShader() {}

		void ClearLights()
		{
			LightCount = 0;
		}

		bool AddLight(ILightSource* lightSource)
		{
			if (LightCount < MaxLightCount)
			{
				Lights[LightCount] = lightSource;
				LightCount++;

				return true;
			}
			else
			{
				return false;
			}
		}

	public:
		void Shade(Rgb8::color_t& color, const material_t& material) final
		{
			FragmentColor = color;
			ApplyBaseLight(color, material);
		}

		void Shade(Rgb8::color_t& color, const material_t& material, const world_position_shade_t& shade) final
		{
			TemplateShade(color, material, shade);
		}

		void Shade(Rgb8::color_t& color, const material_t& material, const world_position_normal_shade_t& shade) final
		{
			TemplateShade(color, material, shade);
		}

	protected:
		void ColorMix(Rgb8::color_t& color, const Rgb8::component_t rMix, const Rgb8::component_t gMix, const Rgb8::component_t bMix)
		{
			color = Rgb8::Color(
				uint8_t(MinValue(uint16_t(Rgb8::COMPONENT_MAX), uint16_t(Rgb8::Red(color) + rMix))),
				uint8_t(MinValue(uint16_t(Rgb8::COMPONENT_MAX), uint16_t(Rgb8::Green(color) + gMix))),
				uint8_t(MinValue(uint16_t(Rgb8::COMPONENT_MAX), uint16_t(Rgb8::Blue(color) + bMix))));
		}

		void ColorMix(Rgb8::color_t& color, const Rgb8::color_t mix)
		{
			ColorMix(color, Rgb8::Red(mix), Rgb8::Green(mix), Rgb8::Blue(mix));
		}

		static Rgb8::color_t ColorMultiply(const Rgb8::color_t a, const Rgb8::color_t b)
		{
			return Rgb8::Color(
				(uint16_t(Rgb8::Red(a)) * Rgb8::Red(b)) >> 8,
				(uint16_t(Rgb8::Green(a)) * Rgb8::Green(b)) >> 8,
				(uint16_t(Rgb8::Blue(a)) * Rgb8::Blue(b)) >> 8);
		}

	private:
		void ApplyDiffuse(Rgb8::color_t& color, const ufraction16_t diffuseWeight)
		{
			if (!DiffuseLightEnabled || diffuseWeight == 0)
				return;

			ColorMix(color,
				Fraction::Scale(diffuseWeight, uint8_t((uint16_t(Rgb8::Red(FragmentColor)) * Rgb8::Red(LightColor)) >> 8)),
				Fraction::Scale(diffuseWeight, uint8_t((uint16_t(Rgb8::Green(FragmentColor)) * Rgb8::Green(LightColor)) >> 8)),
				Fraction::Scale(diffuseWeight, uint8_t((uint16_t(Rgb8::Blue(FragmentColor)) * Rgb8::Blue(LightColor)) >> 8)));
		}

		void ApplySpecular(Rgb8::color_t& color, const material_t& material, const ufraction16_t specularWeight)
		{
			if (!SpecularLightEnabled)
				return;

			if (specularWeight > 0)
			{
				// Metals tint the specular light at low specular fractions.
				ColorMix(color,
					Fraction::Scale(specularWeight,
						Fraction::Interpolate(material.Metallic, Rgb8::Red(LightColor), Fraction::Scale(specularWeight, Rgb8::Red(FragmentColor)))),
					Fraction::Scale(specularWeight,
						Fraction::Interpolate(material.Metallic, Rgb8::Green(LightColor), Fraction::Scale(specularWeight, Rgb8::Green(FragmentColor)))),
					Fraction::Scale(specularWeight,
						Fraction::Interpolate(material.Metallic, Rgb8::Blue(LightColor), Fraction::Scale(specularWeight, Rgb8::Blue(FragmentColor)))));
			}
		}

		void ApplyBaseLight(Rgb8::color_t& color, const material_t& material)
		{
			if (EmissiveLightEnabled && material.Emissive > 0)
			{
				// Apply emissivity first.
				color = Rgb8::Color(
					Fraction::Scale(material.Emissive, Rgb8::Red(FragmentColor)),
					Fraction::Scale(material.Emissive, Rgb8::Green(FragmentColor)),
					Fraction::Scale(material.Emissive, Rgb8::Blue(FragmentColor)));
			}
			else
			{
				color = Rgb8::BLACK;
			}

			// Apply ambient light next.
			if (AmbientLightEnabled && material.Diffuse > 0)
			{
				ColorMix(color,
					Fraction::Scale(material.Diffuse,
						uint8_t((uint16_t(Rgb8::Red(FragmentColor)) * Rgb8::Red(AmbientLight)) >> 8)),
					Fraction::Scale(material.Diffuse,
						uint8_t((uint16_t(Rgb8::Green(FragmentColor)) * Rgb8::Green(AmbientLight)) >> 8)),
					Fraction::Scale(material.Diffuse,
						uint8_t((uint16_t(Rgb8::Blue(FragmentColor)) * Rgb8::Blue(AmbientLight)) >> 8)));
			}
		}

		/// <summary>
		/// Apply curve to specular weight according to material sheen.
		/// </summary>
		/// <param name="specularWeight">Source value to apply curve.</param>
		/// <param name="sheen">A material with a larger sheen will reflect less light at low angles.</param>
		/// <returns>Curved specular weight.</returns>
		ufraction16_t CurveSpecular(const ufraction16_t specularWeight, const ufraction8_t sheen) const
		{
			// Convert ufraction16_t to cover uint16_t.
			uint32_t specularCurve = ((uint32_t(specularWeight) << 1) | (specularWeight >> 14));

			// No shine.
			const uint16_t specularMin = specularCurve;

			// Curve value with power^2.
			specularCurve = (specularCurve * specularCurve) >> 16;

			// Max shine.
			const uint16_t specularMax = specularCurve;

			// Scale between no shine and full shine to get the curve.
			const uint16_t mixed = Fraction::Interpolate(sheen, specularMin, specularMax);

			// Convert down to ufraction16_t.
			return mixed >> 1;
		}

		template<typename shade_t>
		void TemplateShade(Rgb8::color_t& color, const material_t& material, const shade_t& shade)
		{
			FragmentColor = color;
			ApplyBaseLight(color, material);

			if (LightCount > 0)
			{
				// Apply diffuse and specular.
				ufraction16_t diffuseWeight = 0;
				ufraction16_t specularWeight = 0;
				for (uint8_t i = 0; i < LightCount; i++)
				{
					diffuseWeight = 0;
					specularWeight = 0;
					Lights[i]->GetLighting(LightColor, diffuseWeight, specularWeight, shade);

					specularWeight = Fraction::Scale(material.Specular, specularWeight);
					ApplySpecular(color, material, specularWeight);

					// Scale to material weight.
					diffuseWeight = Fraction::Scale(material.Diffuse, diffuseWeight);
					ApplyDiffuse(color, diffuseWeight);
				}
			}
		}
	};
}
#endif