#ifndef _INTEGER_WORLD_PRIMITIVE_SOURCES_ALBEDO_h
#define _INTEGER_WORLD_PRIMITIVE_SOURCES_ALBEDO_h

#include "../Framework/Model.h"

namespace IntegerWorld
{
	namespace PrimitiveSources
	{
		namespace Albedo
		{
			namespace Static
			{
				struct FullSource
				{
					static constexpr bool HasAlbedos()
					{
						return false;
					}

					static constexpr Rgb8::color_t GetAlbedo(const uint16_t groupIndex)
					{
						return Rgb8::WHITE;
					}
				};

				class Source
				{
				private:
					const Rgb8::color_t* Albedos = nullptr;

				public:
					Source(const Rgb8::color_t* albedos) : Albedos(albedos) {}

					static constexpr bool HasAlbedos()
					{
						return true;
					}

					Rgb8::color_t GetAlbedo(const uint16_t groupIndex) const
					{
#if defined(ARDUINO_ARCH_AVR)
						return static_cast<Rgb8::color_t>(pgm_read_dword(&Albedos[groupIndex]));
#else
						return Albedos[groupIndex];
#endif
					}
				};

				class PalletedSource
				{
				private:
					const Rgb8::color_t* Albedos = nullptr;
					const uint8_t* PalleteIndexes = nullptr;

				public:
					PalletedSource(const Rgb8::color_t* albedos, const uint8_t* palleteIndexes)
						: Albedos(albedos), PalleteIndexes(palleteIndexes)
					{
					}

					static constexpr bool HasAlbedos()
					{
						return true;
					}

					Rgb8::color_t GetAlbedo(const uint16_t groupIndex) const
					{
#if defined(ARDUINO_ARCH_AVR)
						const uint8_t palleteIndex = static_cast<uint8_t>(pgm_read_byte(&PalleteIndexes[groupIndex]));
						return static_cast<Rgb8::color_t>(pgm_read_dword(&Albedos[palleteIndex]));
#else
						const uint8_t palleteIndex = PalleteIndexes[groupIndex];
						return Albedos[palleteIndex];
#endif
					}
				};
			}

			namespace Dynamic
			{
				struct SingleSource
				{
					Rgb8::color_t Albedo = Rgb8::WHITE;

					static constexpr bool HasAlbedos()
					{
						return true;
					}

					Rgb8::color_t GetAlbedo(const uint16_t groupIndex) const
					{
						return Albedo;
					}
				};
			}

			static constexpr Static::FullSource FullAlbedoSourceInstance{};
		}
	}
}
#endif