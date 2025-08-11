#ifndef _TINY_ASSETS_h
#define _TINY_ASSETS_h

#include <IntegerWorld.h>

namespace Assets
{
	using namespace IntegerWorld;

	namespace Palletes
	{
		namespace Cube
		{
			static constexpr Rgb8::color_t Pallete[Shapes::Cube::TriangleCount]
			{
				Rgb8::RED,
				Rgb8::GREEN,
				Rgb8::BLUE,
				0xFFFF00,
				0x00FFFF,
				0xFF00FF
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

		struct CubeMeshObject : public MeshWorldObject<Shapes::Cube::VertexCount, Shapes::Cube::TriangleCount>
		{
			material_t Material{ 0, UFRACTION8_1X, 0, 0 };

			CubeMeshObject() : MeshWorldObject<Shapes::Cube::VertexCount, Shapes::Cube::TriangleCount>(
				Shapes::Cube::Vertices,
				Shapes::Cube::Triangles
			) {
			}

		protected:
			void GetFragment(triangle_fragment_t& fragment, const uint16_t index) final
			{
				fragment.color = Palletes::Cube::Pallete[(index >> 1) % Palletes::Cube::PalleteSize];
				fragment.material = Material;
			}
		};

		struct OctahedronMeshObject : public MeshWorldObject<Shapes::Octahedron::VertexCount, Shapes::Octahedron::TriangleCount>
		{
			OctahedronMeshObject() : MeshWorldObject<Shapes::Octahedron::VertexCount, Shapes::Octahedron::TriangleCount>(
				Shapes::Octahedron::Vertices,
				Shapes::Octahedron::Triangles) {
			}
		};


		template<int16_t yRange, typename BaseMeshObject>
		class TemplateFakeYShadeMeshObject : public BaseMeshObject
		{
		public:
			TemplateFakeYShadeMeshObject()
				: BaseMeshObject()
			{
			}

		protected:
			void GetFragment(triangle_fragment_t& fragment, const uint16_t index) final
			{
				const int16_t y = AverageApproximate(fragment.triangleScreenA.y, fragment.triangleScreenB.y, fragment.triangleScreenC.y);
				const int16_t yTravel = LimitValue(y - BaseMeshObject::ObjectPosition.y, -yRange, yRange);

				uint8_t gray = INT8_MAX;
				gray += (-yTravel * INT8_MAX) / yRange;
				fragment.color = Rgb8::Color(gray, gray, gray);

				fragment.material = { UFRACTION8_1X, 0, 0, 0 };
			}
		};
	}
}
#endif