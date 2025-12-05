#ifndef _INTEGER_WORLD_RENDER_OBJECTS_MESH_VERTEX_SHADE_OBJECT_h
#define _INTEGER_WORLD_RENDER_OBJECTS_MESH_VERTEX_SHADE_OBJECT_h

#include "AbstractObject.h"

namespace IntegerWorld
{
	namespace RenderObjects
	{
		namespace Mesh
		{
			template<uint16_t vertexCount,
				uint16_t triangleCount,
				typename VertexSourceType,
				typename TriangleSourceType,
				FrustumCullingEnum frustumCulling = FrustumCullingEnum::PrimitiveCulling,
				FaceCullingEnum faceCulling = FaceCullingEnum::BackfaceCulling,
				typename AlbedoSourceType = PrimitiveSources::Albedo::Static::FullSource,
				typename MaterialSourceType = PrimitiveSources::Material::DiffuseMaterialSource,
				typename NormalSourceType = PrimitiveSources::Normal::Static::NoSource,
				typename UvSourceType = PrimitiveSources::Uv::Static::NoSource>
			class VertexShadeObject : public AbstractObject<
				vertexCount,
				triangleCount,
				VertexSourceType,
				TriangleSourceType,
				frustumCulling,
				faceCulling,
				AlbedoSourceType,
				MaterialSourceType,
				NormalSourceType,
				UvSourceType>
			{
			private:
				using Base = AbstractObject<
					vertexCount,
					triangleCount,
					VertexSourceType,
					TriangleSourceType,
					frustumCulling,
					faceCulling,
					AlbedoSourceType,
					MaterialSourceType,
					NormalSourceType,
					UvSourceType>;

			protected:
				using Base::Vertices;
				using Base::Primitives;
				using Base::TriangleSource;
				using Base::TriangleCount;
				using Base::VertexCount;
				using Base::MeshTransform;
				using Base::VertexSource;
				using Base::AlbedoSource;
				using Base::MaterialSource;
				using Base::NormalSource;
				using Base::UvSource;

			public:
				/// <summary>
				/// Mesh triangle fragment shader. If null, no fragments are shaded.
				/// </summary>
				IFragmentShader<mesh_vertex_fragment_t>* FragmentShader = nullptr;

				/// <summary>
				/// Scene's lighting shader. If null, no lighting is applied.
				/// </summary>
				ISceneShader* SceneShader = nullptr;

			private:
				// Per-triangle lit albedo colors.
				CompactRgb8List<vertexCount> LightBuffer{};

				// Reusable triangle fragment.
				mesh_vertex_fragment_t Fragment{};

			public:
				VertexShadeObject(VertexSourceType& vertexSource,
					TriangleSourceType& triangleSource,
					AlbedoSourceType& albedoSource = const_cast<AlbedoSourceType&>(PrimitiveSources::Albedo::FullAlbedoSourceInstance),
					MaterialSourceType& materialSource = const_cast<MaterialSourceType&>(PrimitiveSources::Material::DiffuseMaterialSourceInstance),
					NormalSourceType& normalSource = const_cast<NormalSourceType&>(PrimitiveSources::Normal::NormalNoSourceInstance),
					UvSourceType& uvSource = const_cast<UvSourceType&>(PrimitiveSources::Uv::NoUvSourceInstance))
					: Base(vertexSource, triangleSource, albedoSource, materialSource, normalSource, uvSource)
				{
				}

				/// <summary>
				/// World pass:
				/// - Computes per-primitive world position (vertices).
				/// - Optionally culls primitives against the frustum.
				/// - Computes or rotates the vertex world normal.
				/// - Applies the scene shader to light the primitive's albedo color.
				/// </summary>
				/// <returns>false to continue processing; true when no more primitives are available.</returns>
				virtual bool WorldShade(const frustum_t& frustum, const uint16_t primitiveIndex)
				{
					if (primitiveIndex < TriangleCount
						&&
						(frustumCulling == FrustumCullingEnum::NoCulling || (Primitives[primitiveIndex] >= 0)))
					{
						switch (frustumCulling)
						{
						case FrustumCullingEnum::PrimitiveCulling:
						{
							const auto triangle = TriangleSource.GetTriangle(primitiveIndex);

							// If the triangle center is outside the frustum, mark it as culled.
							if (!frustum.IsPointInside(Vertices[triangle.a])
								&& !frustum.IsPointInside(Vertices[triangle.b])
								&& !frustum.IsPointInside(Vertices[triangle.c]))
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
								// Use vertex position in object space as normal.
								worldNormal = VertexSource.GetVertex(primitiveIndex);
								ApplyTransformRotation(MeshTransform, worldNormal);
								NormalizeVertex16(worldNormal);
							}

							// Apply scene shader to get lit color.
							primitiveColor = SceneShader->GetLitColor(primitiveColor, primitiveMaterial, Vertices[primitiveIndex], worldNormal);
						}

						// Cache the lit albedo color.
						LightBuffer.SetColor(primitiveColor, primitiveIndex);
					}

					return primitiveIndex >= MaxValue(TriangleCount, VertexCount);
				}

				/// <summary>
				/// Produces a triangle fragment for the rasterizer and calls the fragment shader.
				/// TODO: Split fragment if triangle is partially off-screen and emit the required sub-triangle fragments with correct UVs.
				/// </summary>
				/// <param name="rasterizer">Window rasterizer to receive the shaded triangle.</param>
				/// <param name="primitiveIndex">Triangle index.</param>
				virtual void FragmentShade(WindowRasterizer& rasterizer, const uint16_t primitiveIndex)
				{
					if (FragmentShader == nullptr)
						return;

					const auto triangle = TriangleSource.GetTriangle(primitiveIndex);

					Fragment.index = primitiveIndex;
					Fragment.z = Primitives[primitiveIndex];
					Fragment.vertexA = Vertices[triangle.a];
					Fragment.vertexB = Vertices[triangle.b];
					Fragment.vertexC = Vertices[triangle.c];

					{
						auto color = LightBuffer.GetColor(triangle.a);
						Fragment.redA = Rgb8::Red(color);
						Fragment.greenA = Rgb8::Green(color);
						Fragment.blueA = Rgb8::Blue(color);

						color = LightBuffer.GetColor(triangle.b);
						Fragment.redB = Rgb8::Red(color);
						Fragment.greenB = Rgb8::Green(color);
						Fragment.blueB = Rgb8::Blue(color);

						color = LightBuffer.GetColor(triangle.c);
						Fragment.redC = Rgb8::Red(color);
						Fragment.greenC = Rgb8::Green(color);
						Fragment.blueC = Rgb8::Blue(color);
					}

					if (UvSourceType::HasUvs())
					{
						const auto uvs = UvSource.GetUvs(primitiveIndex);
						Fragment.uvA = uvs.a;
						Fragment.uvB = uvs.b;
						Fragment.uvC = uvs.c;
					}

					FragmentShader->FragmentShade(rasterizer, Fragment);
				}
			};

			template<uint16_t vertexCount,
				uint16_t triangleCount,
				FrustumCullingEnum frustumCulling = FrustumCullingEnum::PrimitiveCulling,
				FaceCullingEnum faceCulling = FaceCullingEnum::BackfaceCulling>
			class SimpleStaticMeshVertexObject : public VertexShadeObject<
				vertexCount,
				triangleCount,
				PrimitiveSources::Vertex::Static::Source,
				PrimitiveSources::Triangle::Static::Source,
				frustumCulling,
				faceCulling,
				PrimitiveSources::Albedo::Dynamic::SingleSource,
				PrimitiveSources::Material::Dynamic::SingleSource>
			{
			private:
				using Base = VertexShadeObject<
					vertexCount,
					triangleCount,
					PrimitiveSources::Vertex::Static::Source,
					PrimitiveSources::Triangle::Static::Source,
					frustumCulling,
					faceCulling,
					PrimitiveSources::Albedo::Dynamic::SingleSource,
					PrimitiveSources::Material::Dynamic::SingleSource>;

			private:
				PrimitiveSources::Vertex::Static::Source VerticesSource;
				PrimitiveSources::Triangle::Static::Source TrianglesSource;

				PrimitiveSources::Albedo::Dynamic::SingleSource AlbedosSource{};
				PrimitiveSources::Material::Dynamic::SingleSource MaterialsSource{};

			public:
				SimpleStaticMeshVertexObject(const vertex16_t* vertices,
					const triangle_face_t* triangles)
					: VerticesSource(vertices)
					, TrianglesSource(triangles)
					, Base(VerticesSource, TrianglesSource, AlbedosSource, MaterialsSource)
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