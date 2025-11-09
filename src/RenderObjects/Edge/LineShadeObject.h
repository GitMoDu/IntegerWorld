#ifndef _INTEGER_WORLD_RENDER_OBJECTS_EDGE_LINE_SHADE_OBJECT_h
#define _INTEGER_WORLD_RENDER_OBJECTS_EDGE_LINE_SHADE_OBJECT_h

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
				FaceCullingEnum faceCulling = FaceCullingEnum::NoCulling,
				typename AlbedoSourceType = PrimitiveSources::Albedo::Static::FullSource,
				typename MaterialSourceType = PrimitiveSources::Material::DiffuseMaterialSource,
				typename NormalSourceType = PrimitiveSources::Normal::Static::NoSource>
			class LineShadeObject : public AbstractObject<
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
				using BaseClass = AbstractObject<
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
				using BaseClass::Vertices;
				using BaseClass::Primitives;
				using BaseClass::EdgeSource;
				using BaseClass::EdgeCount;
				using BaseClass::Average;
				using BaseClass::AlbedoSource;
				using BaseClass::MaterialSource;
				using BaseClass::NormalSource;
				using BaseClass::MeshTransform;
				using BaseClass::WorldPosition;

			public:
				using fragment_t = mesh_triangle_fragment_t;

				/// <summary>
				/// Edge fragment shader. If null, no fragments are shaded.
				/// </summary>
				IFragmentShader<edge_line_fragment_t>* FragmentShader = nullptr;

				/// <summary>
				/// Scene's lighting shader. If null, no lighting is applied.
				/// </summary>
				ISceneShader* SceneShader = nullptr;

			protected:
				// Per-triangle lit albedo colors.
				CompactRgb8List<edgeCount> LightBuffer{};

			private:
				// Reusable edge fragment.
				edge_line_fragment_t Fragment{};

			public:
				LineShadeObject(VertexSourceType& vertexSource,
					EdgeSourceType& edgeSource,
					AlbedoSourceType& albedoSource = const_cast<AlbedoSourceType&>(PrimitiveSources::Albedo::FullAlbedoSourceInstance),
					MaterialSourceType& materialSource = const_cast<MaterialSourceType&>(PrimitiveSources::Material::DiffuseMaterialSourceInstance),
					NormalSourceType& normalSource = const_cast<NormalSourceType&>(PrimitiveSources::Normal::NormalNoSourceInstance)
				)
					: BaseClass(vertexSource, edgeSource, albedoSource, materialSource, normalSource)
				{
				}

				/// <summary>
				/// World pass:
				/// - Computes per-primitive world position (edge line center).
				/// - Optionally culls primitives against the frustum.
				/// - Computes or rotates the edge world normal.
				/// - Applies the scene shader to light the primitive's albedo color.
				/// </summary>
				/// <returns>false to continue processing; true when no more primitives are available.</returns>
				virtual bool WorldShade(const frustum_t& frustum, const uint16_t primitiveIndex)
				{
					if (primitiveIndex >= EdgeCount)
						return true;

					if (Primitives[primitiveIndex] < 0)
						return false;

					const auto edge = EdgeSource.GetEdge(primitiveIndex);

					const vertex16_t worldPosition{
						Average(Vertices[edge.a].x, Vertices[edge.b].x),
						Average(Vertices[edge.a].y, Vertices[edge.b].y),
						Average(Vertices[edge.a].z, Vertices[edge.b].z)
					};

					switch (frustumCulling)
					{
					case FrustumCullingEnum::PrimitiveCulling:
						if (!frustum.IsPointInside(Vertices[edge.a])
							|| !frustum.IsPointInside(Vertices[edge.b]))
						{
							Primitives[primitiveIndex] = -VERTEX16_UNIT;
							return false;
						}
						break;
					default:
						break;
					};

					Rgb8::color_t primitiveColor = Rgb8::WHITE;
					if (AlbedoSourceType::HasAlbedos())
					{
						primitiveColor = AlbedoSource.GetAlbedo(primitiveIndex);
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
							primitiveMaterial = { UFRACTION8_1X, 0, 0, 0 };
						}

						vertex16_t worldNormal;
						if (NormalSourceType::HasNormals())
						{
							// Rotate precomputed normal.
							worldNormal = NormalSource.GetNormal(primitiveIndex);
							ApplyTransformRotation(MeshTransform, worldNormal);
						}
						else
						{
							// Calculate edge normal from the average of the two vertices in world space,
							// relative to the object center position.
							auto vertexA = Vertices[edge.a];
							auto vertexB = Vertices[edge.b];
							worldNormal = {
								static_cast<int16_t>(Average(vertexA.x, vertexB.x) - WorldPosition.x),
								static_cast<int16_t>(Average(vertexA.y, vertexB.y) - WorldPosition.y),
								static_cast<int16_t>(Average(vertexA.z, vertexB.z) - WorldPosition.z) };
							NormalizeVertex16(worldNormal);
						}

						// Apply scene shader to get lit color.
						primitiveColor = SceneShader->GetLitColor(primitiveColor, primitiveMaterial, worldPosition, worldNormal);
					}

					// Cache the modulated color for the fragment shader.
					LightBuffer.SetColor(primitiveColor, primitiveIndex);

					return false;
				}

				/// <summary>
				/// Produces a edge fragment for the rasterizer and calls the fragment shader.
				/// </summary>
				/// <param name="rasterizer">Window rasterizer to receive the shaded edge.</param>
				/// <param name="primitiveIndex">Edge index.</param>
				virtual void FragmentShade(WindowRasterizer& rasterizer, const uint16_t primitiveIndex)
				{
					if (FragmentShader == nullptr)
					{
						return;
					}
					Fragment.index = primitiveIndex;
					Fragment.z = Primitives[primitiveIndex];
					{
						const auto edge = EdgeSource.GetEdge(primitiveIndex);
						Fragment.vertexA = Vertices[edge.a];
						Fragment.vertexB = Vertices[edge.b];
					}
					{
						const auto color = LightBuffer.GetColor(primitiveIndex);
						Fragment.red = Rgb8::Red(color);
						Fragment.green = Rgb8::Green(color);
						Fragment.blue = Rgb8::Blue(color);
					}
					FragmentShader->FragmentShade(rasterizer, Fragment);
				}
			};

			template<uint16_t vertexCount,
				uint16_t edgeCount,
				FrustumCullingEnum frustumCulling = FrustumCullingEnum::PrimitiveCulling,
				FaceCullingEnum faceCulling = FaceCullingEnum::NoCulling>
			class SimpleStaticEdgeLineObject : public LineShadeObject<
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
				using Base = LineShadeObject<
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
				SimpleStaticEdgeLineObject(const vertex16_t* vertices, const edge_line_t* edges)
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