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
			IntegerWorld::PrimitiveSources::Material::DiffuseMaterialSource,
			IntegerWorld::PrimitiveSources::Normal::Static::FixedSource<0, 0, 0>
			>
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
				IntegerWorld::PrimitiveSources::Material::DiffuseMaterialSource,
				IntegerWorld::PrimitiveSources::Normal::Static::FixedSource<0, 0, 0>
			>;

			IntegerWorld::PrimitiveSources::Vertex::Static::Source VertexSource;
			IntegerWorld::PrimitiveSources::Triangle::Static::Source TriangleSource;
			IntegerWorld::PrimitiveSources::Albedo::Static::Source AlbedoSource;

		public:
			CubeMeshObject()
				: VertexSource(Shapes::Cube::Vertices)
				, TriangleSource(Shapes::Cube::Triangles)
				, AlbedoSource(Palletes::Cube::Albedos)
				, Base(VertexSource, TriangleSource, AlbedoSource,
					const_cast<IntegerWorld::PrimitiveSources::Material::DiffuseMaterialSource&>(PrimitiveSources::Material::DiffuseMaterialSourceInstance),
					MockNormalSource)
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
			IntegerWorld::PrimitiveSources::Material::DiffuseMaterialSource,
			IntegerWorld::PrimitiveSources::Normal::Static::FixedSource<0, 0, 0>>
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
				IntegerWorld::PrimitiveSources::Material::DiffuseMaterialSource,
				IntegerWorld::PrimitiveSources::Normal::Static::FixedSource<0, 0, 0>>;

			IntegerWorld::PrimitiveSources::Vertex::Static::Source VertexSource;
			IntegerWorld::PrimitiveSources::Triangle::Static::Source TriangleSource;
			IntegerWorld::PrimitiveSources::Albedo::Dynamic::SingleSource AlbedoSource;

		public:
			OctahedronMeshObject()
				: VertexSource(Shapes::Octahedron::Vertices)
				, TriangleSource(Shapes::Octahedron::Triangles)
				, AlbedoSource()
				, Base(VertexSource, TriangleSource, AlbedoSource,
					const_cast<IntegerWorld::PrimitiveSources::Material::DiffuseMaterialSource&>(PrimitiveSources::Material::DiffuseMaterialSourceInstance),
					MockNormalSource)
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
		/// Shades an object's albedo by reducing each RGB component based on the vertex height (y coordinate).
		/// Assumes shape is centered at y=0, with shading range from around -SHAPE_UNIT/2 to +SHAPE_UNIT/2.
		/// </summary>
		class HeightSceneShader final : public ISceneShader
		{
		private:
			// Hue aware, brightness reduction based on height.
			static uint8_t ScaleColor(const uint8_t component, const uint8_t remove)
			{
				const uint8_t removeComponent = (uint16_t(remove) * component) >> 7;
				return component > removeComponent ? component - removeComponent : 0;
			}

		public:
			Rgb8::color_t GetLitColor(const Rgb8::color_t albedo, const material_t& material, const vertex16_t& position, const vertex16_t& normal)
			{
				const uint8_t yDelta = LimitValue<int16_t, 0, (Shapes::SHAPE_UNIT * 4) / 5>((Shapes::SHAPE_UNIT / 2) - position.y) >> 4;
				return Rgb8::Color(
					ScaleColor(Rgb8::Red(albedo), yDelta),
					ScaleColor(Rgb8::Green(albedo), yDelta),
					ScaleColor(Rgb8::Blue(albedo), yDelta));
			}
		};
	}
}
#endif