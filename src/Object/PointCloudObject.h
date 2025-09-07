#ifndef _INTEGER_WORLD_POINT_CLOUD_OBJECT_h
#define _INTEGER_WORLD_POINT_CLOUD_OBJECT_h

#include "PointObject.h"

namespace IntegerWorld
{
	struct point_cloud_vertex_t : base_vertex_t
	{
	};

	struct flat_point_primitive_t : base_primitive_t
	{
		vertex16_t worldPosition;
	};

	struct lit_point_primitive_t : flat_point_primitive_t
	{
		vertex16_t worldNormal;
	};

	template<uint16_t vertexCount,
		typename vertex_t = point_cloud_vertex_t,
		typename primitive_t = lit_point_primitive_t,
		FrustumCullingEnum frustumCulling = FrustumCullingEnum::PrimitiveCulling>
	class LitPointCloudObject : public AbstractTransformObject<vertexCount, vertexCount, vertex_t, primitive_t>
	{
	private:
		using Base = AbstractTransformObject<vertexCount, vertexCount, vertex_t, primitive_t>;

	public:
		using Base::SceneShader;

	protected:
		using Base::Vertices;
		using Base::Primitives;
		using Base::WorldPosition;
		using Base::VertexCount;

	public:
		IFragmentShader<point_normal_fragment_t>* FragmentShader = nullptr;

	private:
		const vertex16_t* VerticesSource;

		// Screen space object position for edge culling.
		vertex16_t ScreenPosition{};

	private:
		point_normal_fragment_t PointFragment{};

	public:
		LitPointCloudObject(const vertex16_t vertices[vertexCount])
			: Base()
			, VerticesSource(vertices)
		{
		}

		virtual void ObjectShade(const frustum_t& frustum)
		{
			Base::ObjectShade(frustum);

			ScreenPosition = WorldPosition;

			int16_t zFlag = 0;

			switch (frustumCulling)
			{
			case FrustumCullingEnum::ObjectAndPrimitiveCulling:
			case FrustumCullingEnum::ObjectCulling:
				zFlag = -VERTEX16_UNIT * !frustum.IsPointInside(WorldPosition);
				break;
			default:
				break;
			};

			for (uint_fast16_t i = 0; i < VertexCount; i++)
			{
				Vertices[i].x = VerticesSource[i].x;
				Vertices[i].y = VerticesSource[i].y;
				Vertices[i].z = VerticesSource[i].z;
				Primitives[i].z = zFlag;
			}
		}

		virtual bool WorldShade(const frustum_t& frustum, const uint16_t primitiveIndex)
		{
			if (primitiveIndex >= VertexCount)
				return true;

			if (Primitives[primitiveIndex].z < 0)
				return false;

			switch (frustumCulling)
			{
			case FrustumCullingEnum::ObjectAndPrimitiveCulling:
			case FrustumCullingEnum::PrimitiveCulling:
				Primitives[primitiveIndex].z = -VERTEX16_UNIT * !frustum.IsPointInside(Vertices[primitiveIndex]);
				if (Primitives[primitiveIndex].z < 0)
					return false;
				break;
			default:
				break;
			};

			Primitives[primitiveIndex].worldPosition = Vertices[primitiveIndex];

			vertex32_t normal{ Vertices[primitiveIndex].x - WorldPosition.x,
								Vertices[primitiveIndex].y - WorldPosition.y,
								Vertices[primitiveIndex].z - WorldPosition.z };
			NormalizeVertex32Fast(normal);
			Primitives[primitiveIndex].worldNormal.x = normal.x;
			Primitives[primitiveIndex].worldNormal.y = normal.y;
			Primitives[primitiveIndex].worldNormal.z = normal.z;

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

			if (Primitives[primitiveIndex].z >= 0)
			{
				// Calculate screen normal and cull if facing away from viewport.
				const int32_t normalZ = int16_t(Vertices[primitiveIndex].z) - ScreenPosition.z;

				if (normalZ <= 0)
				{
					Primitives[primitiveIndex].z = -VERTEX16_UNIT;
				}

				return false;
			}
		}

		virtual void FragmentCollect(FragmentCollector& fragmentCollector, const uint16_t boundsWidth, const uint16_t boundsHeight)
		{
			for (uint_fast16_t i = 0; i < VertexCount; i++)
			{
				if (Primitives[i].z >= 0)
				{
					fragmentCollector.AddFragment(i, Primitives[i].z);
				}
			}
		}

		virtual void FragmentShade(WindowRasterizer& rasterizer, const uint16_t primitiveIndex)
		{
			if (FragmentShader != nullptr)
			{
				primitive_t& primitive = Primitives[primitiveIndex];

				GetFragment(PointFragment, primitiveIndex);

				PointFragment.world = primitive.worldPosition;
				PointFragment.normal = primitive.worldNormal;
				PointFragment.screen = Vertices[primitiveIndex];

				if (SceneShader != nullptr)
				{
					FragmentShader->FragmentShade(rasterizer, PointFragment, SceneShader);
				}
				else
				{
					FragmentShader->FragmentShade(rasterizer, PointFragment);
				}
			}
		}

	protected:
		virtual void GetFragment(point_fragment_t& fragment, const uint16_t primitiveIndex)
		{
			fragment.color = Rgb8::WHITE;
			fragment.material.Emissive = 0;
			fragment.material.Diffuse = UFRACTION8_1X;
			fragment.material.Specular = 0;
			fragment.material.Metallic = 0;
		}
	};

	template<uint16_t vertexCount,
		FrustumCullingEnum frustumCulling = FrustumCullingEnum::PrimitiveCulling>
	class FlatPointCloudObject : public AbstractTransformObject<vertexCount, vertexCount, point_cloud_vertex_t, flat_point_primitive_t>
	{
	private:
		using Base = AbstractTransformObject<vertexCount, vertexCount, point_cloud_vertex_t, flat_point_primitive_t>;

	public:
		using Base::SceneShader;

	protected:
		using Base::Vertices;
		using Base::Primitives;
		using Base::VertexCount;
		using Base::WorldPosition;

	public:
		IFragmentShader<point_fragment_t>* FragmentShader = nullptr;

	private:
		const vertex16_t* VerticesSource;

	private:
		point_fragment_t PointFragment{};

	public:
		FlatPointCloudObject(const vertex16_t vertices[vertexCount])
			: Base()
			, VerticesSource(vertices)
		{
		}

		virtual void ObjectShade(const frustum_t& frustum)
		{
			Base::ObjectShade(frustum);

			int16_t zFlag = 0;

			switch (frustumCulling)
			{
			case FrustumCullingEnum::ObjectAndPrimitiveCulling:
			case FrustumCullingEnum::ObjectCulling:
				zFlag = -VERTEX16_UNIT * !frustum.IsPointInside(WorldPosition);
				break;
			default:
				break;
			};

			for (uint_fast16_t i = 0; i < VertexCount; i++)
			{
				Vertices[i].x = VerticesSource[i].x;
				Vertices[i].y = VerticesSource[i].y;
				Vertices[i].z = VerticesSource[i].z;
				Primitives[i].z = zFlag;
			}
		}

		virtual bool WorldShade(const frustum_t& frustum, const uint16_t primitiveIndex)
		{
			if (primitiveIndex >= VertexCount)
				return true;

			if (Primitives[primitiveIndex].z < 0)
				return false;

			switch (frustumCulling)
			{
			case FrustumCullingEnum::ObjectAndPrimitiveCulling:
			case FrustumCullingEnum::PrimitiveCulling:
				Primitives[primitiveIndex].z = -VERTEX16_UNIT * !frustum.IsPointInside(Vertices[primitiveIndex]);
				break;
			default:
				break;
			};

			Primitives[primitiveIndex].worldPosition = Vertices[primitiveIndex];

			return false;
		}

		virtual bool ScreenShade(const uint16_t primitiveIndex)
		{
			if (primitiveIndex >= VertexCount)
				return true;

			if (Primitives[primitiveIndex].z < 0)
				return false;

			Primitives[primitiveIndex].z = Vertices[primitiveIndex].z;

			return false;
		}

		virtual void FragmentCollect(FragmentCollector& fragmentCollector)
		{
			for (uint_fast16_t i = 0; i < VertexCount; i++)
			{
				if (Primitives[i].z >= 0)
				{
					fragmentCollector.AddFragment(i, Primitives[i].z);
				}
			}
		}

		virtual void FragmentShade(WindowRasterizer& rasterizer, const uint16_t primitiveIndex)
		{
			if (FragmentShader != nullptr)
			{
				flat_point_primitive_t& primitive = Primitives[primitiveIndex];

				GetFragment(PointFragment, primitiveIndex);
				PointFragment.world = primitive.worldPosition;
				PointFragment.screen = Vertices[primitiveIndex];

				if (SceneShader != nullptr)
				{
					FragmentShader->FragmentShade(rasterizer, PointFragment, SceneShader);
				}
				else
				{
					FragmentShader->FragmentShade(rasterizer, PointFragment);
				}
			}
		}

	protected:
		virtual void GetFragment(point_fragment_t& fragment, const uint16_t primitiveIndex)
		{
			fragment.color = Rgb8::WHITE;
			fragment.material.Emissive = 0;
			fragment.material.Diffuse = UFRACTION8_1X;
			fragment.material.Specular = 0;
			fragment.material.Metallic = 0;
		}
	};
}
#endif