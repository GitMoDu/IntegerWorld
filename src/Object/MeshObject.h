#ifndef _INTEGER_WORLD_MESH_OBJECT_h
#define _INTEGER_WORLD_MESH_OBJECT_h

#include "AbstractObject.h"

namespace IntegerWorld
{
	enum class MeshCullingEnum : uint8_t
	{
		BackfaceCullling,
		NoBackfaceCullling
	};

	template<uint16_t vertexCount, uint16_t triangleCount,
		typename vertex_t = mesh_vertex_t,
		typename primitive_t = mesh_primitive_t,
		FrustumCullingEnum frustumCulling = FrustumCullingEnum::ObjectCulling,
		MeshCullingEnum meshCulling = MeshCullingEnum::BackfaceCullling>
	class AbstractMeshObject : public AbstractTransformObject<vertexCount, triangleCount, vertex_t, primitive_t>
	{
	private:
		using Base = AbstractTransformObject<vertexCount, triangleCount, vertex_t, primitive_t>;

	public:
		IFragmentShader<triangle_fragment_t>* FragmentShader = nullptr;

	protected:
		using Base::Vertices;
		using Base::Primitives;
		using Base::VertexCount;
		using Base::WorldPosition;

	protected:
		const vertex16_t* VerticesSource;
		const triangle_face_t* TrianglesSource;

	protected:
		triangle_fragment_t TriangleFragment{};

	public:
		AbstractMeshObject(const vertex16_t vertices[vertexCount], const triangle_face_t triangles[triangleCount])
			: Base()
			, VerticesSource(vertices)
			, TrianglesSource(triangles)
		{
		}

	public:
		virtual void ObjectShade(const frustum_t& frustum)
		{
			Base::ObjectShade(frustum);

			int16_t zFlag = 0;

			switch (frustumCulling)
			{
			case FrustumCullingEnum::ObjectAndPrimitiveCulling:
			case FrustumCullingEnum::ObjectCulling:
				if (!frustum.IsPointInside(WorldPosition))
				{
					zFlag = -VERTEX16_UNIT;
				}
				for (uint_fast16_t i = 0; i < triangleCount; i++)
				{
					Primitives[i].z = zFlag;
				}
				break;
			case FrustumCullingEnum::PrimitiveCulling:
				for (uint_fast16_t i = 0; i < triangleCount; i++)
				{
					Primitives[i].z = 0;
				}
				break;
			case FrustumCullingEnum::NoCulling:
			default:
				break;
			};

			if (zFlag >= 0)
			{
				for (uint_fast16_t i = 0; i < VertexCount; i++)
				{
#if defined(ARDUINO_ARCH_AVR)
					Vertices[i].x = (int16_t)pgm_read_word(&VerticesSource[i].x);
					Vertices[i].y = (int16_t)pgm_read_word(&VerticesSource[i].y);
					Vertices[i].z = (int16_t)pgm_read_word(&VerticesSource[i].z);
#else
					Vertices[i].x = VerticesSource[i].x;
					Vertices[i].y = VerticesSource[i].y;
					Vertices[i].z = VerticesSource[i].z;
#endif
				}
			}
		}

		bool WorldShade(const frustum_t& frustum, const uint16_t primitiveIndex)
		{
			if (primitiveIndex >= triangleCount)
				return true;

			switch (frustumCulling)
			{
			case FrustumCullingEnum::ObjectAndPrimitiveCulling:
			case FrustumCullingEnum::PrimitiveCulling:
				if (Primitives[primitiveIndex].z >= 0)
				{
					{
#if defined(ARDUINO_ARCH_AVR)
						const triangle_face_t triangle
						{
							(uint16_t)pgm_read_word(&TrianglesSource[primitiveIndex].v1),
							(uint16_t)pgm_read_word(&TrianglesSource[primitiveIndex].v2),
							(uint16_t)pgm_read_word(&TrianglesSource[primitiveIndex].v3)
						};
#else
						const triangle_face_t& triangle = TrianglesSource[primitiveIndex];
#endif
						const vertex16_t triangleCenter
						{
							AverageApproximate(Vertices[triangle.v1].x, Vertices[triangle.v2].x, Vertices[triangle.v3].x),
							AverageApproximate(Vertices[triangle.v1].y, Vertices[triangle.v2].y, Vertices[triangle.v3].y),
							AverageApproximate(Vertices[triangle.v1].z, Vertices[triangle.v2].z, Vertices[triangle.v3].z)
						};

						if (!frustum.IsPointInside(triangleCenter))
							Primitives[primitiveIndex].z = -VERTEX16_UNIT;
					}
				}
				break;
			case FrustumCullingEnum::NoCulling:
				Primitives[primitiveIndex].z = 0;
			default:
				break;
			};

			return false;
		}

		bool ScreenShade(const uint16_t primitiveIndex)
		{
			if (primitiveIndex >= triangleCount)
				return true;

			if (Primitives[primitiveIndex].z >= 0)
			{
#if defined(ARDUINO_ARCH_AVR)
				const triangle_face_t triangle
				{
					(uint16_t)pgm_read_word(&TrianglesSource[primitiveIndex].v1),
					(uint16_t)pgm_read_word(&TrianglesSource[primitiveIndex].v2),
					(uint16_t)pgm_read_word(&TrianglesSource[primitiveIndex].v3)
				};
#else
				const triangle_face_t& triangle = TrianglesSource[primitiveIndex];
#endif

				if (meshCulling == MeshCullingEnum::BackfaceCullling)
				{
					// Back face culling after projection.
					const int32_t signedArea = (int32_t(Vertices[triangle.v2].x - Vertices[triangle.v1].x)
						* (Vertices[triangle.v3].y - Vertices[triangle.v1].y))
						- (int32_t(Vertices[triangle.v2].y - Vertices[triangle.v1].y)
							* (Vertices[triangle.v3].x - Vertices[triangle.v1].x));

					if (signedArea < 0)
					{
						Primitives[primitiveIndex].z = AverageApproximate(Vertices[triangle.v1].z, Vertices[triangle.v2].z, Vertices[triangle.v3].z);
					}
					else
					{
						Primitives[primitiveIndex].z = -VERTEX16_UNIT;
					}
				}
				else
				{
					Primitives[primitiveIndex].z = AverageApproximate(Vertices[triangle.v1].z, Vertices[triangle.v2].z, Vertices[triangle.v3].z);
				}
			}

			return false;
		}

		void FragmentCollect(FragmentCollector& fragmentCollector) final
		{
			for (uint_fast16_t i = 0; i < triangleCount; i++)
			{
				if (Primitives[i].z >= 0)
				{
					fragmentCollector.AddFragment(i, Primitives[i].z);
				}
			}
		}

	protected:
		virtual void GetFragment(triangle_fragment_t& fragment, const uint16_t primitiveIndex)
		{
			fragment.color = Rgb8::WHITE;
			fragment.material.Emissive = 0;
			fragment.material.Diffuse = UFRACTION8_1X;
			fragment.material.Specular = 0;
			fragment.material.Metallic = 0;
		}
	};

	template<uint16_t vertexCount, uint16_t triangleCount,
		FrustumCullingEnum frustumCulling = FrustumCullingEnum::ObjectCulling,
		MeshCullingEnum meshCulling = MeshCullingEnum::BackfaceCullling>
	class MeshWorldObject : public AbstractMeshObject<vertexCount, triangleCount, mesh_vertex_t, mesh_world_primitive_t, frustumCulling, meshCulling>
	{
	private:
		using Base = AbstractMeshObject<vertexCount, triangleCount, mesh_vertex_t, mesh_world_primitive_t, frustumCulling, meshCulling>;

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

		bool WorldShade(const frustum_t& frustum, const uint16_t primitiveIndex)
		{
			if (primitiveIndex >= triangleCount)
				return true;

			// Check for world space frustum culling.
			if (Primitives[primitiveIndex].z < 0)
				return false;

#if defined(ARDUINO_ARCH_AVR)
			const triangle_face_t triangle
			{
				(uint16_t)pgm_read_word(&TrianglesSource[primitiveIndex].v1),
				(uint16_t)pgm_read_word(&TrianglesSource[primitiveIndex].v2),
				(uint16_t)pgm_read_word(&TrianglesSource[primitiveIndex].v3)
			};
#else
			const triangle_face_t& triangle = TrianglesSource[primitiveIndex];
#endif

			Primitives[primitiveIndex].worldPosition.x = AverageApproximate(Vertices[triangle.v1].x, Vertices[triangle.v2].x, Vertices[triangle.v3].x);
			Primitives[primitiveIndex].worldPosition.y = AverageApproximate(Vertices[triangle.v1].y, Vertices[triangle.v2].y, Vertices[triangle.v3].y);
			Primitives[primitiveIndex].worldPosition.z = AverageApproximate(Vertices[triangle.v1].z, Vertices[triangle.v2].z, Vertices[triangle.v3].z);

			switch (frustumCulling)
			{
			case FrustumCullingEnum::ObjectAndPrimitiveCulling:
			case FrustumCullingEnum::PrimitiveCulling:
				if (!frustum.IsPointInside(Primitives[primitiveIndex].worldPosition))
				{
					Primitives[primitiveIndex].z = -VERTEX16_UNIT;
					return false;
				}
				break;
			default:
				break;
			};

			if (NormalsSource != nullptr)
			{
				// Rotate a pre-computed normal.
#if defined(ARDUINO_ARCH_AVR)
				Primitives[primitiveIndex].worldNormal =
				{
					(int16_t)pgm_read_word(&NormalsSource[primitiveIndex].x),
					(int16_t)pgm_read_word(&NormalsSource[primitiveIndex].y),
					(int16_t)pgm_read_word(&NormalsSource[primitiveIndex].z)
				};
#else
				Primitives[primitiveIndex].worldNormal = NormalsSource[primitiveIndex];
#endif
				ApplyTransformRotation(MeshTransform, Primitives[primitiveIndex].worldNormal);
			}
			else
			{
				// Calculate triangle normal.
				vertex32_t normal{};
				GetNormal16(Vertices[triangle.v1], Vertices[triangle.v2], Vertices[triangle.v3], normal);
				NormalizeVertex32Fast(normal);

				Primitives[primitiveIndex].worldNormal.x = (int16_t)normal.x;
				Primitives[primitiveIndex].worldNormal.y = (int16_t)normal.y;
				Primitives[primitiveIndex].worldNormal.z = (int16_t)normal.z;
			}

			return false;
		}

		void FragmentShade(WindowRasterizer& rasterizer, const uint16_t primitiveIndex) final
		{
			if (FragmentShader != nullptr)
			{
				const mesh_world_primitive_t& primitive = Primitives[primitiveIndex];

				TriangleFragment.normalWorld.x = primitive.worldNormal.x;
				TriangleFragment.normalWorld.y = primitive.worldNormal.y;
				TriangleFragment.normalWorld.z = primitive.worldNormal.z;
				TriangleFragment.world = primitive.worldPosition;

#if defined(ARDUINO_ARCH_AVR)
				const triangle_face_t triangle
				{
					(uint16_t)pgm_read_word(&TrianglesSource[primitiveIndex].v1),
					(uint16_t)pgm_read_word(&TrianglesSource[primitiveIndex].v2),
					(uint16_t)pgm_read_word(&TrianglesSource[primitiveIndex].v3)
				};
#else
				const triangle_face_t& triangle = TrianglesSource[primitiveIndex];
#endif
				TriangleFragment.triangleScreenA = Vertices[triangle.v1];
				TriangleFragment.triangleScreenB = Vertices[triangle.v2];
				TriangleFragment.triangleScreenC = Vertices[triangle.v3];

				GetFragment(TriangleFragment, primitiveIndex);

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

	template<uint16_t vertexCount, uint16_t triangleCount,
		FrustumCullingEnum frustumCulling = FrustumCullingEnum::ObjectCulling,
		MeshCullingEnum meshCulling = MeshCullingEnum::BackfaceCullling>
	class MeshScreenObject : public AbstractMeshObject<vertexCount, triangleCount, mesh_vertex_t, mesh_primitive_t, frustumCulling, meshCulling>
	{
	private:
		using Base = AbstractMeshObject<vertexCount, triangleCount, mesh_vertex_t, mesh_primitive_t, frustumCulling, meshCulling>;

	public:
		using Base::FragmentShader;
		using Base::SceneShader;

	protected:
		using Base::Vertices;
		using Base::WorldPosition;
		using Base::TrianglesSource;
		using Base::Primitives;
		using Base::TriangleFragment;

	protected:
		// Screen space object position.
		vertex16_t ScreenPosition{};

	public:
		MeshScreenObject(const vertex16_t vertices[vertexCount], const triangle_face_t triangles[triangleCount], const vertex16_t* normals = nullptr)
			: Base(vertices, triangles)
		{
		}

		virtual void ObjectShade(const frustum_t& frustum)
		{
			Base::ObjectShade(frustum);

			ScreenPosition = WorldPosition;
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

		virtual void FragmentShade(WindowRasterizer& rasterizer, const uint16_t primitiveIndex)
		{
			if (FragmentShader != nullptr)
			{
#if defined(ARDUINO_ARCH_AVR)
				const triangle_face_t triangle
				{
					(uint16_t)pgm_read_word(&TrianglesSource[primitiveIndex].v1),
					(uint16_t)pgm_read_word(&TrianglesSource[primitiveIndex].v2),
					(uint16_t)pgm_read_word(&TrianglesSource[primitiveIndex].v3)
				};
#else
				const triangle_face_t& triangle = TrianglesSource[primitiveIndex];
#endif
				TriangleFragment.triangleScreenA = Vertices[triangle.v1];
				TriangleFragment.triangleScreenB = Vertices[triangle.v2];
				TriangleFragment.triangleScreenC = Vertices[triangle.v3];

				// All primitives share the same Object world position.
				TriangleFragment.world = WorldPosition;
				TriangleFragment.normalWorld.x = 0;
				TriangleFragment.normalWorld.y = VERTEX16_UNIT;
				TriangleFragment.normalWorld.z = 0;

				GetFragment(TriangleFragment, primitiveIndex);

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

	template<uint16_t vertexCount, uint16_t triangleCount,
		FrustumCullingEnum frustumCulling = FrustumCullingEnum::ObjectCulling>
	class MeshWorldSingleColorSingleMaterialObject : public MeshWorldObject<vertexCount, triangleCount, frustumCulling>
	{
	public:
		Rgb8::color_t Color = Rgb8::WHITE;
		material_t Material{ UFRACTION8_1X, 0, 0, 0 };

	public:
		MeshWorldSingleColorSingleMaterialObject(
			const vertex16_t vertices[vertexCount],
			const triangle_face_t triangles[triangleCount],
			const vertex16_t normals[triangleCount] = nullptr)
			: MeshWorldObject<vertexCount, triangleCount, frustumCulling>(vertices, triangles, normals)
		{
		}

	protected:
		virtual void GetFragment(triangle_fragment_t& fragment, const uint16_t primitiveIndex)
		{
			fragment.color = Color;
			fragment.material = Material;
		}
	};
}
#endif