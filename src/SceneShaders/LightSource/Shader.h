#ifndef _INTEGER_WORLD_SCENE_SHADERS_LIGHT_SOURCE_SHADER_h
#define _INTEGER_WORLD_SCENE_SHADERS_LIGHT_SOURCE_SHADER_h

// #define INTEGER_WORLD_LIGHTS_SHADER_DEBUG // Enable light component toggles in the scene lights shader.

#include "../Abstract.h"

namespace IntegerWorld
{
	namespace SceneShaders
	{
		namespace LightSource
		{
			/// <summary>
			/// Physically-inspired (but integer / fixed-point oriented) light accumulation shader.
			/// Supports Point, Directional, and Spot lights with:
			///  - Distance attenuation (Point/Spot)
			///  - Cone shaping (Spot)
			///  - Diffuse (Lambert)
			///  - Specular (Blinn-Phong style, integer gloss shaping)
			///  - Fresnel term (positive values bias towards grazing specular, negative invert/balance)
			/// </summary>
			class Shader : public Abstract::Shader
			{
			private:
				static constexpr uint8_t FocusSpecular = 4; // Max sharpness of specular highlights.
				static constexpr uint8_t FocusCone = 4; // Max tightness of spot light cones.
				static constexpr uint8_t FocusFresnel = 2; // Curve steepness for Fresnel modulation.

#if defined(INTEGER_WORLD_LIGHTS_SHADER_DEBUG)
			public:
				// Per-component toggles to visualize and debug lighting influences.
				bool Ambient = true;
				bool Diffuse = true;
				bool Specular = true;
				bool Fresnel = true;
#endif
			private:
				const light_source_t* Lights = nullptr;
				uint8_t LightCount = 0; // Number of active lights.

			public:
				/// <summary>
				/// Pointer to the world-space camera position. When unset, view direction is approximated
				/// as the inverse light direction (reduces specular accuracy).
				/// </summary>
				const vertex16_t* CameraPosition = nullptr;

				/// <summary>
				/// Scene ambient contribution (already in RGB8 space). Multiplied by material roughness
				/// before becoming the initial shading accumulator.
				/// </summary>
				Rgb8::color_t AmbientLight{};

			public:
				Shader() : Abstract::Shader() {}

				/// <summary>
				/// Registers the active light array. Passing nullptr disables all dynamic lighting.
				/// </summary>
				/// <param name="lights">Pointer to lights array or nullptr</param>
				/// <param name="lightCount">Count of lights in array (ignored if lights == nullptr)</param>
				void SetLights(const light_source_t* lights, const uint8_t lightCount)
				{
					Lights = lights;
					// Multiplying by (Lights != nullptr) collapses count to 0 automatically when disabled.
					LightCount = lightCount * (Lights != nullptr);
				}

				/// <summary>
				/// Computes the lit color for a fragment using material properties and active lights.
				/// Workflow per light:
				///  1) Build illumination vector (L) and view vector (V)
				///  2) Distance attenuation (Point/Spot) and cone shaping (Spot)
				///  3) Diffuse term: Lambert (N dot L) clamped and scaled
				///  4) Specular term: Blinn-Phong using half-vector (H = normalize(L + V))
				///  5) Fresnel adjustment: redistributes energy between diffuse and specular
				///  6) Accumulate scaled RGB contributions with saturation
				/// Returns ambient-only when LightCount == 0.
				/// </summary>
				/// <param name="albedo">Base fragment color (includes alpha)</param>
				/// <param name="material">Material parameters controlling light response</param>
				/// <param name="position">World-space position of fragment</param>
				/// <param name="normal">World-space normal (must be normalized by upstream stages)</param>
				/// <returns>Final lit color with source alpha preserved</returns>
				virtual Rgb8::color_t GetLitColor(const Rgb8::color_t albedo,
					const material_t& material,
					const vertex16_t& position,
					const vertex16_t& normal)
				{
					// Initialize shading accumulator with ambient * material.Rough (acts as diffuse baseline).
#if defined(INTEGER_WORLD_LIGHTS_SHADER_DEBUG)
					if (Ambient)
					{
						StartShade(Fraction(material.Rough, Rgb8::Red(AmbientLight)),
							Fraction(material.Rough, Rgb8::Green(AmbientLight)),
							Fraction(material.Rough, Rgb8::Blue(AmbientLight)));
					}
					else
					{
						StartShade(); // Debug path: start at black.
					}
#else
					StartShade(Fraction(material.Rough, Rgb8::Red(AmbientLight)),
						Fraction(material.Rough, Rgb8::Green(AmbientLight)),
						Fraction(material.Rough, Rgb8::Blue(AmbientLight)));
#endif
					const Rgb8::component_t alpha = Rgb8::Alpha(albedo);
					const Rgb8::component_t albedoR = Rgb8::Red(albedo);
					const Rgb8::component_t albedoG = Rgb8::Green(albedo);
					const Rgb8::component_t albedoB = Rgb8::Blue(albedo);

					if (material.Emit != 0)
					{
						// Add emissive component (unaffected by lighting).
						const Rgb8::component_t emitR = Fraction(material.Emit, albedoR);
						const Rgb8::component_t emitG = Fraction(material.Emit, albedoG);
						const Rgb8::component_t emitB = Fraction(material.Emit, albedoB);
						AddShade(emitR, emitG, emitB);
					}

					vertex16_t illuminationVector; // L
					vertex16_t viewVector;         // V
					for (uint8_t i = 0; i < LightCount; i++)
					{
						const light_source_t& light = Lights[i];
						if (light.Color == 0)
							continue; // Skip black / disabled lights early.

						// Per-light working fractions (all start at 0 and are built up).
						ufraction16_t proximity = 0; // Distance attenuation.
						ufraction16_t diffuse = 0; // Lambertian diffuse component.
						ufraction16_t specular = 0; // Blinn-Phong component.
						ufraction16_t fresnel = 0; // Fresnel modulation term.

						// Extract light RGB components (8-bit).
						const Rgb8::component_t lightR = Rgb8::Red(light.Color);
						const Rgb8::component_t lightG = Rgb8::Green(light.Color);
						const Rgb8::component_t lightB = Rgb8::Blue(light.Color);

						// Light * Albedo pre-scale (component-wise) using 8-bit fixed multiply >> 8.
						const Rgb8::component_t litR = static_cast<Rgb8::component_t>((static_cast<uint16_t>(lightR) * albedoR) >> 8);
						const Rgb8::component_t litG = static_cast<Rgb8::component_t>((static_cast<uint16_t>(lightG) * albedoG) >> 8);
						const Rgb8::component_t litB = static_cast<Rgb8::component_t>((static_cast<uint16_t>(lightB) * albedoB) >> 8);

						// Build illumination vector (L) per light type.
						switch (light.Type)
						{
						case LightTypeEnum::Point:
						case LightTypeEnum::Spot:
							// L = (light.Position - P)
							illuminationVector = {
								static_cast<int16_t>(light.Position.x - position.x),
								static_cast<int16_t>(light.Position.y - position.y),
								static_cast<int16_t>(light.Position.z - position.z) };

							// Distance attenuation computed before normalization (range checks).
							proximity = GetProximityFraction(light, illuminationVector);
							if (proximity == 0)
								continue; // Fully out of range.

							NormalizeVertex16(illuminationVector);
							break;
						case LightTypeEnum::Directional:
							// Direction assumed normalized; convert to illumination ray: L = -Direction
							illuminationVector = {
								static_cast<int16_t>(-light.Direction.x),
								static_cast<int16_t>(-light.Direction.y),
								static_cast<int16_t>(-light.Direction.z) };
							break;
						default:
							continue; // Unknown type: skip.
						}

						// Build view vector (V). If camera unavailable, approximate head-on incidence.
						if (CameraPosition != nullptr)
						{
							viewVector = { static_cast<int16_t>(CameraPosition->x - position.x),
								static_cast<int16_t>(CameraPosition->y - position.y),
								static_cast<int16_t>(CameraPosition->z - position.z) };
						}
						else
						{
							viewVector = {
								static_cast<int16_t>(-illuminationVector.x),
								static_cast<int16_t>(-illuminationVector.y),
								static_cast<int16_t>(-illuminationVector.z) };
						}
						NormalizeVertex16(viewVector);

						// Fresnel precomputation only if material sets a non-zero fresnel control.
						// Positive Fresnel: adds energy to specular, subtracts from diffuse.
						// Negative Fresnel: scales both diffuse and specular uniformly (inversion style).
						if (material.Fresnel != 0)
						{
							const int32_t dotFresnel = AbsValue(DotProduct16(viewVector, normal)); // |V·N|
							fresnel = DotProductToFraction(dotFresnel);
							// Convert to edge emphasis: 1 - clamp(V·N)
							fresnel = UFRACTION16_1X - MinValue<ufraction16_t>(fresnel, UFRACTION16_1X);
						}

						// Specular path only if surface has shine (inverse roughness > 0).
						if (material.Shine > 0)
						{
							// Half-vector approximation: average L and V via arithmetic right shift.
							vertex16_t halfVector = {
								static_cast<int16_t>(SignedRightShift<int32_t>(static_cast<int32_t>(illuminationVector.x) + viewVector.x, 1)),
								static_cast<int16_t>(SignedRightShift<int32_t>(static_cast<int32_t>(illuminationVector.y) + viewVector.y, 1)),
								static_cast<int16_t>(SignedRightShift<int32_t>(static_cast<int32_t>(illuminationVector.z) + viewVector.z, 1)) };
							NormalizeVertex16(halfVector);

							const int32_t dotReflection = DotProduct16(halfVector, normal);
							specular = DotProductToFraction(dotReflection);
						}

						// Diffuse/specular weighting by light type.
						switch (light.Type)
						{
						case LightTypeEnum::Point:
							if (material.Rough > 0)
							{
								const int32_t dotIllumination = DotProduct16(illuminationVector, normal);
								diffuse = Fraction(proximity, DotProductToFraction(dotIllumination));
							}
							if (specular > 0)
								specular = Fraction(proximity,
									Interpolate(material.Gloss, specular, FocusFraction<FocusSpecular>(specular)));
							break;
						case LightTypeEnum::Directional:
							if (material.Rough > 0)
							{
								const int32_t dotIllumination = DotProduct16(illuminationVector, normal);
								diffuse = DotProductToFraction(dotIllumination);
							}
							if (specular > 0)
								specular = Interpolate(material.Gloss, specular, FocusFraction<FocusSpecular>(specular));
							break;
						case LightTypeEnum::Spot:
						{
							// Cone alignment: negative of Direction·L because L points from fragment to light.
							const int32_t dotCone = -DotProduct16(light.Direction, illuminationVector);
							ufraction16_t cone = DotProductToFraction(dotCone);

							// Tighten cone with light.Parameter controlling interpolation.
							cone = Interpolate(light.Parameter, cone, FocusFraction<FocusCone>(cone));

							if (material.Rough > 0)
							{
								const int32_t dotIllumination = DotProduct16(illuminationVector, normal);
								diffuse = Fraction(proximity, Fraction(cone, DotProductToFraction(dotIllumination)));
							}

							if (specular > 0)
								specular = Fraction(proximity, Fraction(cone,
									Interpolate(material.Gloss, specular, FocusFraction<FocusSpecular>(specular))));
						}
						break;
						default:
							break;
						}

						// Fresnel redistribution.
#if defined(INTEGER_WORLD_LIGHTS_SHADER_DEBUG)
						if (Fresnel)
#endif					
							if (fresnel > 0)
							{
								// Focus fresnel based on material gloss.
								fresnel = Interpolate(material.Gloss, fresnel, FocusFraction<FocusFresnel>(fresnel));

								if (material.Fresnel > 0)
								{
									// Positive: transfer energy from diffuse to specular.
									fresnel = Fraction(material.Fresnel, fresnel);
									specular = MinValue<int32_t>(static_cast<int32_t>(specular) + fresnel, UFRACTION16_1X);
									diffuse = MaxValue<int32_t>(static_cast<int32_t>(diffuse) - fresnel, 0);
								}
								else if (material.Fresnel < 0)
								{
									// Negative: global scaling (acts like grazing dampening).
									const ufraction8_t materialScale = (static_cast<ufraction8_t>(-material.Fresnel) << 1);
									fresnel = UFRACTION16_1X - Fraction(materialScale, fresnel);
									specular = Fraction(fresnel, specular);
									diffuse = Fraction(fresnel, diffuse);
								}
							}

						// Final material application:
						diffuse = Fraction(material.Rough, diffuse);
						specular = Fraction(material.Shine, specular);

						// Diffuse accumulation (Lambert * albedo * light color).
#if defined(INTEGER_WORLD_LIGHTS_SHADER_DEBUG)
						if (Diffuse)
#endif
							AddShade(Fraction(diffuse, litR),
								Fraction(diffuse, litG),
								Fraction(diffuse, litB));

						// Specular accumulation (light color tinted towards albedo using SpecularTint).
#if defined(INTEGER_WORLD_LIGHTS_SHADER_DEBUG)
						if (Specular)
#endif
							if (specular > 0)
							{
								const ufraction8_t specularTint = Fraction(specular, material.SpecularTint);
								AddShade(Fraction(specular, Interpolate(specularTint, lightR, litR)),
									Fraction(specular, Interpolate(specularTint, lightG, litG)),
									Fraction(specular, Interpolate(specularTint, lightB, litB)));
							}
					}

					// Preserve original alpha while returning accumulated RGB.
					return EndShade(alpha);
				}

			protected:
				/// <summary>
				/// Computes a 16-bit fractional proximity value based on the squared distance from a light source, with clamped linear falloff between the light's minimum and maximum squared ranges.
				/// </summary>
				/// <param name="light">Reference to a light_source_t that provides RangeSquaredMin and RangeSquaredMax. These bounds are used to clamp the computed squared distance and to determine the proximity falloff.</param>
				/// <param name="vector">A vertex16_t containing the displacement vector (x, y, z) used to compute the squared distance from the light. The function uses the squared length of this vector (no square root).</param>
				/// <returns>A ufraction16_t proximity fraction: returns UFRACTION16_1X when the squared distance is less than RangeSquaredMin, 0 when it is greater than or equal to RangeSquaredMax, and a linearly interpolated fraction between those bounds.</returns>
				static ufraction16_t GetProximityFraction(const light_source_t& light, const vertex16_t& vector)
				{
					const uint32_t squaredDistance = LimitValue<uint32_t>(static_cast<uint32_t>(
						(static_cast<int32_t>(vector.x) * vector.x) +
						(static_cast<int32_t>(vector.y) * vector.y) +
						(static_cast<int32_t>(vector.z) * vector.z)),
						light.RangeSquaredMin,
						light.RangeSquaredMax);

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
			};
		}
	}
}
#endif