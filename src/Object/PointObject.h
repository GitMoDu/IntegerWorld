#ifndef _INTEGER_WORLD_POINT_OBJECT_h
#define _INTEGER_WORLD_POINT_OBJECT_h

#include "AbstractObject.h"

namespace IntegerWorld
{
	/// <summary>
	/// Render Object with only translation.
	/// </summary>
	class TranslationObject : public AbstractObject
	{
	public:
		vertex16_t Translation{};

	protected:
		vertex16_t ObjectPosition{};
		vertex16_t WorldPosition{};

	public:
		TranslationObject() : AbstractObject() {}

		virtual bool VertexShade(const uint16_t index)
		{
			// Apply world transform to object.
			WorldPosition = Translation;

			// Copy world position before camera transform.
			ObjectPosition = WorldPosition;

			return true;
		}

		virtual bool CameraTransform(const camera_transform_t& transform, const uint16_t index)
		{
			ApplyCameraTransform(transform, ObjectPosition);

			return true;
		}

		virtual bool ScreenProject(ViewportProjector& screenProjector, const uint16_t index)
		{
			screenProjector.Project(ObjectPosition);

			return true;
		}
	};

	/// <summary>
	/// Render Object with full transform resize, translation and rotation.
	/// </summary>
	class TransformObject : public TranslationObject
	{
	public:
		rotation_angle_t Rotation{};
		resize16_t Resize = RESIZE16_1X;

	protected:
		object_transform_t MeshTransform{};

	public:
		TransformObject() : TranslationObject() {}

		virtual bool VertexShade(const uint16_t index)
		{
			// Pre-calculate transform.
			MeshTransform.Translation.x = Translation.x;
			MeshTransform.Translation.y = Translation.y;
			MeshTransform.Translation.z = Translation.z;
			MeshTransform.Resize = Resize;
			CalculateTransformRotation(MeshTransform, Rotation.x, Rotation.y, Rotation.z);

			// Apply world transform to object.
			WorldPosition = Translation;

			// Copy world position before camera transform.
			ObjectPosition = WorldPosition;

			return true;
		}
	};

	/// <summary>
	/// Abstract render object with vertices and cached primitives.
	/// </summary>
	template<uint16_t vertexCount,
		uint16_t primitiveCount,
		typename vertex_t = base_vertex_t,
		typename primitive_t = base_primitive_t>
	class AbstractTransformObject : public TransformObject
	{
	protected:
		vertex_t Vertices[vertexCount]{};
		primitive_t Primitives[primitiveCount]{};

	public:
		AbstractTransformObject() : TransformObject() {}

	public:
		virtual bool VertexShade(const uint16_t index)
		{
			switch (index)
			{
			case 0:
				TransformObject::VertexShade(0);
				break;
			default:
				ApplyTransform(MeshTransform, Vertices[index - 1]);
				break;
			}

			return index >= vertexCount;
		}

		virtual bool CameraTransform(const camera_transform_t& transform, const uint16_t index)
		{
			switch (index)
			{
			case 0:
				ApplyCameraTransform(transform, ObjectPosition);
				break;
			default:
				ApplyCameraTransform(transform, Vertices[index - 1]);
				break;
			}

			return index >= vertexCount;
		}

		virtual bool ScreenProject(ViewportProjector& screenProjector, const uint16_t index)
		{
			switch (index)
			{
			case 0:
				screenProjector.Project(ObjectPosition);
				break;
			default:
				screenProjector.Project(Vertices[index - 1]);
				break;
			}

			return index >= vertexCount;
		}
	};
}
#endif