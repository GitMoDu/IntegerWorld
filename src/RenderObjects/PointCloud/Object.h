#ifndef _INTEGER_WORLD_RENDER_OBJECTS_POINT_CLOUD_OBJECT_h
#define _INTEGER_WORLD_RENDER_OBJECTS_POINT_CLOUD_OBJECT_h

#include "../AbstractObject.h"

namespace IntegerWorld
{
	namespace RenderObjects
	{
		namespace PointCloud
		{
			template<uint16_t vertexCount,
				typename VertexSourceType,
				FrustumCullingEnum frustumCulling = FrustumCullingEnum::PrimitiveCulling,
				FaceCullingEnum faceCulling = FaceCullingEnum::NoCulling,
				typename AlbedoSourceType = PrimitiveSources::Albedo::Static::FullSource,
				typename MaterialSourceType = PrimitiveSources::Material::DiffuseMaterialSource,
				typename NormalSourceType = PrimitiveSources::Normal::Static::NoSource
			>
			class ShadeObject : public TemplateTransformObject<vertexCount, vertexCount>
			{
			private:
				using Base = TemplateTransformObject<vertexCount, vertexCount>;

			protected:
				using Base::Vertices;
				using Base::Primitives;
				using Base::WorldPosition;
				using Base::MeshTransform;
				using Base::VertexCount;

			public:
				IFragmentShader<point_cloud_fragment_t>* FragmentShader = nullptr;

				ISceneShader* SceneShader = nullptr;

			protected:
				VertexSourceType& VertexSource;
				AlbedoSourceType& AlbedoSource;
				MaterialSourceType& MaterialSource;
				NormalSourceType& NormalSource;

			protected:
				// Screen space object position for object culling.
				vertex16_t ScreenPosition{};

			private:
				// Per-triangle lit albedo colors.
				CompactRgb8List<vertexCount> LightBuffer{};

				point_cloud_fragment_t Fragment{};

			public:
				ShadeObject(VertexSourceType& vertexSource,
					AlbedoSourceType& albedoSource = const_cast<AlbedoSourceType&>(PrimitiveSources::Albedo::FullAlbedoSourceInstance),
					MaterialSourceType& materialSource = const_cast<MaterialSourceType&>(PrimitiveSources::Material::DiffuseMaterialSourceInstance),
					NormalSourceType& normalSource = const_cast<NormalSourceType&>(PrimitiveSources::Normal::NormalNoSourceInstance))
					: Base()
					, VertexSource(vertexSource)
					, AlbedoSource(albedoSource)
					, MaterialSource(materialSource)
					, NormalSource(normalSource)
				{
				}

				virtual void ObjectShade(const frustum_t& frustum)
				{
					Base::ObjectShade(frustum);

					ScreenPosition = WorldPosition;

					int16_t zFlag = 0;

					switch (frustumCulling)
					{
					case FrustumCullingEnum::ObjectCulling:
						zFlag = -VERTEX16_UNIT * !frustum.IsPointInside(WorldPosition);
						break;
					default:
						break;
					};

					for (uint_fast16_t i = 0; i < VertexCount; i++)
					{
						auto vertex = VertexSource.GetVertex(i);
						Vertices[i].x = vertex.x;
						Vertices[i].y = vertex.y;
						Vertices[i].z = vertex.z;
						Primitives[i] = zFlag;
					}
				}

				virtual bool WorldShade(const frustum_t& frustum, const uint16_t primitiveIndex)
				{
					if (primitiveIndex >= VertexCount)
						return true;

					if (Primitives[primitiveIndex] < 0)
						return false;

					switch (frustumCulling)
					{
					case FrustumCullingEnum::PrimitiveCulling:
						if (!frustum.IsPointInside(Vertices[primitiveIndex]))
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
							primitiveMaterial = { 0, UFRACTION8_1X, 0, 0, 0, 0 };
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
							// Use vertex position in object space as normal.
							worldNormal.x = Vertices[primitiveIndex].x - WorldPosition.x;
							worldNormal.y = Vertices[primitiveIndex].y - WorldPosition.y;
							worldNormal.z = Vertices[primitiveIndex].z - WorldPosition.z;
							NormalizeVertex16(worldNormal);
						}

						// Apply scene shader to get lit color.
						primitiveColor = SceneShader->GetLitColor(primitiveColor, primitiveMaterial, Vertices[primitiveIndex], worldNormal);
					}

					// Cache the lit albedo color.
					LightBuffer.SetColor(primitiveColor, primitiveIndex);

					return false;
				}

				virtual bool CameraTransform(const transform16_camera_t& transform, const uint16_t vertexIndex)
				{
					if (vertexIndex == 0)
						ApplyCameraTransform(transform, ScreenPosition);

					return Base::CameraTransform(transform, vertexIndex);
				}

				virtual bool ScreenProject(ViewportProjector& screenProjector, const uint16_t vertexIndex)
				{
					if (vertexIndex == 0)
						screenProjector.Project(ScreenPosition);

					return Base::ScreenProject(screenProjector, vertexIndex);
				}

				virtual bool ScreenShade(const uint16_t primitiveIndex)
				{
					if (primitiveIndex >= VertexCount)
						return true;

					if (Primitives[primitiveIndex] >= 0)
					{
						Primitives[primitiveIndex] = Vertices[primitiveIndex].z;

						// Calculate screen normal and cull if facing away from viewport.
						switch (faceCulling)
						{
						case IntegerWorld::FaceCullingEnum::BackfaceCulling:
							if ((Vertices[primitiveIndex].z - ScreenPosition.z) > 0)
							{
								Primitives[primitiveIndex] = -VERTEX16_UNIT;
								return false;
							}
							break;
						case IntegerWorld::FaceCullingEnum::FrontfaceCulling:
							if ((Vertices[primitiveIndex].z - ScreenPosition.z) < 0)
							{
								Primitives[primitiveIndex] = -VERTEX16_UNIT;
								return false;
							}
							break;
						case IntegerWorld::FaceCullingEnum::NoCulling:
						default:
							break;
						}
					}

					return false;
				}

				/// <summary>
				/// Collects primitives that passed culling, pushing them to the fragment collector
				/// with their depth key for later sorting.
				/// </summary>
				void FragmentCollect(FragmentCollector& fragmentCollector)
				{
					for (uint_fast16_t i = 0; i < VertexCount; i++)
					{
						if (Primitives[i] >= 0)
						{
							fragmentCollector.AddFragment(i, Primitives[i]);
						}
					}
				}

				virtual void FragmentShade(WindowRasterizer& rasterizer, const uint16_t primitiveIndex)
				{
					if (FragmentShader == nullptr)
						return;

					Fragment.index = primitiveIndex;
					Fragment.x = Vertices[primitiveIndex].x;
					Fragment.y = Vertices[primitiveIndex].y;
					Fragment.z = Primitives[primitiveIndex];
					{
						const auto color = LightBuffer.GetColor(primitiveIndex);
						Fragment.red = Rgb8::Red(color);
						Fragment.green = Rgb8::Green(color);
						Fragment.blue = Rgb8::Blue(color);
					}

					FragmentShader->FragmentShade(rasterizer, Fragment);
				}
			};
		}
	}
}
#endif