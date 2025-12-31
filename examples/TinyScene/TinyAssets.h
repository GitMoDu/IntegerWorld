#ifndef _TINY_ASSETS_h
#define _TINY_ASSETS_h

#include <IntegerWorld.h>

namespace Assets
{
	namespace Palletes
	{
		namespace Cube
		{
			static constexpr Rgb8::color_t Albedos[] PROGMEM
			{
				0xFF0000,
				0xFF0000,
				0x00FF00,
				0x00FF00,
				0x0000FF,
				0x0000FF,
				0xFFFF00,
				0xFFFF00,
				0x00FFFF,
				0x00FFFF,
				0xFF00FF,
				0xFF00FF
			};
		}
	}

	namespace RenderObjects
	{
		using namespace Shapes;
		using namespace IntegerWorld::RenderObjects;

		struct CubeEdgeObject : Edge::SimpleStaticEdgeLineObject<Shapes::Cube::VertexCount, Shapes::Cube::EdgeCount>
		{
			CubeEdgeObject()
				: Edge::SimpleStaticEdgeLineObject<Shapes::Cube::VertexCount, Shapes::Cube::EdgeCount>(
					Shapes::Cube::Vertices,
					Shapes::Cube::Edges) {
			}
		};

		struct OctahedronEdgeObject : IntegerWorld::RenderObjects::Edge::SimpleStaticEdgeLineObject<Shapes::Octahedron::VertexCount, Shapes::Octahedron::EdgeCount>
		{
			OctahedronEdgeObject()
				: IntegerWorld::RenderObjects::Edge::SimpleStaticEdgeLineObject<Shapes::Octahedron::VertexCount, Shapes::Octahedron::EdgeCount>
				(Shapes::Octahedron::Vertices, Shapes::Octahedron::Edges)
			{
			}
		};

		IntegerWorld::PrimitiveSources::Normal::Static::FixedSource<0, 0, 0> MockNormalSource{};

		/// <summary>
		/// Mesh triangle object for a cube.
		/// Minimal memory usage by using static sources.
		/// Minimal processing, no frustum culling is applied and skips calculating normal.
		/// Fixed albedo colors from palette.
		/// </summary>
		class CubeMeshObject
			: public IntegerWorld::RenderObjects::Mesh::TriangleShadeObject<
			Shapes::Cube::VertexCount,
			Shapes::Cube::TriangleCount,
			IntegerWorld::PrimitiveSources::Vertex::Static::Source,
			IntegerWorld::PrimitiveSources::Triangle::Static::Source,
			FrustumCullingEnum::NoCulling,
			FaceCullingEnum::BackfaceCulling,
			IntegerWorld::PrimitiveSources::Albedo::Static::Source,
			IntegerWorld::PrimitiveSources::Material::DiffuseMaterialSource>
		{
		private:
			using Base = IntegerWorld::RenderObjects::Mesh::TriangleShadeObject<
				Shapes::Cube::VertexCount,
				Shapes::Cube::TriangleCount,
				IntegerWorld::PrimitiveSources::Vertex::Static::Source,
				IntegerWorld::PrimitiveSources::Triangle::Static::Source,
				FrustumCullingEnum::NoCulling,
				FaceCullingEnum::BackfaceCulling,
				IntegerWorld::PrimitiveSources::Albedo::Static::Source,
				IntegerWorld::PrimitiveSources::Material::DiffuseMaterialSource>;

			IntegerWorld::PrimitiveSources::Vertex::Static::Source VertexSource;
			IntegerWorld::PrimitiveSources::Triangle::Static::Source TriangleSource;
			IntegerWorld::PrimitiveSources::Albedo::Static::Source AlbedoSource;

		public:
			CubeMeshObject()
				: Base(VertexSource, TriangleSource, AlbedoSource,
					const_cast<IntegerWorld::PrimitiveSources::Material::DiffuseMaterialSource&>(PrimitiveSources::Material::DiffuseMaterialSourceInstance))
				, VertexSource(Shapes::Cube::Vertices)
				, TriangleSource(Shapes::Cube::Triangles)
				, AlbedoSource(Palletes::Cube::Albedos)
			{
			}
		};

		/// <summary>
		/// Mesh triangle object for a octaedron.
		/// Minimal memory usage by using static sources.
		/// Minimal processing, no frustum culling is applied and skips calculating normal.
		/// Single dynamic albedo color.
		/// </summary>
		class OctahedronMeshObject
			: public IntegerWorld::RenderObjects::Mesh::TriangleShadeObject<
			Shapes::Octahedron::VertexCount,
			Shapes::Octahedron::TriangleCount,
			IntegerWorld::PrimitiveSources::Vertex::Static::Source,
			IntegerWorld::PrimitiveSources::Triangle::Static::Source,
			FrustumCullingEnum::NoCulling,
			FaceCullingEnum::BackfaceCulling,
			IntegerWorld::PrimitiveSources::Albedo::Dynamic::SingleSource,
			IntegerWorld::PrimitiveSources::Material::DiffuseMaterialSource>
		{
		private:
			using Base = IntegerWorld::RenderObjects::Mesh::TriangleShadeObject<
				Shapes::Octahedron::VertexCount,
				Shapes::Octahedron::TriangleCount,
				IntegerWorld::PrimitiveSources::Vertex::Static::Source,
				IntegerWorld::PrimitiveSources::Triangle::Static::Source,
				FrustumCullingEnum::NoCulling,
				FaceCullingEnum::BackfaceCulling,
				IntegerWorld::PrimitiveSources::Albedo::Dynamic::SingleSource,
				IntegerWorld::PrimitiveSources::Material::DiffuseMaterialSource>;

			IntegerWorld::PrimitiveSources::Vertex::Static::Source VertexSource;
			IntegerWorld::PrimitiveSources::Triangle::Static::Source TriangleSource;

			IntegerWorld::PrimitiveSources::Albedo::Dynamic::SingleSource AlbedoSource{};

		public:
			OctahedronMeshObject()
				: Base(VertexSource, TriangleSource, AlbedoSource,
					const_cast<IntegerWorld::PrimitiveSources::Material::DiffuseMaterialSource&>(PrimitiveSources::Material::DiffuseMaterialSourceInstance))
				, VertexSource(Shapes::Octahedron::Vertices)
				, TriangleSource(Shapes::Octahedron::Triangles)
			{
			}

			void SetAlbedo(const Rgb8::color_t albedo)
			{
				AlbedoSource.Albedo = albedo;
			}

			Rgb8::color_t GetAlbedo() const
			{
				return AlbedoSource.Albedo;
			}
		};

		struct OctahedronSimpleMeshObject : IntegerWorld::RenderObjects::Mesh::SimpleStaticMeshTriangleObject<
			Shapes::Octahedron::VertexCount,
			Shapes::Octahedron::TriangleCount,
			FrustumCullingEnum::NoCulling,
			FaceCullingEnum::BackfaceCulling>
		{
			OctahedronSimpleMeshObject() : IntegerWorld::RenderObjects::Mesh::SimpleStaticMeshTriangleObject<
				Shapes::Octahedron::VertexCount,
				Shapes::Octahedron::TriangleCount,
				FrustumCullingEnum::NoCulling,
				FaceCullingEnum::BackfaceCulling>(
					Shapes::Octahedron::Vertices,
					Shapes::Octahedron::Triangles) {
			}
		};
	}

	namespace SceneShaders
	{
		/// <summary>
		/// Applies a global lighting shade, brightening colors based on normal direction.
		/// </summary>
		class GlobalLightSceneShader final : public ISceneShader
		{
		private:
			static uint8_t MixColor(const uint8_t component, const int8_t gain)
			{
				const uint8_t scaled = int16_t(component >> 1) + SignedRightShift(static_cast<int16_t>(component) * gain, 8);

				if (gain > 0)
					return (UINT8_MAX - scaled) > gain ? scaled + gain : UINT8_MAX;
				else
					return scaled;
			}

		public:
			GlobalLightSceneShader() : ISceneShader() {}

			Rgb8::color_t GetLitColor(const Rgb8::color_t albedo, const material_t& /*material*/, const vertex16_t& /*position*/, const vertex16_t& normal)
			{
				// Calculate normal gain based on normal x and y components (light is from top-left corner).
				const int8_t normalGain = SignedRightShift(normal.y, 7) + SignedRightShift(normal.x, 7);
				return Rgb8::Color(MixColor(Rgb8::Red(albedo), normalGain),
					MixColor(Rgb8::Green(albedo), normalGain),
					MixColor(Rgb8::Blue(albedo), normalGain));
			}
		};
	}
}
#endif