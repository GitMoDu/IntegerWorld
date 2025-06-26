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
		typename primitive_t = lit_point_primitive_t>
	class LitPointCloudObject : public AbstractOrderedTransformObject<vertexCount, vertexCount, vertex_t, primitive_t>
	{
	private:
		using BaseObject = AbstractOrderedTransformObject<vertexCount, vertexCount, vertex_t, primitive_t>;

	public:
		using BaseObject::SceneShader;

	protected:
		using BaseObject::ObjectPosition;
		using BaseObject::WorldPosition;

	protected:
		using BaseObject::Vertices;
		using BaseObject::Primitives;
		using BaseObject::MeshTransform;;

	public:
		IFragmentShader<point_normal_fragment_t>* FragmentShader = nullptr;

	private:
		const vertex16_t* VerticesSource;

	private:
		point_normal_fragment_t PointFragment{};

	protected:
		virtual void VertexAnimate(const uint16_t index) {}

	public:
		LitPointCloudObject(const vertex16_t vertices[vertexCount])
			: BaseObject()
			, VerticesSource(vertices)
		{
		}

		virtual bool VertexShade(const uint16_t index)
		{
			switch (index)
			{
			case 0:
				BaseObject::VertexShade(0);
				break;
			default:
				Vertices[index - 1].x = VerticesSource[index - 1].x;
				Vertices[index - 1].y = VerticesSource[index - 1].y;
				Vertices[index - 1].z = VerticesSource[index - 1].z;

				VertexAnimate(index - 1);

				ApplyTransform(MeshTransform, Vertices[index - 1]);
				break;
			}

			return index >= vertexCount;
		}

		virtual bool PrimitiveWorldShade(const uint16_t index)
		{
			if (index < vertexCount)
			{
				//TODO: Check for world space frustum culling.
				// Flag fragment to render.
				Primitives[index].z = 0;
				Primitives[index].worldPosition = Vertices[index];

				vertex32_t normal{ Vertices[index].x - WorldPosition.x,
									Vertices[index].y - WorldPosition.y,
									Vertices[index].z - WorldPosition.z };
				NormalizeVertex32Fast(normal);
				Primitives[index].worldNormal.x = normal.x;
				Primitives[index].worldNormal.y = normal.y;
				Primitives[index].worldNormal.z = normal.z;
			}

			return index >= vertexCount - 1;
		}

		virtual bool PrimitiveScreenShade(const uint16_t index)
		{
			if (index < vertexCount)
			{
				//TODO: Check for screen space frustum culling.
				if (Primitives[index].z != VERTEX16_RANGE)
				{
					Primitives[index].z = Vertices[index].z;
					if (Primitives[index].z <= 0)
					{
						Primitives[index].z = VERTEX16_RANGE;
					}
					else
					{
						// Calculate screen normal and cull if facing away from viewport.
						const int32_t normalZ = int16_t(Vertices[index].z) - ObjectPosition.z;
						if (normalZ > 0)
						{
							Primitives[index].z = VERTEX16_RANGE;
						}
					}
				}
			}

			return index >= vertexCount - 1;
		}

		virtual void FragmentCollect(FragmentCollector& fragmentCollector, const uint16_t boundsWidth, const uint16_t boundsHeight)
		{
			for (uint16_t i = 0; i < vertexCount; i++)
			{
				const int16_t fragmentZ = Primitives[i].z;
				if (fragmentZ != VERTEX16_RANGE
					&& Vertices[i].x >= 0
					&& Vertices[i].y >= 0
					&& Vertices[i].x < boundsWidth
					&& Vertices[i].y < boundsHeight)
				{
					fragmentCollector.AddFragment(i, fragmentZ);
				}
				else
				{
					Primitives[i].z = VERTEX16_RANGE;
				}
			}
		}

		virtual void FragmentShade(WindowRasterizer& rasterizer, const uint16_t index)
		{
			if (FragmentShader != nullptr)
			{
				primitive_t& primitive = Primitives[index];

				if (primitive.z != VERTEX16_RANGE)
				{
					GetFragment(PointFragment, index);

					PointFragment.world = primitive.worldPosition;
					PointFragment.normal = primitive.worldNormal;
					PointFragment.screen = Vertices[index];

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
		}

	protected:
		virtual void GetFragment(point_fragment_t& fragment, const uint16_t index)
		{
			fragment.color.r = UFRACTION16_1X;
			fragment.color.g = UFRACTION16_1X;
			fragment.color.b = UFRACTION16_1X;
			fragment.material.Emissive = 0;
			fragment.material.Diffuse = UFRACTION8_1X;
			fragment.material.Specular = 0;
			fragment.material.Metallic = 0;
		}
	};

	template<uint16_t vertexCount,
		typename vertex_t = point_cloud_vertex_t,
		typename primitive_t = flat_point_primitive_t>
	class FlatPointCloudObject : public AbstractTransformObject<vertexCount, vertexCount, vertex_t, primitive_t>
	{
		using BaseObject = AbstractTransformObject<vertexCount, vertexCount, vertex_t, primitive_t>;

	public:
		using BaseObject::SceneShader;

	protected:
		using BaseObject::Vertices;
		using BaseObject::Primitives;
		using BaseObject::MeshTransform;

	public:
		IFragmentShader<point_fragment_t>* FragmentShader = nullptr;

	private:
		const vertex16_t* VerticesSource;

	private:
		point_fragment_t PointFragment{};

	protected:
		virtual void GeometryShade(const uint16_t index) {}

	public:
		FlatPointCloudObject(const vertex16_t vertices[vertexCount])
			: BaseObject()
			, VerticesSource(vertices)
		{
		}

		virtual bool VertexShade(const uint16_t index)
		{
			switch (index)
			{
			case 0:
				return BaseObject::VertexShade(0);
			default:
				Vertices[index - 1].x = VerticesSource[index - 1].x;
				Vertices[index - 1].y = VerticesSource[index - 1].y;
				Vertices[index - 1].z = VerticesSource[index - 1].z;

				GeometryShade(index - 1);

				ApplyTransform(MeshTransform, Vertices[index - 1]);
				break;
			}

			return index >= vertexCount;
		}

		virtual bool PrimitiveWorldShade(const uint16_t index)
		{
			//TODO: Check for world space frustum culling.
			// Flag fragment to render.
			Primitives[index].z = 0;
			Primitives[index].worldPosition = Vertices[index];

			return index >= vertexCount - 1;
		}

		virtual bool PrimitiveScreenShade(const uint16_t index, const uint16_t boundsWidth, const uint16_t boundsHeight)
		{
			//TODO: Check for screen space frustum culling.
			// Check flagged fragments.
			if (Primitives[index].z != VERTEX16_RANGE
				&& Vertices[index].z >= 0
				&& Vertices[index].x >= 0
				&& Vertices[index].y >= 0
				&& Vertices[index].x < boundsWidth
				&& Vertices[index].y < boundsHeight)
			{
				Primitives[index].z = Vertices[index].z;
			}
			else
			{
				Primitives[index].z = VERTEX16_RANGE;
			}

			return index >= vertexCount - 1;
		}

		virtual void FragmentCollect(FragmentCollector& fragmentCollector)
		{
			for (uint16_t i = 0; i < vertexCount; i++)
			{
				if (Primitives[i].z != VERTEX16_RANGE)
				{
					fragmentCollector.AddFragment(i, Primitives[i].z);
				}
				else
				{
					Primitives[i].z = VERTEX16_RANGE;
				}
			}
		}

		virtual void FragmentShade(WindowRasterizer& rasterizer, const uint16_t index)
		{
			if (FragmentShader != nullptr)
			{
				primitive_t& primitive = Primitives[index];

				GetFragment(PointFragment, index);
				PointFragment.world = primitive.worldPosition;
				PointFragment.screen = Vertices[index];

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
		virtual void GetFragment(point_fragment_t& fragment, const uint16_t index)
		{
			fragment.color.r = UFRACTION16_1X;
			fragment.color.g = UFRACTION16_1X;
			fragment.color.b = UFRACTION16_1X;
			fragment.material.Emissive = 0;
			fragment.material.Diffuse = UFRACTION8_1X;
			fragment.material.Specular = 0;
			fragment.material.Metallic = 0;
		}
	};
}
#endif