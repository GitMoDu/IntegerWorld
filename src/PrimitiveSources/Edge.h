#ifndef _INTEGER_WORLD_PRIMITIVE_SOURCES_EDGE_h
#define _INTEGER_WORLD_PRIMITIVE_SOURCES_EDGE_h

#include "../Framework/Model.h"

namespace IntegerWorld
{
	namespace PrimitiveSources
	{
		namespace Edge
		{
			namespace Static
			{
				class Source
				{
				private:
					const edge_line_t* Edges = nullptr;

				public:
					Source(const edge_line_t* edges) : Edges(edges) {}

					/// <summary>
					/// Returns a reference to an edge line from ROM.
					/// </summary>
					const edge_line_t GetEdge(const uint16_t index) const
					{
#if defined(ARDUINO_ARCH_AVR)
						return edge_line_t{
							static_cast<uint16_t>(pgm_read_word(&Edges[index].a)),
							static_cast<uint16_t>(pgm_read_word(&Edges[index].b))
						};
#else
						return Edges[index];
#endif
					}
				};
			}

			namespace Dynamic
			{
				class Source
				{
				private:
					// External edge array.
					edge_line_t* Edges = nullptr;

				public:
					Source(edge_line_t* edges) : Edges(edges) {}

					/// <summary>
					/// Returns a reference to an edge line from RAM.
					/// </summary>
					const edge_line_t& GetEdge(const uint16_t index) const
					{
						return Edges[index];
					}
				};
			}
		}
	}
}
#endif