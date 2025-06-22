#ifndef _TINY_ASSETS_h
#define _TINY_ASSETS_h

#include <IntegerWorld.h>

namespace Assets
{
	using namespace IntegerWorld;

	namespace Objects
	{
		struct CubeEdgeObject : public EdgeSingleColorSingleMaterialObject<Shapes::Cube::VertexCount, Shapes::Cube::EdgeCount>
		{
			CubeEdgeObject(const EdgeDrawModeEnum edgeDrawMode = EdgeDrawModeEnum::NoCulling)
				: EdgeSingleColorSingleMaterialObject<Shapes::Cube::VertexCount, Shapes::Cube::EdgeCount>(
					Shapes::Cube::Vertices,
					Shapes::Cube::Edges, edgeDrawMode) {
			}
		};

		struct OctahedronEdgeObject : public EdgeSingleColorSingleMaterialObject<Shapes::Octahedron::VertexCount, Shapes::Octahedron::EdgeCount>
		{
			OctahedronEdgeObject(const EdgeDrawModeEnum edgeDrawMode = EdgeDrawModeEnum::NoCulling)
				: EdgeSingleColorSingleMaterialObject<Shapes::Octahedron::VertexCount, Shapes::Octahedron::EdgeCount>(
					Shapes::Octahedron::Vertices,
					Shapes::Octahedron::Edges, edgeDrawMode) {
			}
		};
	}
}
#endif