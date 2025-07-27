#ifndef _INTEGER_WORLD_MESH_OBJECT_h
#define _INTEGER_WORLD_MESH_OBJECT_h

#include "AbstractObject.h"

namespace IntegerWorld
{
	struct mesh_vertex_t : base_vertex_t
	{
	};

	struct mesh_primitive_t : base_primitive_t
	{
	};

	struct mesh_world_primitive_t : base_primitive_t
	{
		vertex16_t worldPosition;
		vertex16_t worldNormal;
	};


	template<uint16_t vertexCount, uint16_t triangleCount,
		typename vertex_t = mesh_vertex_t,
		typename primitive_t = mesh_primitive_t>
	class AbstractMeshObject : public TransformObject
	{
	public:
		IFragmentShader<triangle_fragment_t>* FragmentShader = nullptr;

	protected:
		const vertex16_t* VerticesSource;
		const triangle_face_t* TrianglesSource;

	protected:
		vertex_t Vertices[vertexCount]{};
		primitive_t Primitives[triangleCount]{};

		triangle_fragment_t TriangleFragment{};

	protected:
		virtual void GeometryShade(const uint16_t index) {}

	public:
		AbstractMeshObject(const vertex16_t vertices[vertexCount], const triangle_face_t triangles[triangleCount])
			: TransformObject()
			, VerticesSource(vertices)
			, TrianglesSource(triangles)
		{
		}

	public:
		bool VertexShade(const uint16_t index) final
		{
			switch (index)
			{
			case 0:
				TransformObject::VertexShade(0);
				break;
			default:
#if defined(ARDUINO_ARCH_AVR)
				Vertices[index - 1].x = (int16_t)pgm_read_word(&VerticesSource[index - 1].x);
				Vertices[index - 1].y = (int16_t)pgm_read_word(&VerticesSource[index - 1].y);
				Vertices[index - 1].z = (int16_t)pgm_read_word(&VerticesSource[index - 1].z);
#else
				Vertices[index - 1].x = VerticesSource[index - 1].x;
				Vertices[index - 1].y = VerticesSource[index - 1].y;
				Vertices[index - 1].z = VerticesSource[index - 1].z;
#endif
				GeometryShade(index - 1);

				ApplyTransform(MeshTransform, Vertices[index - 1]);
				break;
			}

			return index >= vertexCount;
		}

		bool CameraTransform(const transform32_rotate_translate_t& transform, const uint16_t index) final
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

		bool ScreenProject(ViewportProjector& screenProjector, const uint16_t index) final
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

		bool PrimitiveScreenShade(const uint16_t index, const uint16_t boundsWidth, const uint16_t boundsHeight) final
		{
			if (index < triangleCount)
			{
#if defined(ARDUINO_ARCH_AVR)
				const triangle_face_t triangle
				{
					(uint16_t)pgm_read_word(&TrianglesSource[index].v1),
					(uint16_t)pgm_read_word(&TrianglesSource[index].v2),
					(uint16_t)pgm_read_word(&TrianglesSource[index].v3)
				};
#else
				const triangle_face_t& triangle = TrianglesSource[index];
#endif

				if (Primitives[index].z != -VERTEX16_RANGE)
				{
					// Check if triangle is entirely out of bounds.
					if ((Vertices[triangle.v1].z <= 0
						&& Vertices[triangle.v1].z <= 0
						&& Vertices[triangle.v1].z <= 0)
						|| (Vertices[triangle.v1].x <= 0
							&& Vertices[triangle.v2].x <= 0
							&& Vertices[triangle.v3].x <= 0)
						|| (Vertices[triangle.v1].x > boundsWidth
							&& Vertices[triangle.v2].x > boundsWidth
							&& Vertices[triangle.v3].x > boundsWidth)
						|| (Vertices[triangle.v1].y <= 0
							&& Vertices[triangle.v2].y <= 0
							&& Vertices[triangle.v3].y <= 0)
						|| (Vertices[triangle.v1].y > boundsHeight
							&& Vertices[triangle.v2].y > boundsHeight
							&& Vertices[triangle.v3].y > boundsHeight))
					{
						Primitives[index].z = -VERTEX16_RANGE;
					}
					else
					{
						// Back face culling after projection.
						const int32_t signedArea = (int32_t(Vertices[triangle.v2].x - Vertices[triangle.v1].x) * (Vertices[triangle.v3].y - Vertices[triangle.v1].y)) - (int32_t(Vertices[triangle.v2].y - Vertices[triangle.v1].y) * (Vertices[triangle.v3].x - Vertices[triangle.v1].x));
						if (signedArea < 0)
						{
							Primitives[index].z = AverageApproximate(Vertices[triangle.v1].z, Vertices[triangle.v2].z, Vertices[triangle.v3].z);
						}
						else
						{
							Primitives[index].z = -VERTEX16_RANGE;
						}
					}
				}
			}

			return index >= triangleCount - 1;
		}

		void FragmentCollect(FragmentCollector& fragmentCollector) final
		{
			for (uint16_t i = 0; i < triangleCount; i++)
			{
				if (Primitives[i].z != -VERTEX16_RANGE)
				{
					fragmentCollector.AddFragment(i, Primitives[i].z);
				}
			}
		}

	protected:
		virtual void GetFragment(triangle_fragment_t& fragment, const uint16_t index)
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

	template<uint16_t vertexCount, uint16_t triangleCount>
	class MeshWorldObject : public AbstractMeshObject<vertexCount, triangleCount, mesh_vertex_t, mesh_world_primitive_t>
	{
	private:
		using Base = AbstractMeshObject<vertexCount, triangleCount, mesh_vertex_t, mesh_world_primitive_t>;

	public:
		using Base::FragmentShader;
		using Base::SceneShader;

	protected:
		using Base::TrianglesSource;
		using Base::Vertices;
		using Base::Primitives;
		using Base::MeshTransform;
		using Base::TriangleFragment;
		using Base::GetFragment;

	protected:
		const vertex16_t* NormalsSource;

	public:
		MeshWorldObject(const vertex16_t vertices[vertexCount], const triangle_face_t triangles[triangleCount], const vertex16_t normals[triangleCount] = nullptr)
			: Base(vertices, triangles)
			, NormalsSource(normals)
		{
		}

		bool PrimitiveWorldShade(const uint16_t index) final
		{
			if (index < triangleCount)
			{
				//TODO: Check for world space frustum culling.
				// Flag fragment to render.
				Primitives[index].z = 0;

#if defined(ARDUINO_ARCH_AVR)
				const triangle_face_t triangle
				{
					(uint16_t)pgm_read_word(&TrianglesSource[index].v1),
					(uint16_t)pgm_read_word(&TrianglesSource[index].v2),
					(uint16_t)pgm_read_word(&TrianglesSource[index].v3)
				};
#else
				const triangle_face_t& triangle = TrianglesSource[index];
#endif

				Primitives[index].worldPosition.x = AverageApproximate(Vertices[triangle.v1].x, Vertices[triangle.v2].x, Vertices[triangle.v3].x);
				Primitives[index].worldPosition.y = AverageApproximate(Vertices[triangle.v1].y, Vertices[triangle.v2].y, Vertices[triangle.v3].y);
				Primitives[index].worldPosition.z = AverageApproximate(Vertices[triangle.v1].z, Vertices[triangle.v2].z, Vertices[triangle.v3].z);

				if (NormalsSource != nullptr)
				{
					// Rotate a pre-computed normal.
#if defined(ARDUINO_ARCH_AVR)
					Primitives[index].worldNormal =
					{
						(int16_t)pgm_read_word(&NormalsSource[index].x),
						(int16_t)pgm_read_word(&NormalsSource[index].y),
						(int16_t)pgm_read_word(&NormalsSource[index].z)
					};
#else
					Primitives[index].worldNormal = NormalsSource[index];
#endif
					ApplyRotate(MeshTransform, Primitives[index].worldNormal);
				}
				else
				{
					// Calculate triangle normal.
					vertex32_t normal{};
					GetNormal16(Vertices[triangle.v1], Vertices[triangle.v2], Vertices[triangle.v3], normal);
					NormalizeVertex32Fast(normal);

					Primitives[index].worldNormal.x = (int16_t)normal.x;
					Primitives[index].worldNormal.y = (int16_t)normal.y;
					Primitives[index].worldNormal.z = (int16_t)normal.z;
				}
			}

			return index >= triangleCount - 1;
		}

		void FragmentShade(WindowRasterizer& rasterizer, const uint16_t index) final
		{
			if (FragmentShader != nullptr)
			{
				const mesh_world_primitive_t& primitive = Primitives[index];

				TriangleFragment.normalWorld.x = primitive.worldNormal.x;
				TriangleFragment.normalWorld.y = primitive.worldNormal.y;
				TriangleFragment.normalWorld.z = primitive.worldNormal.z;
				TriangleFragment.world = primitive.worldPosition;

#if defined(ARDUINO_ARCH_AVR)
				const triangle_face_t triangle
				{
					(uint16_t)pgm_read_word(&TrianglesSource[index].v1),
					(uint16_t)pgm_read_word(&TrianglesSource[index].v2),
					(uint16_t)pgm_read_word(&TrianglesSource[index].v3)
				};
#else
				const triangle_face_t& triangle = TrianglesSource[index];
#endif
				TriangleFragment.triangleScreenA.x = Vertices[triangle.v1].x;
				TriangleFragment.triangleScreenA.y = Vertices[triangle.v1].y;
				TriangleFragment.triangleScreenA.z = Vertices[triangle.v1].z;
				TriangleFragment.triangleScreenB.x = Vertices[triangle.v2].x;
				TriangleFragment.triangleScreenB.y = Vertices[triangle.v2].y;
				TriangleFragment.triangleScreenB.z = Vertices[triangle.v2].z;
				TriangleFragment.triangleScreenC.x = Vertices[triangle.v3].x;
				TriangleFragment.triangleScreenC.y = Vertices[triangle.v3].y;
				TriangleFragment.triangleScreenC.z = Vertices[triangle.v3].z;

				GetFragment(TriangleFragment, index);

				if (SceneShader != nullptr)
				{
					FragmentShader->FragmentShade(rasterizer, TriangleFragment, SceneShader);
				}
				else
				{
					FragmentShader->FragmentShade(rasterizer, TriangleFragment);
				}
			}
		}
	};

	template<uint16_t vertexCount, uint16_t triangleCount>
	class MeshObject : public AbstractMeshObject<vertexCount, triangleCount, mesh_vertex_t, mesh_primitive_t>
	{
	private:
		using Base = AbstractMeshObject<vertexCount, triangleCount, mesh_vertex_t, mesh_primitive_t>;

	public:
		using Base::FragmentShader;
		using Base::SceneShader;

	protected:
		using Base::WorldPosition;
		using Base::MeshTransform;
		using Base::TrianglesSource;
		using Base::Vertices;
		using Base::Primitives;
		using Base::TriangleFragment;

	public:
		MeshObject(const vertex16_t vertices[vertexCount], const triangle_face_t triangles[triangleCount], const vertex16_t* normals = nullptr)
			: Base(vertices, triangles)
		{
		}

		bool PrimitiveWorldShade(const uint16_t index) final
		{
			for (uint16_t i = 0; i < triangleCount; i++)
			{
				//TODO: Check for world space frustum culling.
				// Flag fragment to render.
				Primitives[i].z = 0;
			}

			return true;
		}

		void FragmentShade(WindowRasterizer& rasterizer, const uint16_t index)
		{
			if (FragmentShader != nullptr)
			{
#if defined(ARDUINO_ARCH_AVR)
				const triangle_face_t triangle
				{
					(uint16_t)pgm_read_word(&TrianglesSource[index].v1),
					(uint16_t)pgm_read_word(&TrianglesSource[index].v2),
					(uint16_t)pgm_read_word(&TrianglesSource[index].v3)
				};
#else
				const triangle_face_t& triangle = TrianglesSource[index];
#endif

				TriangleFragment.triangleScreenA.x = Vertices[triangle.v1].x;
				TriangleFragment.triangleScreenA.y = Vertices[triangle.v1].y;
				TriangleFragment.triangleScreenA.z = Vertices[triangle.v1].z;
				TriangleFragment.triangleScreenB.x = Vertices[triangle.v2].x;
				TriangleFragment.triangleScreenB.y = Vertices[triangle.v2].y;
				TriangleFragment.triangleScreenB.z = Vertices[triangle.v2].z;
				TriangleFragment.triangleScreenC.x = Vertices[triangle.v3].x;
				TriangleFragment.triangleScreenC.y = Vertices[triangle.v3].y;
				TriangleFragment.triangleScreenC.z = Vertices[triangle.v3].z;

				// All primitives share the same Object world position.
				TriangleFragment.world = WorldPosition;
				TriangleFragment.normalWorld.x = 0;
				TriangleFragment.normalWorld.y = VERTEX16_UNIT;
				TriangleFragment.normalWorld.z = 0;

				GetFragment(TriangleFragment, index);

				if (SceneShader != nullptr)
				{
					// Calculate triangle normal.
					vertex32_t normal{};
					GetNormal16(Vertices[triangle.v1], Vertices[triangle.v2], Vertices[triangle.v3], normal);
					NormalizeVertex32Fast(normal);

					FragmentShader->FragmentShade(rasterizer, TriangleFragment, SceneShader);
				}
				else
				{
					FragmentShader->FragmentShade(rasterizer, TriangleFragment);
				}
			}
		}
	};

	template<uint16_t vertexCount, uint16_t triangleCount>
	class MeshWorldSingleColorSingleMaterialObject : public MeshWorldObject<vertexCount, triangleCount>
	{
	public:
		color_fraction16_t Color{ UFRACTION16_1X, UFRACTION16_1X ,UFRACTION16_1X };
		material_t Material{ UFRACTION8_1X, 0, 0, 0 };

	public:
		MeshWorldSingleColorSingleMaterialObject(
			const vertex16_t vertices[vertexCount],
			const triangle_face_t triangles[triangleCount],
			const vertex16_t normals[triangleCount] = nullptr)
			: MeshWorldObject<vertexCount, triangleCount>(vertices, triangles, normals)
		{
		}

	protected:
		virtual void GetFragment(triangle_fragment_t& fragment, const uint16_t index)
		{
			fragment.color = Color;
			fragment.material = Material;
		}
	};
}
#endif