#ifndef _INTEGER_WORLD_RENDER_OBJECTS_EDGE_ABSTRACT_OBJECT_h
#define _INTEGER_WORLD_RENDER_OBJECTS_EDGE_ABSTRACT_OBJECT_h

#include "../AbstractObject.h"

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
			class AbstractObject : public TemplateTransformObject<vertexCount, edgeCount>
			{
			private:
				using Base = TemplateTransformObject<vertexCount, edgeCount>;

			protected:
				using Base::Vertices;
				using Base::Primitives;
				using Base::WorldPosition;
				using Base::MeshTransform;
				using Base::VertexCount;

			protected:
				VertexSourceType& VertexSource;
				EdgeSourceType& EdgeSource;
				AlbedoSourceType& AlbedoSource;
				MaterialSourceType& MaterialSource;
				NormalSourceType& NormalSource;

			protected:
				// Screen space object position for object culling.
				vertex16_t ScreenPosition{};

			protected:
				uint16_t EdgeCount = edgeCount;

			public:
				AbstractObject(VertexSourceType& vertexSource,
					EdgeSourceType& edgeSource,
					AlbedoSourceType& albedoSource = const_cast<AlbedoSourceType&>(PrimitiveSources::Albedo::FullAlbedoSourceInstance),
					MaterialSourceType& materialSource = const_cast<MaterialSourceType&>(PrimitiveSources::Material::DiffuseMaterialSourceInstance),
					NormalSourceType& normalSource = const_cast<NormalSourceType&>(PrimitiveSources::Normal::NormalNoSourceInstance)
				)
					: Base()
					, VertexSource(vertexSource)
					, EdgeSource(edgeSource)
					, AlbedoSource(albedoSource)
					, MaterialSource(materialSource)
					, NormalSource(normalSource)
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

					ScreenPosition = WorldPosition;

					int16_t zFlag = 0;

					switch (frustumCulling)
					{
					case FrustumCullingEnum::ObjectCulling:
						if (!frustum.IsPointInside(WorldPosition))
						{
							zFlag = -VERTEX16_UNIT;
						}
						for (uint_fast16_t i = 0; i < EdgeCount; i++)
						{
							Primitives[i] = zFlag;
						}
						break;
					case FrustumCullingEnum::PrimitiveCulling:
					case FrustumCullingEnum::NoCulling:
						for (uint_fast16_t i = 0; i < EdgeCount; i++)
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
							const auto vertex = VertexSource.GetVertex(i);

							Vertices[i].x = vertex.x;
							Vertices[i].y = vertex.y;
							Vertices[i].z = vertex.z;
						}
					}
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

				/// <summary>
				/// Screen pass:
				/// - Applies mesh culling mode (backface/frontface/none) using projected 2D winding.
				/// - Emits a z key (average vertex z) for depth ordering when the primitive is visible.
				/// </summary>
				/// <returns>false to continue; true when no more primitives are available.</returns>
				virtual bool ScreenShade(const uint16_t primitiveIndex)
				{
					if (primitiveIndex >= EdgeCount)
						return true;

					if (Primitives[primitiveIndex] >= 0)
					{
						const auto edge = EdgeSource.GetEdge(primitiveIndex);

						// Compute average edge depth for depth sorting.
						Primitives[primitiveIndex] = Average(Vertices[edge.a].z, Vertices[edge.b].z);

						// Compute 2D edge normal z components.
						switch (faceCulling)
						{
						case IntegerWorld::FaceCullingEnum::NoCulling:
							break;
						case IntegerWorld::FaceCullingEnum::BackfaceCulling:
							if ((Primitives[primitiveIndex] - ScreenPosition.z) > 0)
							{
								Primitives[primitiveIndex] = -VERTEX16_UNIT;
								return false;
							}
							break;
						case IntegerWorld::FaceCullingEnum::FrontfaceCulling:
							if ((Primitives[primitiveIndex] - ScreenPosition.z) < 0)
							{
								Primitives[primitiveIndex] = -VERTEX16_UNIT;
								return false;
							}
							break;
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
				void FragmentCollect(FragmentCollector& fragmentCollector) final
				{
					for (uint_fast16_t i = 0; i < EdgeCount; i++)
					{
						if (Primitives[i] >= 0)
						{
							fragmentCollector.AddFragment(i, Primitives[i]);
						}
					}
				}

			protected:
				static constexpr int16_t Average(const int16_t a, const int16_t b)
				{
					return static_cast<int16_t>(SignedRightShift(static_cast<int32_t>(a) + b, 1));
				}
			};
		}
	}
}
#endif