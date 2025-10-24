#ifndef _INTEGER_WORLD_PRIMITIVE_SOURCES_TRIANGLE_h
#define _INTEGER_WORLD_PRIMITIVE_SOURCES_TRIANGLE_h

#include "../Framework/Model.h"

namespace IntegerWorld
{
	namespace PrimitiveSources
	{
		namespace Triangle
		{
			namespace Static
			{
				class Source
				{
				private:
					const triangle_face_t* Triangles = nullptr;

				public:
					Source(const triangle_face_t* triangles) : Triangles(triangles) {}

					/// <summary>
					/// Returns a triangle face from ROM. 
					/// </summary>
					triangle_face_t GetTriangle(const uint16_t index) const
					{
#if defined(ARDUINO_ARCH_AVR)
						return triangle_face_t{
							static_cast<uint16_t>(pgm_read_word(&Triangles[index].a)),
							static_cast<uint16_t>(pgm_read_word(&Triangles[index].b)),
							static_cast<uint16_t>(pgm_read_word(&Triangles[index].c))
						};
#else
						return Triangles[index];
#endif
					}
				};
			}

			namespace Dynamic
			{
				class Source
				{
				private:
					// External triangle array.
					triangle_face_t* Triangles = nullptr;

				public:
					Source(triangle_face_t* triangles) : Triangles(triangles) {}

					/// <summary>
					/// Returns a reference to a triangle face from RAM.
					/// </summary>
					const triangle_face_t& GetTriangle(const uint16_t index) const
					{
						return Triangles[index];
					}
				};
			}
		}
	}
}
#endif