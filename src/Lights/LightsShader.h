#ifndef _INTEGER_WORLD_LIGHTS_SHADER_h
#define _INTEGER_WORLD_LIGHTS_SHADER_h

#include "AbstractLightsShader.h"

namespace IntegerWorld
{
	/// <summary>
	/// Scene shader that applies emissive + ambient, then accumulates contributions from multiple light types.
	/// Light types:
	/// - Directional: infinite light with no distance attenuation; uses normalized -Direction as L.
	/// - Point: range-based attenuation via GetProximityFraction(); both diffuse and specular are attenuated.
	/// - Spot: range-based attenuation + cone-based falloff via GetConeFraction(); both diffuse and specular are attenuated.
	/// - Camera: only affects front-facing (shade.z >= 0); attenuated by view-space z^2. When CameraPosition
	///   is not set, assumes a viewer-forward L = (0, 0, shade.z).
	///
	/// Shading model:
	/// - Diffuse: Lambert, using max(dot(N, L), 0).
	/// - Specular: Blinn�Phong, using H = normalize(L + V), requires CameraPosition and a valid normal.
	/// - Metallic: tints the specular term by the Metallic factor.
	/// </summary>
	class LightsShader : public AbstractLightsShader
	{
	private:
		const light_source_t* Lights = nullptr;
		uint8_t LightCount = 0;

	private:
		// Half-vector H scratch variable for specular calculations.
		vertex32_t HalfVector{};

	public:
		/// <summary>
		/// Pointer to the world-space camera position. When unset, specular highlights are limited.
		/// Camera lights still function using a viewer-forward L fallback for illumination only.
		/// </summary>
		const vertex16_t* CameraPosition = nullptr;

		// Ambient light color.
		Rgb8::color_t AmbientLight{};

	public:
		LightsShader() : AbstractLightsShader() {}

		/// <summary>
		/// Provide the array of active lights. Passing nullptr disables lighting.
		/// </summary>
		void SetLights(const light_source_t* lights, const uint8_t lightCount)
		{
			Lights = lights;
			LightCount = lightCount * (Lights != nullptr);
		}

		/// <summary>
		/// Shade with emissive + ambient only (no per-light processing).
		/// </summary>
		virtual void Shade(Rgb8::color_t& color, const material_t& material)
		{
			StartLightShade(color, material);

			color = EndShade();
		}

		/// <summary>
		/// Full shading: emissive + ambient, then per-light diffuse/specular accumulation.
		/// Diffuse and specular are attenuated for Point/Spot/Camera; Directional is not attenuated.
		/// </summary>
		virtual void Shade(Rgb8::color_t& color, const material_t& material, const scene_shade_t& shade)
		{
			StartLightShade(color, material);

			if (LightCount < 1)
				return;

			ufraction16_t proximityFraction = 0;
			ufraction16_t diffuseWeight = 0;
			ufraction16_t specularWeight = 0;
			for (uint8_t i = 0; i < LightCount; i++)
			{
				// Reset per-light accumulators
				diffuseWeight = 0;
				specularWeight = 0;
				proximityFraction = 0;

				const light_source_t& light = Lights[i];

				if (light.Color == 0)
					continue;

				// Calculate illumination vector based on light type.
				switch (light.Type)
				{
				case LightTypeEnum::Point:
				case LightTypeEnum::Spot:
					// L = (light.Position - P)
					IlluminationVector = {
						(int32_t(light.Position.x) - shade.position.x),
						(int32_t(light.Position.y) - shade.position.y),
						(int32_t(light.Position.z) - shade.position.z)
					};

					// Distance falloff before normalization.
					proximityFraction = GetProximityFraction(light, IlluminationVector);
					if (proximityFraction == 0)
						continue;

					// Normalize for dot computations.
					NormalizeVertex32Fast(IlluminationVector);
					break;
				case LightTypeEnum::Directional:
					// Use -Direction as L (Direction must already be normalized).
					IlluminationVector = { -light.Direction.x, -light.Direction.y, -light.Direction.z };
					break;
				case LightTypeEnum::Camera:
					// Camera light only contributes in front of the viewer.
					if (shade.z < 0)
						continue;

					// Distance falloff from view-space z^2.
					proximityFraction = GetProximityFraction(light, uint32_t(shade.z) * shade.z);
					if (proximityFraction == 0)
						continue;

					// L = (camera.Position - P) when available; otherwise a viewer-forward fallback.
					if (CameraPosition != nullptr)
					{
						IlluminationVector = {
							int32_t(CameraPosition->x) - shade.position.x,
							int32_t(CameraPosition->y) - shade.position.y,
							int32_t(CameraPosition->z) - shade.position.z
						};
					}
					else
					{
						// No camera position: approximate L with forward view direction.
						IlluminationVector = { 0, 0, shade.z };
					}
					NormalizeVertex32Fast(IlluminationVector);
					break;
				default:
					continue;
				}

				// Check if we have a valid normal.
				const bool hasNormal = (shade.normal.x != 0 || shade.normal.y != 0 || shade.normal.z != 0);

				// Compute half vector only if needed and camera is available (specular requires V).
				if (hasNormal && material.Specular > 0 && CameraPosition != nullptr)
				{
					// V = (camera.Position - P)
					HalfVector = { int32_t(CameraPosition->x) - shade.position.x,
									int32_t(CameraPosition->y) - shade.position.y,
									int32_t(CameraPosition->z) - shade.position.z };

					// H = normalize(L + V). Normalize call accounts for scale.
					HalfVector = { static_cast<int16_t>(SignedRightShift(IlluminationVector.x + HalfVector.x, 1)),
										 static_cast<int16_t>(SignedRightShift(IlluminationVector.y + HalfVector.y, 1)),
										 static_cast<int16_t>(SignedRightShift(IlluminationVector.z + HalfVector.z, 1)) };
					NormalizeVertex32Fast(HalfVector);
				}
				else
				{
					HalfVector = { 0, 0, 0 };
				}

				// Compute diffuse/specular by light type.
				switch (light.Type)
				{
				case LightTypeEnum::Point:
				case LightTypeEnum::Camera:
					if (hasNormal)
					{
						GetWeightsLambertBlinnPhong(shade.normal, HalfVector, diffuseWeight, specularWeight);
						// Attenuate both diffuse and specular by distance.
						diffuseWeight = UFraction16::Fraction(proximityFraction, diffuseWeight);
						specularWeight = UFraction16::Fraction(proximityFraction, specularWeight);
					}
					else
					{
						// No normal: use proximity fraction as diffuse/specular.
						diffuseWeight = proximityFraction;
						specularWeight = proximityFraction;
					}
					break;
				case LightTypeEnum::Spot:
				{
					const ufraction16_t coneFraction = GetConeFraction(light.Direction, light.Parameter);
					if (hasNormal)
					{
						GetWeightsLambertBlinnPhong(shade.normal, HalfVector, diffuseWeight, specularWeight);
						// Attenuate by cone and distance.
						diffuseWeight = UFraction16::Fraction(coneFraction, UFraction16::Fraction(proximityFraction, diffuseWeight));
						specularWeight = UFraction16::Fraction(coneFraction, UFraction16::Fraction(proximityFraction, specularWeight));
					}
					else
					{
						// No normal: use cone fraction as diffuse/specular.
						diffuseWeight = coneFraction;
						specularWeight = coneFraction;
					}
				}
				break;
				case LightTypeEnum::Directional:
					if (hasNormal)
					{
						GetWeightsLambertBlinnPhong(shade.normal, HalfVector, diffuseWeight, specularWeight);
					}
					else
					{
						// No normal available: use provided flat diffuse parameter.
						diffuseWeight = light.Parameter;
						specularWeight = 0;
					}
					break;
				default:
					continue;
				}

				// Scale by material properties.
				diffuseWeight = UFraction16::Fraction(material.Diffuse, diffuseWeight);
				specularWeight = UFraction16::Fraction(material.Specular, specularWeight);

				// Fetch light color components.
				const Rgb8::component_t lightR = Rgb8::Red(light.Color);
				const Rgb8::component_t lightG = Rgb8::Green(light.Color);
				const Rgb8::component_t lightB = Rgb8::Blue(light.Color);

				// Apply diffuse contribution (albedo scaled by light color).
				if (diffuseWeight > 0)
				{
					ColorMix(UFraction16::Fraction(diffuseWeight, static_cast<uint8_t>((uint16_t(FragmentR) * lightR) >> 8)),
						UFraction16::Fraction(diffuseWeight, static_cast<uint8_t>((uint16_t(FragmentG) * lightG) >> 8)),
						UFraction16::Fraction(diffuseWeight, static_cast<uint8_t>((uint16_t(FragmentB) * lightB) >> 8)));
				}

				// Apply specular contribution (metallic-tinted).
				if (specularWeight > 0)
				{
					ColorMix(UFraction16::Fraction(specularWeight,
						Fraction::Interpolate(material.Metallic, lightR, UFraction16::Fraction(specularWeight, FragmentR))),
						UFraction16::Fraction(specularWeight,
							Fraction::Interpolate(material.Metallic, lightG, UFraction16::Fraction(specularWeight, FragmentG))),
						UFraction16::Fraction(specularWeight,
							Fraction::Interpolate(material.Metallic, lightB, UFraction16::Fraction(specularWeight, FragmentB))));
				}
			}

			color = EndShade();
		}

	private:
		/// <summary>
		/// Initializes per-fragment state, then applies emissive and ambient contributions.
		/// - Emissive scales the base color and seeds the accumulator.
		/// - Ambient mixes the ambient light color modulated by material.Diffuse.
		/// </summary>
		void StartLightShade(Rgb8::color_t& color, const material_t& material)
		{
			StartShade(color);

			// Start with emissive.
			if (material.Emissive > 0)
			{
				// Apply emissivity first.
				ShadeR = UFraction16::Fraction(material.Emissive, FragmentR);
				ShadeG = UFraction16::Fraction(material.Emissive, FragmentG);
				ShadeB = UFraction16::Fraction(material.Emissive, FragmentB);
			}

			// Apply ambient light next (scaled by material.Diffuse).
			if (material.Diffuse > 0)
			{
				ColorMix(UFraction16::Fraction(material.Diffuse,
					uint8_t((uint16_t(FragmentR) * Rgb8::Red(AmbientLight)) >> 8)),
					UFraction16::Fraction(material.Diffuse,
						uint8_t((uint16_t(FragmentG) * Rgb8::Green(AmbientLight)) >> 8)),
					UFraction16::Fraction(material.Diffuse,
						uint8_t((uint16_t(FragmentB) * Rgb8::Blue(AmbientLight)) >> 8)));
			}
		}
	};
}
#endif