#ifndef _TINY_ASSETS_h
#define _TINY_ASSETS_h

#include <IntegerWorld.h>

namespace Assets
{
	using namespace IntegerWorld;

	namespace Shapes
	{
		namespace Cube
		{
			static constexpr color_fraction16_t Pallete[TriangleCount]
			{
				ColorFraction::COLOR_RED,
				ColorFraction::COLOR_GREEN,
				ColorFraction::COLOR_BLUE,
				ColorFraction::RgbToColorFraction(uint32_t(0xFFFF00)),
				ColorFraction::RgbToColorFraction(uint32_t(0x00FFFF)),
				ColorFraction::RgbToColorFraction(uint32_t(0xFF00FF))
			};
			constexpr uint8_t PalleteSize = sizeof(Pallete) / sizeof(Pallete[0]);
		}
	}

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

		struct CubeMeshObject : public MeshObject<Shapes::Cube::VertexCount, Shapes::Cube::TriangleCount>
		{
			material_t Material{ 0, UFRACTION8_1X, 0, 0 };

			CubeMeshObject() : MeshObject<Shapes::Cube::VertexCount, Shapes::Cube::TriangleCount>(
				Shapes::Cube::Vertices,
				Shapes::Cube::Triangles,
				Shapes::Cube::Normals) {
			}

		protected:
			virtual void GetFragment(triangle_fragment_t& fragment, const uint16_t index)
			{
				fragment.color = Shapes::Cube::Pallete[(index / 2) % Shapes::Cube::PalleteSize];
				fragment.material = Material;
			}
		};

		struct OctahedronMeshObject : public MeshSingleColorSingleMaterialObject<Shapes::Octahedron::VertexCount, Shapes::Octahedron::TriangleCount>
		{
			OctahedronMeshObject() : MeshSingleColorSingleMaterialObject<Shapes::Octahedron::VertexCount, Shapes::Octahedron::TriangleCount>(
				Shapes::Octahedron::Vertices,
				Shapes::Octahedron::Triangles,
				Shapes::Octahedron::Normals) {
			}

		protected:
			virtual void GetFragment(triangle_fragment_t& fragment, const uint16_t index)
			{
				fragment.color = Color;
				fragment.material = Material;
				fragment.color.r >>= index >> 1;
				fragment.color.g >>= index >> 1;
				fragment.color.b >>= index >> 1;
			}
		};
	}
}
#endif