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
				template<ufraction8_t emissive,
					ufraction8_t diffuse,
					ufraction8_t specular,
					ufraction8_t metallic>
				struct TemplateSource
				{
					static constexpr bool HasMaterials()
					{
						return true;
					}

					static constexpr material_t GetMaterial(const uint16_t index)
					{
						return material_t{ emissive, diffuse, specular, metallic };
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
							static_cast<ufraction8_t>(pgm_read_byte(&Materials[index].Emissive)),
							static_cast<ufraction8_t>(pgm_read_byte(&Materials[index].Diffuse)),
							static_cast<ufraction8_t>(pgm_read_byte(&Materials[index].Specular)),
							static_cast<ufraction8_t>(pgm_read_byte(&Materials[index].Metallic))
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
							static_cast<ufraction8_t>(pgm_read_byte(&Materials[palleteIndex].Emissive)),
							static_cast<ufraction8_t>(pgm_read_byte(&Materials[palleteIndex].Diffuse)),
							static_cast<ufraction8_t>(pgm_read_byte(&Materials[palleteIndex].Specular)),
							static_cast<ufraction8_t>(pgm_read_byte(&Materials[palleteIndex].Metallic)) };
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
					material_t Material{ 0, UFRACTION8_1X, 0, 0 };

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

			using DiffuseMaterialSource = Static::TemplateSource<0, UFRACTION8_1X, 0, 0>;
			using EmissiveMaterialSource = Static::TemplateSource<UFRACTION8_1X, 0, 0, 0>;

			static constexpr DiffuseMaterialSource DiffuseMaterialSourceInstance{};
			static constexpr EmissiveMaterialSource EmissiveMaterialSourceInstance{};
		}
	}
}
#endif
