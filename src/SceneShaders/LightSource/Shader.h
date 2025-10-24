#ifndef _INTEGER_WORLD_SCENE_SHADERS_LIGHT_SOURCE_SHADER_h
#define _INTEGER_WORLD_SCENE_SHADERS_LIGHT_SOURCE_SHADER_h

#define INTEGER_WORLD_LIGHTS_SHADER_DEBUG // Enable material component toggles.

#include "Abstract.h"

namespace IntegerWorld
{
	namespace SceneShaders
	{
		namespace LightSource
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
			/// - Specular: Blinn–Phong, using H = normalize(L + V), requires CameraPosition and a valid normal.
			/// - Metallic: tints the specular term by the Metallic factor.
			/// </summary>
			class Shader : public AbstractShader
			{
			private:
				const light_source_t* Lights = nullptr;
				uint8_t LightCount = 0;

			private:
				// Half-vector H scratch variable for specular calculations.
				vertex16_t HalfVector{};

#if defined(INTEGER_WORLD_LIGHTS_SHADER_DEBUG)
				// Material component toggles for debugging.
			public:
				bool Ambient = true;
				bool Emissive = true;
				bool Diffuse = true;
				bool Specular = true;
#endif

			public:
				/// <summary>
				/// Pointer to the world-space camera position. When unset, specular highlights are limited.
				/// Camera lights still function using a viewer-forward L fallback for illumination only.
				/// </summary>
				const vertex16_t* CameraPosition = nullptr;

				// Ambient light color.
				Rgb8::color_t AmbientLight{};

			public:
				Shader() : AbstractShader() {}

				/// <summary>
				/// Provide the array of active lights. Passing nullptr disables lighting.
				/// </summary>
				void SetLights(const light_source_t* lights, const uint8_t lightCount)
				{
					Lights = lights;
					LightCount = lightCount * (Lights != nullptr);
				}

				virtual Rgb8::color_t GetLitColor(const Rgb8::color_t albedo, const material_t& material, const vertex16_t& position, const vertex16_t& normal)
				{
					// Start with ambient light as the base for shading.
#if defined(INTEGER_WORLD_LIGHTS_SHADER_DEBUG)
					if (Ambient)
					{
						StartShade(Fraction(material.Diffuse, Rgb8::Red(AmbientLight)),
							Fraction(material.Diffuse, Rgb8::Green(AmbientLight)),
							Fraction(material.Diffuse, Rgb8::Blue(AmbientLight)));
					}
					else
					{
						StartShade();
					}
#else
					StartShade(Fraction(material.Diffuse, Rgb8::Red(AmbientLight)),
						Fraction(material.Diffuse, Rgb8::Green(AmbientLight)),
						Fraction(material.Diffuse, Rgb8::Blue(AmbientLight)));
#endif


					const Rgb8::component_t albedoR = Rgb8::Red(albedo);
					const Rgb8::component_t albedoG = Rgb8::Green(albedo);
					const Rgb8::component_t albedoB = Rgb8::Blue(albedo);

					// Apply emissivity next.
					if (material.Emissive > 0
#if defined(INTEGER_WORLD_LIGHTS_SHADER_DEBUG)
						&& Emissive
#endif
						)
					{
						AddShade(Fraction(material.Emissive, albedoR),
							Fraction(material.Emissive, albedoG),
							Fraction(material.Emissive, albedoB));
					}

					if (LightCount < 1)
						return EndShade();

					ufraction16_t proximityFraction = 0;
					ufraction16_t diffuseWeight = 0;
					ufraction16_t specularWeight = 0;
					for (uint8_t i = 0; i < LightCount; i++)
					{
						const light_source_t& light = Lights[i];

						// Reset per-light accumulators
						diffuseWeight = 0;
						specularWeight = 0;
						proximityFraction = 0;

						// Fetch light color components.
						const Rgb8::component_t lightR = Rgb8::Red(light.Color);
						const Rgb8::component_t lightG = Rgb8::Green(light.Color);
						const Rgb8::component_t lightB = Rgb8::Blue(light.Color);

						// Albedo modulated by light color.
						const Rgb8::component_t litR = static_cast<uint16_t>(lightR) * albedoR >> 8;
						const Rgb8::component_t litG = static_cast<uint16_t>(lightG) * albedoG >> 8;
						const Rgb8::component_t litB = static_cast<uint16_t>(lightB) * albedoB >> 8;

						{
							if (light.Color == 0)
								continue;

							// Calculate illumination vector based on light type.
							switch (light.Type)
							{
							case LightTypeEnum::Point:
							case LightTypeEnum::Spot:
								// L = (light.Position - P)
								IlluminationVector = {
									(int32_t(light.Position.x) - position.x),
									(int32_t(light.Position.y) - position.y),
									(int32_t(light.Position.z) - position.z)
								};

								// Distance falloff before normalization.
								proximityFraction = GetProximityFraction(light, IlluminationVector);
								if (proximityFraction == 0)
									continue;

								break;
							case LightTypeEnum::Directional:
								// Use -Direction as L (Direction must already be normalized).
								IlluminationVector = { -light.Direction.x, -light.Direction.y, -light.Direction.z };
								break;
							default:
								continue;
							}

							// Check if we have a valid normal.
							const bool hasNormal = (normal.x != 0 || normal.y != 0 || normal.z != 0);

							// Compute half vector only if needed and camera is available (specular requires V).
							if (hasNormal && material.Specular > 0 && CameraPosition != nullptr)
							{
								// V = (camera.Position - P)
								HalfVector = { CameraPosition->x - position.x,
												CameraPosition->y - position.y,
												CameraPosition->z - position.z };

								// H = normalize(L + V). Normalize call accounts for scale.
								HalfVector = { static_cast<int16_t>(SignedRightShift(static_cast<int32_t>(IlluminationVector.x) + HalfVector.x, 1)),
											static_cast<int16_t>(SignedRightShift(static_cast<int32_t>(IlluminationVector.y) + HalfVector.y, 1)),
											static_cast<int16_t>(SignedRightShift(static_cast<int32_t>(IlluminationVector.z) + HalfVector.z, 1)) };
								NormalizeVertex16(HalfVector);
							}

							// Compute diffuse/specular by light type.
							switch (light.Type)
							{
							case LightTypeEnum::Point:
								//case LightTypeEnum::Camera:
								if (hasNormal)
								{
									GetWeightsLambertBlinnPhong(normal, HalfVector, material.Metallic, diffuseWeight, specularWeight);
									// Attenuate both diffuse and specular by distance.
									diffuseWeight = Fraction(proximityFraction, diffuseWeight);
									specularWeight = Fraction(proximityFraction, specularWeight);
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
									GetWeightsLambertBlinnPhong(normal, HalfVector, material.Metallic, diffuseWeight, specularWeight);

									// Attenuate by cone and distance.
									diffuseWeight = Fraction(coneFraction, Fraction(proximityFraction, diffuseWeight));
									specularWeight = Fraction(coneFraction, Fraction(proximityFraction, specularWeight));
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
									GetWeightsLambertBlinnPhong(normal, HalfVector, material.Metallic, diffuseWeight, specularWeight);
								}
								else
								{
									// No normal available: use provided flat diffuse parameter.
									diffuseWeight = light.Parameter;
								}
								break;
							default:
								continue;
							}

							// Scale by material properties.
							diffuseWeight = Fraction(material.Diffuse, diffuseWeight);
							specularWeight = Fraction(material.Specular, specularWeight);
						}

						// Apply diffuse contribution (albedo scaled by light color).
						if (diffuseWeight > 0
#if defined(INTEGER_WORLD_LIGHTS_SHADER_DEBUG)
							&& Diffuse
#endif
							)
						{
							AddShade(Fraction(diffuseWeight, litR),
								Fraction(diffuseWeight, litG),
								Fraction(diffuseWeight, litB));
						}

						// Apply specular contribution (metallic-tinted).
						if (specularWeight > 0
#if defined(INTEGER_WORLD_LIGHTS_SHADER_DEBUG)
							&& Specular
#endif
							)
						{
							const ufraction8_t glassy = Fraction<ufraction8_t>(specularWeight, UFRACTION8_1X - material.Metallic);
							AddShade(
								Fraction(specularWeight, Interpolate(glassy, litR, lightR)),
								Fraction(specularWeight, Interpolate(glassy, litG, lightG)),
								Fraction(specularWeight, Interpolate(glassy, litB, lightB))
							);
						}
					}

					return EndShade();
				}
			};
		}
	}
}
#endif