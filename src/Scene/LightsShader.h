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
		color_fraction16_t AmbientLight{};

	private:
		color_fraction16_t Tinted{};
		color_fraction16_t LightColor{};
		color_fraction16_t FragmentColor{};


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
		void Shade(color_fraction16_t& color, const material_t& material) final
		{
			FragmentColor = color;
			ApplyBaseLight(color, material);
		}

		void Shade(color_fraction16_t& color, const material_t& material, const world_position_shade_t& shade) final
		{
			TemplateShade(color, material, shade);
		}

		void Shade(color_fraction16_t& color, const material_t& material, const world_position_normal_shade_t& shade) final
		{
			TemplateShade(color, material, shade);
		}

	protected:
		void ColorMix(color_fraction16_t& color, const ufraction16_t rMix, const ufraction16_t gMix, const ufraction16_t bMix)
		{
			color.r = MinValue(uint32_t(UFRACTION16_1X), uint32_t(color.r + rMix));
			color.g = MinValue(uint32_t(UFRACTION16_1X), uint32_t(color.g + gMix));
			color.b = MinValue(uint32_t(UFRACTION16_1X), uint32_t(color.b + bMix));
		}

		void ColorMix(color_fraction16_t& color, const color_fraction16_t mix)
		{
			ColorMix(color, mix.r, mix.g, mix.b);
		}

	private:
		void ApplyDiffuse(color_fraction16_t& color, const ufraction16_t diffuseWeight)
		{
			if (!DiffuseLightEnabled)
				return;

			if (diffuseWeight > 0)
			{
				ColorMix(color,
					Fraction::Scale(diffuseWeight, Fraction::Scale(FragmentColor.r, LightColor.r)),
					Fraction::Scale(diffuseWeight, Fraction::Scale(FragmentColor.g, LightColor.g)),
					Fraction::Scale(diffuseWeight, Fraction::Scale(FragmentColor.b, LightColor.b)));
			}
		}

		void ApplySpecular(color_fraction16_t& color, const material_t& material, const ufraction16_t specularWeight)
		{
			if (!SpecularLightEnabled)
				return;

			if (specularWeight > 0)
			{
				// Metals tint the specular light at low specular fractions.
				ColorMix(color,
					Fraction::Scale(specularWeight,
						Fraction::Interpolate(material.Metallic, LightColor.r, Fraction::Scale(specularWeight, FragmentColor.r))),
					Fraction::Scale(specularWeight,
						Fraction::Interpolate(material.Metallic, LightColor.g, Fraction::Scale(specularWeight, FragmentColor.g))),
					Fraction::Scale(specularWeight,
						Fraction::Interpolate(material.Metallic, LightColor.b, Fraction::Scale(specularWeight, FragmentColor.b))));
			}
		}

		void ApplyBaseLight(color_fraction16_t& color, const material_t& material)
		{
			// 15 bit output color components.
			if (EmissiveLightEnabled && material.Emissive > 0)
			{
				// Apply emissivity first.
				color.r = Fraction::Scale(material.Emissive, FragmentColor.r);
				color.g = Fraction::Scale(material.Emissive, FragmentColor.g);
				color.b = Fraction::Scale(material.Emissive, FragmentColor.b);
			}
			else
			{
				color = { 0,0,0 };
			}

			// Apply ambient light next.
			if (AmbientLightEnabled && material.Diffuse > 0)
			{
				ColorMix(color,
					Fraction::Scale(material.Diffuse, Fraction::Scale(FragmentColor.r, AmbientLight.r)),
					Fraction::Scale(material.Diffuse, Fraction::Scale(FragmentColor.g, AmbientLight.g)),
					Fraction::Scale(material.Diffuse, Fraction::Scale(FragmentColor.b, AmbientLight.b)));
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
		void TemplateShade(color_fraction16_t& color, const material_t& material, const shade_t& shade)
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