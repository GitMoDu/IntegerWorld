#ifndef _INTEGER_WORLD_RENDER_OBJECTS_MESH_TRIANGLE_SHADE_OBJECT_h
#define _INTEGER_WORLD_RENDER_OBJECTS_MESH_TRIANGLE_SHADE_OBJECT_h

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
			class TriangleShadeObject : public AbstractObject<
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
				using BaseClass = AbstractObject<
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
				using BaseClass::Vertices;
				using BaseClass::Primitives;
				using BaseClass::TriangleSource;
				using BaseClass::TriangleCount;
				using BaseClass::MeshTransform;
				using BaseClass::AlbedoSource;
				using BaseClass::MaterialSource;
				using BaseClass::NormalSource;
				using BaseClass::UvSource;

			public:
				using fragment_t = mesh_triangle_fragment_t;

				/// <summary>
				/// Mesh triangle fragment shader. If null, no fragments are shaded.
				/// </summary>
				IFragmentShader<mesh_triangle_fragment_t>* FragmentShader = nullptr;

				/// <summary>
				/// Scene's lighting shader. If null, no lighting is applied.
				/// </summary>
				ISceneShader* SceneShader = nullptr;

			private:
				// Per-triangle lit albedo colors.
				CompactRgb8List<triangleCount> LightBuffer{};

				// Reusable triangle fragment.
				mesh_triangle_fragment_t Fragment{};

			public:
				TriangleShadeObject(VertexSourceType& vertexSource,
					TriangleSourceType& triangleSource,
					AlbedoSourceType& albedoSource = const_cast<AlbedoSourceType&>(PrimitiveSources::Albedo::FullAlbedoSourceInstance),
					MaterialSourceType& materialSource = const_cast<MaterialSourceType&>(PrimitiveSources::Material::DiffuseMaterialSourceInstance),
					NormalSourceType& normalSource = const_cast<NormalSourceType&>(PrimitiveSources::Normal::NormalNoSourceInstance),
					UvSourceType& uvSource = const_cast<UvSourceType&>(PrimitiveSources::Uv::NoUvSourceInstance))
					: BaseClass(vertexSource, triangleSource, albedoSource, materialSource, normalSource, uvSource)
				{
				}

				/// <summary>
				/// World pass:
				/// - Computes per-primitive world position (triangle center).
				/// - Optionally culls primitives against the frustum.
				/// - Computes or rotates the triangle world normal.
				/// - Applies the scene shader to light the primitive's albedo color.
				/// </summary>
				/// <returns>false to continue processing; true when no more primitives are available.</returns>
				virtual bool WorldShade(const frustum_t& frustum, const uint16_t primitiveIndex)
				{
					if (primitiveIndex >= TriangleCount)
						return true;

					if (Primitives[primitiveIndex] < 0)
						return false;

					const auto triangle = TriangleSource.GetTriangle(primitiveIndex);

					const vertex16_t worldPosition{
						AverageApproximate(Vertices[triangle.a].x, Vertices[triangle.b].x, Vertices[triangle.c].x),
						AverageApproximate(Vertices[triangle.a].y, Vertices[triangle.b].y, Vertices[triangle.c].y),
						AverageApproximate(Vertices[triangle.a].z, Vertices[triangle.b].z, Vertices[triangle.c].z) };

					switch (frustumCulling)
					{
					case FrustumCullingEnum::ObjectAndPrimitiveCulling:
					case FrustumCullingEnum::PrimitiveCulling:
						if (!frustum.IsPointInside(worldPosition))
						{
							Primitives[primitiveIndex] = -VERTEX16_UNIT;
							return false;
						}
						break;
					case FrustumCullingEnum::NoCulling:
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
							// Calculate triangle normal.
							worldNormal = GetNormal16(Vertices[triangle.a], Vertices[triangle.b], Vertices[triangle.c]);
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
				/// Produces a triangle fragment for the rasterizer and calls the fragment shader.
				/// TODO: Split fragment if triangle is partially off-screen and emit the required sub-triangle fragments with correct UVs.
				/// </summary>
				/// <param name="rasterizer">Window rasterizer to receive the shaded triangle.</param>
				/// <param name="primitiveIndex">Triangle index.</param>
				virtual void FragmentShade(WindowRasterizer& rasterizer, const uint16_t primitiveIndex)
				{
					if (FragmentShader == nullptr)
						return;

					Fragment.index = primitiveIndex;
					Fragment.z = Primitives[primitiveIndex];
					{
						const auto triangle = TriangleSource.GetTriangle(primitiveIndex);
						Fragment.vertexA = Vertices[triangle.a];
						Fragment.vertexB = Vertices[triangle.b];
						Fragment.vertexC = Vertices[triangle.c];
					}
					{
						const auto color = LightBuffer.GetColor(primitiveIndex);
						Fragment.red = Rgb8::Red(color);
						Fragment.green = Rgb8::Green(color);
						Fragment.blue = Rgb8::Blue(color);
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

			template<uint16_t vertexCount, uint16_t triangleCount,
				FrustumCullingEnum frustumCulling = FrustumCullingEnum::PrimitiveCulling,
				FaceCullingEnum faceCulling = FaceCullingEnum::BackfaceCulling>
			class SimpleStaticMeshTriangleObject : public TriangleShadeObject<
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
				using Base = TriangleShadeObject<
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
				SimpleStaticMeshTriangleObject(const vertex16_t* vertices, const triangle_face_t* triangles)
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