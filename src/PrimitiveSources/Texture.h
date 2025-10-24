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
				class Source
				{
				private:
					const Rgb8::color_t* Texels = nullptr;
					const uint16_t Width = 0;
					const uint16_t Height = 0;

				public:
					Source(const Rgb8::color_t* texels, const uint16_t width, const uint16_t height)
						: Texels(texels), Width(width), Height(height)
					{
					}

					Rgb8::color_t GetTexel(const uint16_t u, const uint16_t v) const
					{
						// Wrap texture coordinates.
						const size_t index = ((static_cast<size_t>(v % Height) * Width) + (u % Width));
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
				class Source
				{
				public:
					// External texture data.
					Rgb8::color_t* Texels;

					// Texture dimensions.
					uint16_t Width;
					uint16_t Height;

				public:
					Source(Rgb8::color_t* texels
						, const uint16_t width
						, const uint16_t height)
						: Texels(texels)
						, Width(width), Height(height)
					{
					}

					Rgb8::color_t GetTexel(const uint16_t u, const uint16_t v) const
					{
						// Wrap texture coordinates.
						const size_t index = ((static_cast<size_t>(v % Height) * Width) + (u % Width));
						return static_cast<Rgb8::color_t>(Texels[index]);
					}
				};
			}

		}
	}
}
#endif