#ifndef _INTEGER_WORLD_RENDER_OBJECTS_MESH_ABSTRACT_OBJECT_h
#define _INTEGER_WORLD_RENDER_OBJECTS_MESH_ABSTRACT_OBJECT_h

#include "../AbstractObject.h"

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
			class AbstractObject : public TemplateTransformObject<vertexCount, triangleCount>
			{
			private:
				using Base = TemplateTransformObject<vertexCount, triangleCount>;

			protected:
				using Base::Vertices;
				using Base::Primitives;
				using Base::WorldPosition;
				using Base::VertexCount;
				using Base::MeshTransform;

			protected:
				VertexSourceType& VertexSource;
				TriangleSourceType& TriangleSource;
				AlbedoSourceType& AlbedoSource;
				MaterialSourceType& MaterialSource;
				NormalSourceType& NormalSource;
				UvSourceType& UvSource;

			protected:
				uint16_t TriangleCount = triangleCount;

			public:
				AbstractObject(VertexSourceType& vertexSource,
					TriangleSourceType& triangleSource,
					AlbedoSourceType& albedoSource = const_cast<AlbedoSourceType&>(PrimitiveSources::Albedo::FullAlbedoSourceInstance),
					MaterialSourceType& materialSource = const_cast<MaterialSourceType&>(PrimitiveSources::Material::DiffuseMaterialSourceInstance),
					NormalSourceType& normalSource = const_cast<NormalSourceType&>(PrimitiveSources::Normal::NormalNoSourceInstance),
					UvSourceType& uvSource = const_cast<UvSourceType&>(PrimitiveSources::Uv::NoUvSourceInstance)
				)
					: Base()
					, VertexSource(vertexSource)
					, TriangleSource(triangleSource)
					, AlbedoSource(albedoSource)
					, MaterialSource(materialSource)
					, NormalSource(normalSource)
					, UvSource(uvSource)
				{
				}

				/// <summary>
				/// Object pass:
				/// - Applies object-level frustum culling and initializes primitive z flags.
				/// - Loads source vertices into the working buffer if the object is inside the frustum.
				/// </summary>
				virtual void ObjectShade(const frustum_t& frustum)
				{
					Base::ObjectShade(frustum);

					int16_t zFlag = 0;

					switch (frustumCulling)
					{
					case FrustumCullingEnum::ObjectCulling:
						if (!frustum.IsPointInside(WorldPosition))
						{
							zFlag = -VERTEX16_UNIT;
						}
						for (uint_fast16_t i = 0; i < TriangleCount; i++)
						{
							Primitives[i] = zFlag;
						}
						break;
					case FrustumCullingEnum::PrimitiveCulling:
					case FrustumCullingEnum::NoCulling:
						for (uint_fast16_t i = 0; i < TriangleCount; i++)
						{
							Primitives[i] = 0;
						}
					default:
						break;
					};

					if (zFlag >= 0)
					{
						for (uint_fast16_t i = 0; i < VertexCount; i++)
						{
							auto vertex = VertexSource.GetVertex(i);

							Vertices[i].x = vertex.x;
							Vertices[i].y = vertex.y;
							Vertices[i].z = vertex.z;
						}
					}
				}

				/// <summary>
				/// Screen pass:
				/// - Applies mesh culling mode (backface/frontface/none) using projected 2D winding.
				/// - Emits a z key (average vertex z) for depth ordering when the primitive is visible.
				/// </summary>
				/// <returns>false to continue; true when no more primitives are available.</returns>
				virtual bool ScreenShade(const uint16_t primitiveIndex)
				{
					if (primitiveIndex >= TriangleCount)
						return true;

					if (Primitives[primitiveIndex] >= 0)
					{
						const auto triangle = TriangleSource.GetTriangle(primitiveIndex);

						switch (faceCulling)
						{
						case IntegerWorld::FaceCullingEnum::NoCulling:
							Primitives[primitiveIndex] = AverageApproximate(Vertices[triangle.a].z, Vertices[triangle.b].z, Vertices[triangle.c].z);
							return false;
						default:
							break;
						}

						// Back/Front face culling after projection.
						const int32_t signedArea = (static_cast<int32_t>(Vertices[triangle.b].x - Vertices[triangle.a].x)
							* (Vertices[triangle.c].y - Vertices[triangle.a].y))
							- (static_cast<int32_t>(Vertices[triangle.b].y - Vertices[triangle.a].y)
								* (Vertices[triangle.c].x - Vertices[triangle.a].x));

						switch (faceCulling)
						{
						case IntegerWorld::FaceCullingEnum::BackfaceCulling:
							if (signedArea < 0)
							{
								Primitives[primitiveIndex] = AverageApproximate(Vertices[triangle.a].z, Vertices[triangle.b].z, Vertices[triangle.c].z);
								return false;
							}
							break;
						case IntegerWorld::FaceCullingEnum::FrontfaceCulling:
							if (signedArea > 0)
							{
								Primitives[primitiveIndex] = AverageApproximate(Vertices[triangle.a].z, Vertices[triangle.b].z, Vertices[triangle.c].z);
								return false;
							}
							break;
						default:
							break;
						}

						// Culled.
						Primitives[primitiveIndex] = -VERTEX16_UNIT;
					}

					return false;
				}

				/// <summary>
				/// Collects primitives that passed culling, pushing them to the fragment collector
				/// with their depth key for later sorting.
				/// </summary>
				virtual void FragmentCollect(FragmentCollector& fragmentCollector) override
				{
					for (uint_fast16_t i = 0; i < TriangleCount; i++)
					{
						if (Primitives[i] >= 0)
						{
							fragmentCollector.AddFragment(i, Primitives[i]);
						}
					}
				}
			};
		}
	}
}
#endif