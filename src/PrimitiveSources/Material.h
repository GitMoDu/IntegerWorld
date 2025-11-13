#ifndef _INTEGER_WORLD_PRIMITIVE_SOURCES_MATERIAL_h
#define _INTEGER_WORLD_PRIMITIVE_SOURCES_MATERIAL_h

#include "../Framework/Model.h"

namespace IntegerWorld
{
	namespace PrimitiveSources
	{
		namespace Material
		{
			namespace Static
			{
				template<ufraction8_t rough, // Surface roughness affecting light scattering. Inversely related to shininess.
					ufraction8_t gloss, // Controls the sharpness of specular highlights and fresnel effect.
					ufraction8_t specularTint, // Tints specular highlights towards the albedo color at low incidence angles.
					fraction8_t fresnel // Strength of the fresnel effect on specular reflections at glancing angles. Positive values increases specular, negative values increase diffuse.
				>
				struct TemplateSource
				{
					static constexpr bool HasMaterials()
					{
						return true;
					}

					static constexpr material_t GetMaterial(const uint16_t index)
					{
						return material_t{ rough, gloss, specularTint, fresnel };
					}
				};

				class Source
				{
				private:
					const material_t* Materials = nullptr;

				public:
					Source(const material_t* materials) : Materials(materials) {}

					static constexpr bool HasMaterials()
					{
						return true;
					}

					const material_t& GetMaterial(const uint16_t index) const
					{
#if defined(ARDUINO_ARCH_AVR)
						return material_t{
							static_cast<ufraction8_t>(pgm_read_byte(&Materials[index].Rough)),
							static_cast<ufraction8_t>(pgm_read_byte(&Materials[index].Shine)),
							static_cast<ufraction8_t>(pgm_read_byte(&Materials[index].SpecularTint)),
							static_cast<ufraction8_t>(pgm_read_byte(&Materials[index].Fresnel))
						};
#else
						return Materials[index];
#endif
					}
				};

				class PalletedSource
				{
				private:
					const material_t* Materials = nullptr;
					const uint8_t* PalleteIndexes = nullptr;

				public:
					PalletedSource(const material_t* materials, const uint8_t* palleteIndexes)
						: Materials(materials), PalleteIndexes(palleteIndexes)
					{
					}

					static constexpr bool HasMaterials()
					{
						return true;
					}

					const material_t& GetMaterial(const uint16_t index) const
					{
#if defined(ARDUINO_ARCH_AVR)
						const uint16_t palleteIndex = static_cast<uint16_t>(pgm_read_word(&PalleteIndexes[index]));
						return material_t{
							static_cast<ufraction8_t>(pgm_read_byte(&Materials[palleteIndex].Rough)),
							static_cast<ufraction8_t>(pgm_read_byte(&Materials[palleteIndex].Shine)),
							static_cast<ufraction8_t>(pgm_read_byte(&Materials[palleteIndex].SpecularTint)),
							static_cast<ufraction8_t>(pgm_read_byte(&Materials[palleteIndex].Fresnel)) };
#else
						const uint8_t palleteIndex = PalleteIndexes[index];
						return Materials[palleteIndex];
#endif
					}
				};
			}

			namespace Dynamic
			{
				struct SingleSource
				{
					material_t Material{ UFRACTION8_1X, 0, 0, 0 };

					static constexpr bool HasMaterials()
					{
						return true;
					}

					const material_t& GetMaterial(const uint16_t index) const
					{
						return Material;
					}
				};
			}

			using DiffuseMaterialSource = Static::TemplateSource<UFRACTION8_1X, 0, 0, 0>;
			using GlassyMaterialSource = Static::TemplateSource<0, UFRACTION8_1X, 0, 0>;

			static constexpr DiffuseMaterialSource DiffuseMaterialSourceInstance{};
			static constexpr GlassyMaterialSource GlassyMaterialSourceInstance{};
		}
	}
}
#endif
