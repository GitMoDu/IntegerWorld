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
							coordinate_t{ 0, 0 },
							coordinate_t{ 0, 0 },
							coordinate_t{ 0, 0 }
						};
					}
				};

				class Source
				{
				private:
					const coordinate_t* UvMap = nullptr;

					triangle_uv_t Uvs{};

				public:
					Source(const coordinate_t* uvMap) : UvMap(uvMap) {}

					static constexpr bool HasUvs()
					{
						return true;
					}

					const triangle_uv_t& GetUvs(const uint16_t triangleIndex)
					{
						const size_t baseIndex = static_cast<size_t>(triangleIndex) * 3;

#if defined(ARDUINO_ARCH_AVR)
						Uvs.a.x = (int16_t)pgm_read_word(&UvMap[baseIndex + 0].x);
						Uvs.a.y = (int16_t)pgm_read_word(&UvMap[baseIndex + 0].y);
						Uvs.b.x = (int16_t)pgm_read_word(&UvMap[baseIndex + 1].x);
						Uvs.b.y = (int16_t)pgm_read_word(&UvMap[baseIndex + 1].y);
						Uvs.c.x = (int16_t)pgm_read_word(&UvMap[baseIndex + 2].x);
						Uvs.c.y = (int16_t)pgm_read_word(&UvMap[baseIndex + 2].y);
#else
						Uvs.a = UvMap[baseIndex + 0];
						Uvs.b = UvMap[baseIndex + 1];
						Uvs.c = UvMap[baseIndex + 2];
#endif
						return Uvs;
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