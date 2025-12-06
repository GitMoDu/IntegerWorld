#ifndef _INTEGER_WORLD_PRIMITIVE_SOURCES_UV_h
#define _INTEGER_WORLD_PRIMITIVE_SOURCES_UV_h

#include "../Framework/Model.h"

namespace IntegerWorld
{
	namespace PrimitiveSources
	{
		namespace Uv
		{
			namespace Static
			{
				struct NoSource
				{
					static constexpr bool HasUvs()
					{
						return false;
					}

					static constexpr triangle_uv_t GetUvs(const uint16_t triangleIndex)
					{
						return triangle_uv_t{
							uv_t{ 0, 0 },
							uv_t{ 0, 0 },
							uv_t{ 0, 0 } };
					}
				};

				class Source
				{
				private:
					const uv_t* UvMap = nullptr;

#if defined(ARDUINO_ARCH_AVR)
					triangle_uv_t Uvs{};
#endif

				public:
					Source(const uv_t* uvMap) : UvMap(uvMap) {}

					static constexpr bool HasUvs()
					{
						return true;
					}

#if defined(ARDUINO_ARCH_AVR)
					const triangle_uv_t& GetUvs(const uint16_t triangleIndex)
					{
						const size_t baseIndex = static_cast<size_t>(triangleIndex) * 3;

						Uvs.a.x = (uint8_t)pgm_read_byte(&UvMap[baseIndex + 0].x);
						Uvs.a.y = (uint8_t)pgm_read_byte(&UvMap[baseIndex + 0].y);
						Uvs.b.x = (uint8_t)pgm_read_byte(&UvMap[baseIndex + 1].x);
						Uvs.b.y = (uint8_t)pgm_read_byte(&UvMap[baseIndex + 1].y);
						Uvs.c.x = (uint8_t)pgm_read_byte(&UvMap[baseIndex + 2].x);
						Uvs.c.y = (uint8_t)pgm_read_byte(&UvMap[baseIndex + 2].y);
					}
#else
					triangle_uv_t GetUvs(const uint16_t triangleIndex) const
					{
						const size_t baseIndex = static_cast<size_t>(triangleIndex) * 3;

						return triangle_uv_t{
							UvMap[baseIndex + 0],
							UvMap[baseIndex + 1],
							UvMap[baseIndex + 2] };
#endif
					}
				};
			}

			namespace Dynamic
			{

			}

			static constexpr Static::NoSource NoUvSourceInstance{};
		}
	}
}
#endif