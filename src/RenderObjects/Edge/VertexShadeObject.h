#ifndef _INTEGER_WORLD_RENDER_OBJECTS_EDGE_VERTEX_SHADE_OBJECT_h
#define _INTEGER_WORLD_RENDER_OBJECTS_EDGE_VERTEX_SHADE_OBJECT_h

#include "AbstractObject.h"

namespace IntegerWorld
{
	namespace RenderObjects
	{
		namespace Edge
		{
			template<uint16_t vertexCount,
				uint16_t edgeCount,
				typename VertexSourceType,
				typename EdgeSourceType,
				FrustumCullingEnum frustumCulling = FrustumCullingEnum::PrimitiveCulling,
				FaceCullingEnum faceCulling = FaceCullingEnum::BackfaceCulling,
				typename AlbedoSourceType = PrimitiveSources::Albedo::Static::FullSource,
				typename MaterialSourceType = PrimitiveSources::Material::DiffuseMaterialSource,
				typename NormalSourceType = PrimitiveSources::Normal::Static::NoSource>
			class VertexShadeObject : public AbstractObject<
				vertexCount,
				edgeCount,
				VertexSourceType,
				EdgeSourceType,
				frustumCulling,
				faceCulling,
				AlbedoSourceType,
				MaterialSourceType,
				NormalSourceType>
			{
			private:
				using Base = AbstractObject<
					vertexCount,
					edgeCount,
					VertexSourceType,
					EdgeSourceType,
					frustumCulling,
					faceCulling,
					AlbedoSourceType,
					MaterialSourceType,
					NormalSourceType>;

			protected:
				using Base::Vertices;
				using Base::Primitives;
				using Base::EdgeSource;
				using Base::EdgeCount;
				using Base::VertexCount;
				using Base::AlbedoSource;
				using Base::MaterialSource;
				using Base::NormalSource;
				using Base::VertexSource;
				using Base::MeshTransform;
				using Base::Average;
				using Base::WorldPosition;

			public:
				/// <summary>
				/// Edge vertex fragment shader. If null, no fragments are shaded.
				/// </summary>
				IFragmentShader<edge_vertex_fragment_t>* FragmentShader = nullptr;

				/// <summary>
				/// Scene's lighting shader. If null, no lighting is applied.
				/// </summary>
				ISceneShader* SceneShader = nullptr;

			private:
				// Per-vertex lit albedo colors.
				CompactRgb8List<vertexCount> LightBuffer{};

				// Reusable edge fragment.
				edge_vertex_fragment_t Fragment{};

			public:
				VertexShadeObject(VertexSourceType& vertexSource,
					EdgeSourceType& edgeSource,
					AlbedoSourceType& albedoSource = const_cast<AlbedoSourceType&>(PrimitiveSources::Albedo::FullAlbedoSourceInstance),
					MaterialSourceType& materialSource = const_cast<MaterialSourceType&>(PrimitiveSources::Material::DiffuseMaterialSourceInstance),
					NormalSourceType& normalSource = const_cast<NormalSourceType&>(PrimitiveSources::Normal::NormalNoSourceInstance))
					: Base(vertexSource, edgeSource, albedoSource, materialSource, normalSource)
				{
				}

				/// <summary>
				/// World pass:
				/// - Computes per-primitive world position (edge midpoint).
				/// - Optionally culls primitives against the frustum.
				/// - Computes or rotates the vertex world normal.
				/// - Applies the scene shader to light the primitive's albedo color.
				/// </summary>
				/// <returns>false to continue processing; true when no more primitives are available.</returns>
				virtual bool WorldShade(const frustum_t& frustum, const uint16_t primitiveIndex)
				{
					if (primitiveIndex < EdgeCount
						&&
						(frustumCulling == FrustumCullingEnum::NoCulling || (Primitives[primitiveIndex] >= 0)))
					{
						switch (frustumCulling)
						{
						case FrustumCullingEnum::PrimitiveCulling:
						{
							const auto edge = EdgeSource.GetEdge(primitiveIndex);

							// If the edge is outside the frustum, cull it.
							if (!frustum.IsPointInside(Vertices[edge.a])
								|| !frustum.IsPointInside(Vertices[edge.b]))
							{
								Primitives[primitiveIndex] = -VERTEX16_UNIT;
							}
						}
						break;
						case FrustumCullingEnum::NoCulling:
						default:
							break;
						};
					}

					if (primitiveIndex < VertexCount)
					{
						Rgb8::color_t primitiveColor;
						if (AlbedoSourceType::HasAlbedos())
						{
							primitiveColor = AlbedoSource.GetAlbedo(primitiveIndex);
						}
						else
						{
							primitiveColor = Rgb8::WHITE;
						}

						if (SceneShader != nullptr)
						{
							material_t primitiveMaterial;
							if (MaterialSourceType::HasMaterials())
							{
								primitiveMaterial = MaterialSource.GetMaterial(primitiveIndex);
							}
							else
							{
								primitiveMaterial = { 0, UFRACTION8_1X, 0, 0, 0, 0 };
							}

							vertex16_t worldNormal;
							if (NormalSourceType::HasNormals())
							{
								// Rotate precomputed vertex normal.
								worldNormal = NormalSource.GetNormal(primitiveIndex);
								ApplyTransformRotation(MeshTransform, worldNormal);
							}
							else
							{
								// Calculate edge normal from the vertex position in object space.
								auto vertex = Vertices[primitiveIndex];
								worldNormal = { vertex.x - WorldPosition.x,
									vertex.y - WorldPosition.y,
									vertex.z - WorldPosition.z };
								NormalizeVertex16(worldNormal);
							}

							// Apply scene shader to get lit color.
							primitiveColor = SceneShader->GetLitColor(primitiveColor, primitiveMaterial, Vertices[primitiveIndex], worldNormal);
						}

						// Cache the lit albedo color.
						LightBuffer.SetColor(primitiveColor, primitiveIndex);
					}

					return primitiveIndex >= MaxValue(EdgeCount, VertexCount);
				}

				/// <summary>
				/// Produces a vertex-lit edge fragment for the rasterizer and calls the fragment shader.
				/// </summary>
				/// <param name="rasterizer">Window rasterizer to receive the shaded edge.</param>
				/// <param name="primitiveIndex">Edge index.</param>
				virtual void FragmentShade(WindowRasterizer& rasterizer, const uint16_t primitiveIndex)
				{
					if (FragmentShader == nullptr)
						return;

					if (primitiveIndex >= EdgeCount)
						return;

					// Skip culled edges
					if (Primitives[primitiveIndex] < 0)
						return;

					const auto edge = EdgeSource.GetEdge(primitiveIndex);

					Fragment.index = primitiveIndex;
					Fragment.z = Primitives[primitiveIndex];
					Fragment.vertexA = Vertices[edge.a];
					Fragment.vertexB = Vertices[edge.b];

					{
						auto color = LightBuffer.GetColor(edge.a);
						Fragment.redA = Rgb8::Red(color);
						Fragment.greenA = Rgb8::Green(color);
						Fragment.blueA = Rgb8::Blue(color);

						color = LightBuffer.GetColor(edge.b);
						Fragment.redB = Rgb8::Red(color);
						Fragment.greenB = Rgb8::Green(color);
						Fragment.blueB = Rgb8::Blue(color);
					}

					FragmentShader->FragmentShade(rasterizer, Fragment);
				}
			};

			template<uint16_t vertexCount,
				uint16_t edgeCount,
				FrustumCullingEnum frustumCulling = FrustumCullingEnum::PrimitiveCulling,
				FaceCullingEnum faceCulling = FaceCullingEnum::NoCulling>
			class SimpleStaticEdgeVertexObject : public VertexShadeObject<
				vertexCount,
				edgeCount,
				PrimitiveSources::Vertex::Static::Source,
				PrimitiveSources::Edge::Static::Source,
				frustumCulling,
				faceCulling,
				PrimitiveSources::Albedo::Dynamic::SingleSource,
				PrimitiveSources::Material::Dynamic::SingleSource>
			{
			private:
				using Base = VertexShadeObject<
					vertexCount,
					edgeCount,
					PrimitiveSources::Vertex::Static::Source,
					PrimitiveSources::Edge::Static::Source,
					frustumCulling,
					faceCulling,
					PrimitiveSources::Albedo::Dynamic::SingleSource,
					PrimitiveSources::Material::Dynamic::SingleSource>;

			private:
				PrimitiveSources::Vertex::Static::Source VerticesSource;
				PrimitiveSources::Edge::Static::Source EdgesSource;

				PrimitiveSources::Albedo::Dynamic::SingleSource AlbedosSource{};
				PrimitiveSources::Material::Dynamic::SingleSource MaterialsSource{};

			public:
				SimpleStaticEdgeVertexObject(const vertex16_t* vertices,
					const edge_line_t* edges)
					: VerticesSource(vertices)
					, EdgesSource(edges)
					, Base(VerticesSource, EdgesSource, AlbedosSource, MaterialsSource)
				{
				}

				void SetAlbedo(const Rgb8::color_t albedo)
				{
					AlbedosSource.Albedo = albedo;
				}

				Rgb8::color_t GetAlbedo() const
				{
					return AlbedosSource.Albedo;
				}

				void SetMaterial(const material_t& material)
				{
					MaterialsSource.Material = material;
				}

				material_t GetMaterial() const
				{
					return MaterialsSource.Material;
				}
			};
		}
	}
}
#endif