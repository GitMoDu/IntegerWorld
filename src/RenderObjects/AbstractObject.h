#ifndef _INTEGER_WORLD_RENDER_OBJECTS_ABSTRACT_OBJECT_h
#define _INTEGER_WORLD_RENDER_OBJECTS_ABSTRACT_OBJECT_h

#include "../Framework/Interface.h"

#include "../PrimitiveSources/Vertex.h"
#include "../PrimitiveSources/Triangle.h"
#include "../PrimitiveSources/Albedo.h"
#include "../PrimitiveSources/Material.h"
#include "../PrimitiveSources/Normal.h"
#include "../PrimitiveSources/Uv.h"
#include "../PrimitiveSources/Texture.h"


namespace IntegerWorld
{
	namespace RenderObjects
	{
		/// <summary>
		/// Abstract Render Object.
		/// </summary>
		class AbstractObject : public IRenderObject
		{
		public:
			AbstractObject() : IRenderObject() {}

			// Default implementation since most objects do not have per-vertex animation.
			virtual bool VertexShade(const uint16_t vertexIndex)
			{
				return true;
			}
		};

		/// <summary>
		/// Render Object with only translation.
		/// </summary>
		class AbstractTranslationObject : public AbstractObject
		{
		public:
			vertex16_t Translation{};

		protected:
			vertex16_t WorldPosition{};

		public:
			AbstractTranslationObject() : AbstractObject() {}

			virtual void ObjectShade(const frustum_t& frustum)
			{
				// Apply world transform to object.
				WorldPosition = Translation;
			}
		};

		/// <summary>
		/// Render Object with full transform resize, translation and rotation.
		/// </summary>
		class AbstractTransformObject : public AbstractTranslationObject
		{
		public:
			rotation_angle_t Rotation{};
			scale16_t Resize = Scale16::SCALE_1X;

		protected:
			transform16_scale_rotate_translate_t MeshTransform{};

		public:
			AbstractTransformObject() : AbstractTranslationObject() {}

			virtual void ObjectShade(const frustum_t& frustum)
			{
				AbstractTranslationObject::ObjectShade(frustum);

				// Pre-calculate transform.
				MeshTransform.Translation = WorldPosition;
				MeshTransform.Resize = Resize;
				CalculateTransformRotation(MeshTransform, Rotation.x, Rotation.y, Rotation.z);
			}
		};

		/// <summary>
		/// Abstract render object with vertices and primitives' z cache.
		/// Implements world, camera transform and screen projection passes.
		/// </summary>
		template<uint16_t vertexCount,
			uint16_t primitiveCount>
		class TemplateTransformObject : public AbstractTransformObject
		{
		protected:
			vertex16_t Vertices[vertexCount]{};
			int16_t Primitives[primitiveCount]{};

		protected:
			uint16_t VertexCount = vertexCount;

		public:
			TemplateTransformObject() : AbstractTransformObject() {}

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
}
#endif