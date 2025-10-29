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
				/// </summary>
				class Source
				{
				private:
					/// <summary>Pointer to texel data.</summary>
					const Rgb8::color_t* Texels = nullptr;

					/// <summary>Texture width in texels (row stride).</summary>
					const uint16_t Width =0;

				public:
					/// <summary>
					/// Construct a static texture source.
					/// </summary>
					/// <param name="texels">Pointer to immutable texel data.</param>
					/// <param name="width">Texture width (row stride) in texels.</param>
					Source(const Rgb8::color_t* texels, const uint16_t width)
						: Texels(texels), Width(width)
					{
					}

					/// <summary>
					/// Get the texel color at coordinates (u, v).
					/// Uses row-major addressing: index = v * Width + u.
					/// No wrapping or bounds checking is performed.
					/// </summary>
					/// <param name="u">Horizontal texel coordinate (column index).</param>
					/// <param name="v">Vertical texel coordinate (row index).</param>
					/// <returns>Texel color at (u, v).</returns>
					Rgb8::color_t GetTexel(const uint16_t u, const uint16_t v) const
					{
						const size_t index = ((static_cast<size_t>(v) * Width) + (u));
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
				/// </summary>
				class Source
				{
				public:
					/// <summary>External texel buffer (mutable).</summary>
					Rgb8::color_t* Texels;

					/// <summary>Texture width in texels (row stride).</summary>
					uint16_t Width;

				public:
					/// <summary>
					/// Construct a dynamic texture source.
					/// </summary>
					/// <param name="texels">Pointer to texel buffer in RAM.</param>
					/// <param name="width">Texture width (row stride) in texels.</param>
					Source(Rgb8::color_t* texels
						, const uint16_t width)
						: Texels(texels)
						, Width(width)
					{
					}

					/// <summary>
					/// Get the texel color at coordinates (u, v).
					/// Uses row-major addressing: index = v * Width + u.
					/// No wrapping or bounds checking is performed.
					/// </summary>
					/// <param name="u">Horizontal texel coordinate (column index).</param>
					/// <param name="v">Vertical texel coordinate (row index).</param>
					/// <returns>Texel color at (u, v).</returns>
					Rgb8::color_t GetTexel(const uint16_t u, const uint16_t v) const
					{
						const size_t index = ((static_cast<size_t>(v) * Width) + (u));

						return static_cast<Rgb8::color_t>(Texels[index]);
					}
				};
			}
		}
	}
}
#endif