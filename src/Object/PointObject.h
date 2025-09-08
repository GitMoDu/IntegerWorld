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
		vertex16_t WorldPosition{};

	public:
		TranslationObject() : AbstractObject() {}

		virtual void ObjectShade(const frustum_t& frustum)
		{
			// Apply world transform to object.
			WorldPosition = Translation;
		}
	};

	/// <summary>
	/// Render Object with full transform resize, translation and rotation.
	/// </summary>
	class TransformObject : public TranslationObject
	{
	public:
		rotation_angle_t Rotation{};
		Scale16::factor_t Resize = Scale16::SCALE_1X;

	protected:
		transform16_scale_rotate_translate_t MeshTransform{};

	public:
		TransformObject() : TranslationObject() {}

		virtual void ObjectShade(const frustum_t& frustum)
		{
			TranslationObject::ObjectShade(frustum);

			// Pre-calculate transform.
			MeshTransform.Translation = WorldPosition;
			MeshTransform.Resize = Resize;
			CalculateTransformRotation(MeshTransform, Rotation.x, Rotation.y, Rotation.z);
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
	private:
		using Base = TransformObject;

	protected:
		vertex_t Vertices[vertexCount]{};
		primitive_t Primitives[primitiveCount]{};

	protected:
		uint16_t VertexCount;

	public:
		AbstractTransformObject()
			: TransformObject()
			, VertexCount(vertexCount)
		{
		}

	public:
		virtual bool WorldTransform(const uint16_t vertexIndex)
		{
			if (vertexIndex >= VertexCount)
				return true;

			ApplyTransform(MeshTransform, Vertices[vertexIndex]);

			return false;
		}

		virtual bool CameraTransform(const transform16_camera_t& transform, const uint16_t vertexIndex)
		{
			if (vertexIndex >= VertexCount)
				return true;

			ApplyCameraTransform(transform, Vertices[vertexIndex]);

			return false;
		}

		virtual bool ScreenProject(ViewportProjector& screenProjector, const uint16_t vertexIndex)
		{
			if (vertexIndex >= VertexCount)
				return true;

			screenProjector.Project(Vertices[vertexIndex]);

			return false;
		}
	};
}
#endif