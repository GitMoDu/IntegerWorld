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
				Shapes::Cube::Triangles
			) {
			}

		protected:
			void GetFragment(triangle_fragment_t& fragment, const uint16_t index) final
			{
				fragment.color = Shapes::Cube::Pallete[(index / 2) % Shapes::Cube::PalleteSize];
				fragment.material = Material;
			}
		};

		struct OctahedronMeshObject : public MeshObject<Shapes::Octahedron::VertexCount, Shapes::Octahedron::TriangleCount>
		{
			OctahedronMeshObject() : MeshObject<Shapes::Octahedron::VertexCount, Shapes::Octahedron::TriangleCount>(
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

				const fraction16_t fraction = Fraction::GetFraction16(yTravel, yRange);

				fragment.color.r = fraction16_t(UFRACTION16_1X / 2) - fraction;
				fragment.color.g = fragment.color.r;
				fragment.color.b = fragment.color.r;

				fragment.material = { UFRACTION8_1X, 0, 0, 0 };
			}
		};
	}
}
#endif