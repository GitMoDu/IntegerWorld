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
		struct CubeEdgeObject : StaticEdgeSingleColorSingleMaterialObject<Shapes::Cube::VertexCount, Shapes::Cube::EdgeCount>
		{
			CubeEdgeObject()
				: StaticEdgeSingleColorSingleMaterialObject<Shapes::Cube::VertexCount, Shapes::Cube::EdgeCount>(
					Shapes::Cube::Vertices,
					Shapes::Cube::Edges) {
			}
		};

		struct OctahedronEdgeObject : StaticEdgeSingleColorSingleMaterialObject<Shapes::Octahedron::VertexCount, Shapes::Octahedron::EdgeCount>
		{
			OctahedronEdgeObject()
				: StaticEdgeSingleColorSingleMaterialObject<Shapes::Octahedron::VertexCount, Shapes::Octahedron::EdgeCount>
				(Shapes::Octahedron::Vertices, Shapes::Octahedron::Edges)
			{
			}
		};

		struct CubeMeshObject : StaticMeshObject<Shapes::Cube::VertexCount, Shapes::Cube::TriangleCount>
		{
			material_t Material{ 0, UFRACTION8_1X, 0, 0 };

			CubeMeshObject() : StaticMeshObject<Shapes::Cube::VertexCount, Shapes::Cube::TriangleCount>
				(Shapes::Cube::Vertices, Shapes::Cube::Triangles)
			{
			}

		protected:
			void GetFragment(triangle_fragment_t& fragment, const uint16_t index) final
			{
				fragment.color = Palletes::Cube::Pallete[(index >> 1) % Palletes::Cube::PalleteSize];
				fragment.material = Material;
			}
		};

		struct OctahedronMeshObject : StaticMeshSingleColorSingleMaterialObject<Shapes::Octahedron::VertexCount, Shapes::Octahedron::TriangleCount>
		{
			OctahedronMeshObject() : StaticMeshSingleColorSingleMaterialObject<Shapes::Octahedron::VertexCount, Shapes::Octahedron::TriangleCount>(
				Shapes::Octahedron::Vertices,
				Shapes::Octahedron::Triangles) {
			}
		};

		template<int16_t Range, typename BaseMeshObject>
		class TemplateOffsetShadeMeshObject : public BaseMeshObject
		{
		public:
			TemplateOffsetShadeMeshObject() : BaseMeshObject() {}

		public:
			using BaseMeshObject::WorldPosition;

		private:
			// Screen space object position.
			vertex16_t ScreenPosition{};

		public:
			virtual bool CameraTransform(const transform16_camera_t& transform, const uint16_t vertexIndex)
			{
				if (vertexIndex == 0)
				{
					ScreenPosition = WorldPosition;
					ApplyCameraTransform(transform, ScreenPosition);
				}

				return BaseMeshObject::CameraTransform(transform, vertexIndex);
			}

			virtual bool ScreenProject(ViewportProjector& screenProjector, const uint16_t vertexIndex)
			{
				if (vertexIndex == 0)
					screenProjector.Project(ScreenPosition);

				return BaseMeshObject::ScreenProject(screenProjector, vertexIndex);
			}

		protected:
			void GetFragment(triangle_fragment_t& fragment, const uint16_t index) final
			{
				const int16_t x = AverageApproximate(fragment.triangleScreenA.x, fragment.triangleScreenB.x, fragment.triangleScreenC.x);
				const int16_t y = AverageApproximate(fragment.triangleScreenA.y, fragment.triangleScreenB.y, fragment.triangleScreenC.y);
				const int16_t z = AverageApproximate(fragment.triangleScreenA.z, fragment.triangleScreenB.z, fragment.triangleScreenC.z);
				const int16_t xTravel = LimitValue<int16_t>(x - ScreenPosition.x, -Range, Range);
				const int16_t yTravel = LimitValue<int16_t>(y - ScreenPosition.y, -Range, Range);
				const int16_t zTravel = LimitValue<int16_t>(z - ScreenPosition.z, -Range, Range);

				uint8_t gray = INT8_MAX;
				gray += (-int32_t(xTravel) * (32)) / Range;
				gray += (-int32_t(yTravel) * (90)) / Range;
				gray += (-int32_t(zTravel) * (16)) / Range;
				fragment.color = Rgb8::Color(gray, gray, gray);
				fragment.material = { UFRACTION8_1X, 0, 0, 0 };
			}
		};
	}
}
#endif