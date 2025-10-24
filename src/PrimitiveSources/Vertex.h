#ifndef _INTEGER_WORLD_PRIMITIVE_SOURCES_VERTEX_h
#define _INTEGER_WORLD_PRIMITIVE_SOURCES_VERTEX_h

#include "../Framework/Model.h"

namespace IntegerWorld
{
	namespace PrimitiveSources
	{
		namespace Vertex
		{
			namespace Static
			{
				class Source
				{
				private:
					const vertex16_t* Vertices = nullptr;

				public:
					Source(const vertex16_t* vertices) : Vertices(vertices) {}

					vertex16_t GetVertex(const uint16_t index) const
					{
#if defined(ARDUINO_ARCH_AVR)
						return vertex16_t{
							static_cast<int16_t>(pgm_read_word(&Vertices[index].x)),
							static_cast<int16_t>(pgm_read_word(&Vertices[index].y)),
							static_cast<int16_t>(pgm_read_word(&Vertices[index].z))
						};
#else
						return Vertices[index];
#endif
					}
				};
			}

			namespace Dynamic
			{
				class Source
				{
				private:
					// External vertex array.
					vertex16_t* Vertices = nullptr;

				public:
					Source(vertex16_t* vertices) : Vertices(vertices) {}

					vertex16_t GetVertex(const uint16_t index) const
					{
						return Vertices[index];
					}
				};
			}
		}
	}
}
#endif