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
				template<
					ufraction8_t emit,
					ufraction8_t rough,
					ufraction8_t shine,
					ufraction8_t gloss,
					ufraction8_t specularTint,
					fraction8_t fresnel
				>
				struct TemplateSource
				{
					static constexpr material_t StaticMaterial{ emit, rough, shine, gloss, specularTint, fresnel };
					static constexpr bool HasMaterials()
					{
						return true;
					}

					static constexpr material_t GetMaterial(const uint16_t /*index*/)
					{
						return StaticMaterial;
					}
				};

				class Source
				{
				private:
					const material_t* Materials = nullptr;

#if defined(ARDUINO_ARCH_AVR)
					material_t MaterialBuffer{};
#endif
				public:
					Source(const material_t* materials) : Materials(materials) {}

					static constexpr bool HasMaterials()
					{
						return true;
					}


#if defined(ARDUINO_ARCH_AVR)
					const material_t& GetMaterial(const uint16_t index)
					{
						MaterialBuffer.Emit = static_cast<ufraction8_t>(pgm_read_byte(&Materials[index].Emit));
						MaterialBuffer.Rough = static_cast<ufraction8_t>(pgm_read_byte(&Materials[index].Rough));
						MaterialBuffer.Shine = static_cast<ufraction8_t>(pgm_read_byte(&Materials[index].Shine));
						MaterialBuffer.Gloss = static_cast<ufraction8_t>(pgm_read_byte(&Materials[index].Gloss));
						MaterialBuffer.SpecularTint = static_cast<ufraction8_t>(pgm_read_byte(&Materials[index].SpecularTint));
						MaterialBuffer.Fresnel = static_cast<fraction8_t>(pgm_read_byte(&Materials[index].Fresnel));
						return MaterialBuffer;
					}
#else
					const material_t& GetMaterial(const uint16_t index) const
					{
						return Materials[index];
					}
#endif
				};

				class PalletedSource
				{
				private:
					const material_t* Materials = nullptr;
					const uint8_t* PalleteIndexes = nullptr;

#if defined(ARDUINO_ARCH_AVR)
					material_t MaterialBuffer{};
#endif
				public:
					PalletedSource(const material_t* materials, const uint8_t* palleteIndexes)
						: Materials(materials), PalleteIndexes(palleteIndexes)
					{
					}

					static constexpr bool HasMaterials()
					{
						return true;
					}


#if defined(ARDUINO_ARCH_AVR)
					const material_t& GetMaterial(const uint16_t index)
					{
						MaterialBuffer.Emit = static_cast<ufraction8_t>(pgm_read_byte(&Materials[index].Emit));
						MaterialBuffer.Rough = static_cast<ufraction8_t>(pgm_read_byte(&Materials[index].Rough));
						MaterialBuffer.Shine = static_cast<ufraction8_t>(pgm_read_byte(&Materials[index].Shine));
						MaterialBuffer.Gloss = static_cast<ufraction8_t>(pgm_read_byte(&Materials[index].Gloss));
						MaterialBuffer.SpecularTint = static_cast<ufraction8_t>(pgm_read_byte(&Materials[index].SpecularTint));
						MaterialBuffer.Fresnel = static_cast<fraction8_t>(pgm_read_byte(&Materials[index].Fresnel));

						return MaterialBuffer;
					}
#else
					const material_t& GetMaterial(const uint16_t index) const
					{
						const uint8_t palleteIndex = PalleteIndexes[index];
						return Materials[palleteIndex];
					}
#endif
				};
			}

			namespace Dynamic
			{
				struct SingleSource
				{
					material_t Material{ 0, UFRACTION8_1X, 0, 0, 0, 0 };

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

			using DiffuseMaterialSource = Static::TemplateSource<0, UFRACTION8_1X, 0, 0, 0, 0>;
			using GlassyMaterialSource = Static::TemplateSource<0, 0, UFRACTION8_1X, 0, 0, 0>;

			static constexpr DiffuseMaterialSource DiffuseMaterialSourceInstance{};
			static constexpr GlassyMaterialSource GlassyMaterialSourceInstance{};
		}
	}
}
#endif
