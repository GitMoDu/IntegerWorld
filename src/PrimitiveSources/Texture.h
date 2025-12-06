#ifndef _INTEGER_WORLD_PRIMITIVE_SOURCES_TEXTURE_h
#define _INTEGER_WORLD_PRIMITIVE_SOURCES_TEXTURE_h

#include "../Framework/Model.h"

namespace IntegerWorld
{
	namespace PrimitiveSources
	{
		namespace Texture
		{
			namespace Static
			{
				/// <summary>
				/// Read-only texture source stored in program/ROM memory.
				/// Provides random access to texels using row-major addressing.
				/// Texture dimensions must be powers of two.
				/// The texture can be offset in both X and Y directions.
				/// Wrapping is performed using bitmasking based on texture size.
				/// </summary>
				template<typename TextureSize>
				class Source
				{
				private:
					/// <summary>Pointer to texel data.</summary>
					const Rgb8::color_t* Texels = nullptr;

					static constexpr uint8_t WidthMask = TextureSize::Width - 1;
					static constexpr uint8_t HeightMask = TextureSize::Height - 1;

				public:
					int8_t OffsetX = 0;
					int8_t OffsetY = 0;

				public:
					/// <summary>
					/// Construct a static texture source.
					/// </summary>
					/// <param name="texels">Pointer to immutable texel data.</param>
					Source(const Rgb8::color_t* texels)
						: Texels(texels)
					{
					}

					/// <summary>
					/// Set the texture offsets.
					/// </summary>
					/// <param name="offsetX"></param>
					/// <param name="offsetY"></param>
					void SetOffsets(const int16_t offsetX, const int16_t offsetY)
					{
						OffsetX = offsetX;
						OffsetY = offsetY;
					}

					/// <summary>
					/// Get the texel color at coordinates (u, v).
					/// Uses row-major addressing: index = v * Width + u.
					/// No wrapping or bounds checking is performed.
					/// </summary>
					/// <param name="u">Horizontal texel coordinate (column index).</param>
					/// <param name="v">Vertical texel coordinate (row index).</param>
					/// <returns>Texel color at (u, v).</returns>
					Rgb8::color_t GetTexel(const uint8_t u, const uint8_t v) const
					{
						const uint16_t index = ((static_cast<uint16_t>(static_cast<uint8_t>(v + OffsetY) & HeightMask) * TextureSize::Width)
							+ (static_cast<uint8_t>(u + OffsetX) & WidthMask));
#if defined(ARDUINO_ARCH_AVR)
						return static_cast<Rgb8::color_t>(pgm_read_dword(&Texels[index]));
#else
						return static_cast<Rgb8::color_t>(Texels[index]);
#endif
					}
				};
			}

			namespace Dynamic
			{
				/// <summary>
				/// Read/write texture source stored in RAM.
				/// Provides random access to texels using row-major addressing.
				/// Texture dimensions must be powers of two.
				/// The texture can be offset in both X and Y directions.
				/// Wrapping is performed using bitmasking based on texture size.
				/// </summary>
				template<typename TextureSize>
				class Source
				{
				public:
					/// <summary>External texel buffer (mutable).</summary>
					Rgb8::color_t* Texels;

					static constexpr uint8_t WidthMask = TextureSize::Width - 1;
					static constexpr uint8_t HeightMask = TextureSize::Height - 1;

				public:
					int8_t OffsetX = 0;
					int8_t OffsetY = 0;

				public:
					/// <summary>
					/// Construct a dynamic texture source.
					/// </summary>
					/// <param name="texels">Pointer to texel buffer in RAM.</param>
					/// <param name="width">Texture width (row stride) in texels.</param>
					Source(Rgb8::color_t* texels)
						: Texels(texels)
					{
					}

					/// <summary>
					/// Set the texture offsets.
					/// </summary>
					/// <param name="offsetX"></param>
					/// <param name="offsetY"></param>
					void SetOffsets(const int16_t offsetX, const int16_t offsetY)
					{
						OffsetX = offsetX;
						OffsetY = offsetY;
					}

					/// <summary>
					/// Get the texel color at coordinates (u, v).
					/// Uses row-major addressing: index = v * Width + u.
					/// No wrapping or bounds checking is performed.
					/// </summary>
					/// <param name="u">Horizontal texel coordinate (column index).</param>
					/// <param name="v">Vertical texel coordinate (row index).</param>
					/// <returns>Texel color at (u, v).</returns>
					Rgb8::color_t GetTexel(const uint8_t u, const uint8_t v) const
					{
						const uint16_t index = ((static_cast<uint16_t>(static_cast<uint8_t>(v + OffsetY) & HeightMask) * TextureSize::Width)
							+ (static_cast<uint8_t>(u + OffsetX) & WidthMask));

						return static_cast<Rgb8::color_t>(Texels[index]);
					}
				};
			}
		}
	}
}
#endif