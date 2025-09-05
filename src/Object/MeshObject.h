#ifndef _INTEGER_WORLD_MESH_OBJECT_h
#define _INTEGER_WORLD_MESH_OBJECT_h

#include "AbstractObject.h"

namespace IntegerWorld
{

	/// <summary>
	/// Mesh culling modes applied after projection based on face orientation.
	/// </summary>
	enum class MeshCullingEnum : uint8_t
	{
		/// <summary>
		/// Triangles whose normal is facing away from the camera are not drawn.
		/// </summary>
		BackfaceCullling,

		/// <summary>
		/// All triangles are drawn, regardless of orientation.
		/// </summary>
		NoCullling,

		/// <summary>
		/// Triangles whose normal is facing towards the camera are not drawn.
		/// </summary>
		FrontfaceCullling
	};

	/// <summary>
	/// Abstract static mesh object with a fixed number of vertices and triangles.
	/// Provides loader utilities (vertex/triangle/normal access) with AVR PROGMEM support.
	/// </summary>
	/// <typeparam name="vertexCount">The number of vertices in the mesh.</typeparam>
	/// <typeparam name="triangleCount">The number of triangles in the mesh.</typeparam>
	/// <typeparam name="vertex_t">Vertex type stored in the working buffer (default: mesh_vertex_t).</typeparam>
	/// <typeparam name="primitive_t">Primitive type for world data (default: mesh_world_primitive_t).</typeparam>
	template<uint16_t vertexCount, uint16_t triangleCount,
		typename vertex_t = mesh_vertex_t,
		typename primitive_t = mesh_world_primitive_t>
	class AbstractStaticMeshObject : public AbstractTransformObject<vertexCount, triangleCount, vertex_t, primitive_t>
	{
	private:
		using Base = AbstractTransformObject<vertexCount, triangleCount, vertex_t, primitive_t>;

	protected:
		using Base::Vertices;

	protected:
		// Static sources (can point to PROGMEM on AVR).
		const vertex16_t* VerticesSource = nullptr;
		const triangle_face_t* TrianglesSource = nullptr;
		const vertex16_t* NormalsSource = nullptr;

	protected:
		static constexpr uint16_t TriangleCount = triangleCount;

	public:
		AbstractStaticMeshObject() : Base() {}

		/// <summary>
		/// Binds static vertex and triangle arrays as data sources.
		/// </summary>
		/// <param name="vertices">Pointer to vertex array of size vertexCount.</param>
		/// <param name="triangles">Pointer to triangle index array of size triangleCount.</param>
		void SetStaticSources(const vertex16_t vertices[vertexCount], const triangle_face_t triangles[triangleCount])
		{
			VerticesSource = vertices;
			TrianglesSource = triangles;
		}

		/// <summary>
		/// Binds optional per-triangle normals source (one normal per triangle).
		/// If not set, normals are computed per-primitive.
		/// </summary>
		/// <param name="normals">Pointer to normals array of size triangleCount (or nullptr).</param>
		void SetStaticNormals(const vertex16_t normals[triangleCount])
		{
			NormalsSource = normals;
		}

	protected:
		/// <summary>
		/// Returns true when a normal source buffer is bound.
		/// </summary>
		bool HasNormals() const
		{
			return NormalsSource != nullptr;
		}

#if defined(ARDUINO_ARCH_AVR)
		/// <summary>
		/// Reads a triangle face from PROGMEM on AVR; otherwise returns by value.
		/// </summary>
		const triangle_face_t GetTriangle(const uint16_t index) const
		{
			return triangle_face_t
			{
				(uint16_t)pgm_read_word(&TrianglesSource[index].v1),
				(uint16_t)pgm_read_word(&TrianglesSource[index].v2),
				(uint16_t)pgm_read_word(&TrianglesSource[index].v3)
			};
		}

		/// <summary>
		/// Reads a precomputed triangle normal from PROGMEM on AVR; otherwise returns by value.
		/// </summary>
		const vertex16_t GetNormal(const uint16_t index) const
		{
			return vertex16_t
			{
				(int16_t)pgm_read_word(&NormalsSource[index].x),
				(int16_t)pgm_read_word(&NormalsSource[index].y),
				(int16_t)pgm_read_word(&NormalsSource[index].z)
			};
		}
#else
		/// <summary>
		/// Returns a reference to a triangle face from RAM.
		/// </summary>
		const triangle_face_t& GetTriangle(const uint16_t index) const
		{
			return TrianglesSource[index];
		}

		/// <summary>
		/// Returns a reference to a precomputed triangle normal from RAM.
		/// </summary>
		const vertex16_t& GetNormal(const uint16_t index) const
		{
			return NormalsSource[index];
		}
#endif

		/// <summary>
		/// Loads a source vertex into the working vertex buffer.
		/// On AVR reads from PROGMEM, on other platforms copies directly.
		/// </summary>
		void LoadVertex(const uint16_t index)
		{
#if defined(ARDUINO_ARCH_AVR)
			Vertices[index].x = (int16_t)pgm_read_word(&VerticesSource[index].x);
			Vertices[index].y = (int16_t)pgm_read_word(&VerticesSource[index].y);
			Vertices[index].z = (int16_t)pgm_read_word(&VerticesSource[index].z);
#else
			Vertices[index].x = VerticesSource[index].x;
			Vertices[index].y = VerticesSource[index].y;
			Vertices[index].z = VerticesSource[index].z;
#endif
		}
	};

	/// <summary>
	/// Abstract mesh object with dynamic (mutable) vertices/triangles and optional normals.
	/// Provides an in-RAM source and loader API identical to the static variant.
	/// Overriding classes can modify the source buffers and effective vertex count directly.
	/// </summary>
	/// <typeparam name="vertexCount">Max number of vertices.</typeparam>
	/// <typeparam name="triangleCount">Max number of triangles.</typeparam>
	/// <typeparam name="vertex_t">Vertex type stored in the working buffer.</typeparam>
	/// <typeparam name="primitive_t">Primitive type for world data.</typeparam>
	/// <typeparam name="hasNormals">Build-time flag indicating a normals buffer is available.</typeparam>
	template<uint16_t vertexCount, uint16_t triangleCount,
		typename vertex_t = mesh_vertex_t,
		typename primitive_t = mesh_world_primitive_t,
		bool hasNormals = false>
	class AbstractDynamicMeshObject : public AbstractTransformObject<vertexCount, triangleCount, vertex_t, primitive_t>
	{
	private:
		using Base = AbstractTransformObject<vertexCount, triangleCount, vertex_t, primitive_t>;

	protected:
		using Base::Vertices;

	protected:
		// Dynamic sources stored in RAM.
		vertex16_t VerticesSource[vertexCount]{};
		triangle_face_t TrianglesSource[triangleCount]{};
		vertex16_t NormalsSource[hasNormals * triangleCount]{};

	protected:
		static constexpr uint16_t TriangleCount = triangleCount;

	public:
		AbstractDynamicMeshObject() : Base() {}

	protected:
		/// <summary>
		/// Returns true when the dynamic normal buffer is compiled in (hasNormals == true).
		/// </summary>
		bool HasNormals() const
		{
			return hasNormals;
		}

		/// <summary>
		/// Returns a reference to a triangle face from the dynamic source.
		/// </summary>
		const triangle_face_t& GetTriangle(const uint16_t index) const
		{
			return TrianglesSource[index];
		}

		/// <summary>
		/// Returns a reference to a precomputed triangle normal from the dynamic source.
		/// </summary>
		const vertex16_t& GetNormal(const uint16_t index) const
		{
			return NormalsSource[index];
		}

		/// <summary>
		/// Loads a source vertex into the working vertex buffer.
		/// </summary>
		void LoadVertex(const uint16_t index)
		{
			Vertices[index].x = VerticesSource[index].x;
			Vertices[index].y = VerticesSource[index].y;
			Vertices[index].z = VerticesSource[index].z;
		}
	};

	/// <summary>
	/// Extends a base mesh loader (static/dynamic) and implements the world-space mesh pipeline:
	/// - Object/primitive frustum culling
	/// - Optional use of precomputed triangle normals (rotated) or on-the-fly normal calculation
	/// - Backface/frontface/no culling in screen space
	/// - Fragment collection and shading API
	/// This mirrors the pattern used by Edge objects and enables zero-overhead code reuse.
	/// </summary>
	/// <typeparam name="BaseMeshObject">AbstractStaticMeshObject or AbstractDynamicMeshObject.</typeparam>
	/// <typeparam name="frustumCulling">Frustum culling mode (default: ObjectCulling).</typeparam>
	/// <typeparam name="meshCulling">Mesh culling mode (default: BackfaceCullling).</typeparam>
	template<typename BaseMeshObject,
		FrustumCullingEnum frustumCulling = FrustumCullingEnum::ObjectCulling,
		MeshCullingEnum meshCulling = MeshCullingEnum::BackfaceCullling>
	class TemplateMeshWorldObject : public BaseMeshObject
	{
	public:
		using BaseMeshObject::SceneShader;

	protected:
		using BaseMeshObject::Vertices;
		using BaseMeshObject::Primitives;
		using BaseMeshObject::WorldPosition;
		using BaseMeshObject::VertexCount;
		using BaseMeshObject::TriangleCount;
		using BaseMeshObject::MeshTransform;

	public:
		/// <summary>
		/// Triangle fragment shader (optional). If null, no fragments are shaded.
		/// </summary>
		IFragmentShader<triangle_fragment_t>* FragmentShader = nullptr;

	protected:
		// Reusable triangle fragment.
		triangle_fragment_t TriangleFragment{};

	public:
		TemplateMeshWorldObject() : BaseMeshObject() {}

		/// <summary>
		/// Object pass:
		/// - Applies object-level frustum culling and initializes primitive z flags.
		/// - Loads source vertices into the working buffer if the object is inside the frustum.
		/// </summary>
		virtual void ObjectShade(const frustum_t& frustum)
		{
			BaseMeshObject::ObjectShade(frustum);

			int16_t zFlag = 0;

			switch (frustumCulling)
			{
			case FrustumCullingEnum::ObjectAndPrimitiveCulling:
			case FrustumCullingEnum::ObjectCulling:
				if (!frustum.IsPointInside(WorldPosition))
				{
					zFlag = -VERTEX16_UNIT;
				}
				for (uint_fast16_t i = 0; i < TriangleCount; i++)
				{
					Primitives[i].z = zFlag;
				}
				break;
			case FrustumCullingEnum::PrimitiveCulling:
				for (uint_fast16_t i = 0; i < TriangleCount; i++)
				{
					Primitives[i].z = 0;
				}
			case FrustumCullingEnum::NoCulling:
			default:
				break;
			};

			if (zFlag >= 0)
			{
				for (uint_fast16_t i = 0; i < VertexCount; i++)
				{
					BaseMeshObject::LoadVertex((uint16_t)i);
				}
			}
		}

		/// <summary>
		/// World pass:
		/// - Computes per-primitive world position (triangle center).
		/// - Optionally culls primitives against the frustum.
		/// - Computes or rotates the triangle world normal.
		/// </summary>
		/// <returns>false to continue processing; true when no more primitives are available.</returns>
		virtual bool WorldShade(const frustum_t& frustum, const uint16_t primitiveIndex)
		{
			if (primitiveIndex >= TriangleCount)
				return true;

			// Check for world space frustum culling flag first.
			if (frustumCulling != FrustumCullingEnum::NoCulling
				&& Primitives[primitiveIndex].z < 0)
				return false;

			const auto& triangle = BaseMeshObject::GetTriangle(primitiveIndex);

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
			case FrustumCullingEnum::NoCulling:
			default:
				Primitives[primitiveIndex].z = 0;
				break;
			};

			if (BaseMeshObject::HasNormals())
			{
				// Rotate precomputed normal.
				Primitives[primitiveIndex].worldNormal = BaseMeshObject::GetNormal(primitiveIndex);
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

			if (Primitives[primitiveIndex].z >= 0)
			{
				const auto& triangle = BaseMeshObject::GetTriangle(primitiveIndex);

				switch (meshCulling)
				{
				case IntegerWorld::MeshCullingEnum::NoCullling:
					Primitives[primitiveIndex].z = AverageApproximate(Vertices[triangle.v1].z, Vertices[triangle.v2].z, Vertices[triangle.v3].z);
					return false;
				default:
					break;
				}

				// Back/Front face culling after projection.
				const int32_t signedArea = (int32_t(Vertices[triangle.v2].x - Vertices[triangle.v1].x)
					* (Vertices[triangle.v3].y - Vertices[triangle.v1].y))
					- (int32_t(Vertices[triangle.v2].y - Vertices[triangle.v1].y)
						* (Vertices[triangle.v3].x - Vertices[triangle.v1].x));

				switch (meshCulling)
				{
				case IntegerWorld::MeshCullingEnum::BackfaceCullling:
					if (signedArea < 0)
					{
						Primitives[primitiveIndex].z = AverageApproximate(Vertices[triangle.v1].z, Vertices[triangle.v2].z, Vertices[triangle.v3].z);
						return false;
					}
				case IntegerWorld::MeshCullingEnum::FrontfaceCullling:
					if (signedArea > 0)
					{
						Primitives[primitiveIndex].z = AverageApproximate(Vertices[triangle.v1].z, Vertices[triangle.v2].z, Vertices[triangle.v3].z);
						return false;
					}
					break;
					break;
				default:
					break;
				}

				// Culled.
				Primitives[primitiveIndex].z = -VERTEX16_UNIT;
			}

			return false;
		}

		/// <summary>
		/// Collects primitives that passed culling, pushing them to the fragment collector
		/// with their depth key for later sorting.
		/// </summary>
		void FragmentCollect(FragmentCollector& fragmentCollector) final
		{
			for (uint_fast16_t i = 0; i < TriangleCount; i++)
			{
				if (Primitives[i].z >= 0)
				{
					fragmentCollector.AddFragment(i, Primitives[i].z);
				}
			}
		}

		/// <summary>
		/// Produces a triangle fragment for the rasterizer and calls the fragment shader.
		/// </summary>
		/// <param name="rasterizer">Window rasterizer to receive the shaded triangle.</param>
		/// <param name="primitiveIndex">Triangle index.</param>
		virtual void FragmentShade(WindowRasterizer& rasterizer, const uint16_t primitiveIndex)
		{
			if (FragmentShader == nullptr)
				return;

			const auto& triangle = GetTriangle(primitiveIndex);
			const auto& primitive = Primitives[primitiveIndex];

			TriangleFragment.normalWorld = primitive.worldNormal;
			TriangleFragment.world = primitive.worldPosition;

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

	protected:
		/// <summary>
		/// Default fragment initializer. Override to provide color/material per primitive.
		/// </summary>
		virtual void GetFragment(triangle_fragment_t& fragment, const uint16_t primitiveIndex)
		{
			fragment.color = Rgb8::WHITE;
			fragment.material.Emissive = 0;
			fragment.material.Diffuse = UFRACTION8_1X;
			fragment.material.Specular = 0;
			fragment.material.Metallic = 0;
		}
	};

	/// <summary>
	/// Static (ROM-backed) mesh world object using the static loader.
	/// Supplies sources via SetStaticSources/SetStaticNormals and runs the world pipeline.
	/// </summary>
	/// <typeparam name="vertexCount">Number of vertices.</typeparam>
	/// <typeparam name="triangleCount">Number of triangles.</typeparam>
	/// <typeparam name="frustumCulling">Frustum culling mode.</typeparam>
	/// <typeparam name="meshCulling">Mesh culling mode.</typeparam>
	template<uint16_t vertexCount, uint16_t triangleCount,
		FrustumCullingEnum frustumCulling = FrustumCullingEnum::ObjectCulling,
		MeshCullingEnum meshCulling = MeshCullingEnum::BackfaceCullling>
	class StaticMeshObject : public TemplateMeshWorldObject<
		AbstractStaticMeshObject<vertexCount, triangleCount, mesh_vertex_t, mesh_world_primitive_t>, frustumCulling, meshCulling>
	{
	private:
		using Base = TemplateMeshWorldObject<
			AbstractStaticMeshObject<vertexCount, triangleCount, mesh_vertex_t, mesh_world_primitive_t>, frustumCulling, meshCulling>;

	public:
		/// <summary>
		/// Constructs a static mesh object using the provided vertex, triangle, and optional normal sources.
		/// </summary>
		StaticMeshObject(const vertex16_t vertices[vertexCount], const triangle_face_t triangles[triangleCount], const vertex16_t normals[triangleCount] = nullptr)
			: Base()
		{
			Base::SetStaticSources(vertices, triangles);
			Base::SetStaticNormals(normals);
		}
	};

	/// <summary>
	/// Dynamic (RAM-backed) mesh world object alias using the dynamic loader.
	/// Overriding classes can modify the vertex/triangle/normal buffers directly.
	/// </summary>
	/// <typeparam name="vertexCount">Max number of vertices.</typeparam>
	/// <typeparam name="triangleCount">Max number of triangles.</typeparam>
	/// <typeparam name="frustumCulling">Frustum culling mode.</typeparam>
	/// <typeparam name="meshCulling">Mesh culling mode.</typeparam>
	/// <typeparam name="hasNormals">If true, a normals buffer is available and used.</typeparam>
	template<uint16_t vertexCount, uint16_t triangleCount,
		FrustumCullingEnum frustumCulling = FrustumCullingEnum::ObjectCulling,
		MeshCullingEnum meshCulling = MeshCullingEnum::BackfaceCullling,
		bool hasNormals = false>
	using DynamicMeshObject = TemplateMeshWorldObject<
		AbstractDynamicMeshObject<vertexCount, triangleCount, mesh_vertex_t, mesh_world_primitive_t, hasNormals>,
		frustumCulling, meshCulling>;

	/// <summary>
	/// Static mesh object with a single color and material, supporting frustum and mesh culling.
	/// </summary>
	/// <typeparam name="vertexCount">Number of vertices.</typeparam>
	/// <typeparam name="triangleCount">Number of triangles.</typeparam>
	/// <typeparam name="frustumCulling">Frustum culling mode.</typeparam>
	/// <typeparam name="meshCulling">Mesh culling mode.</typeparam>
	template<uint16_t vertexCount, uint16_t triangleCount,
		FrustumCullingEnum frustumCulling = FrustumCullingEnum::ObjectCulling,
		MeshCullingEnum meshCulling = MeshCullingEnum::BackfaceCullling>
	class StaticMeshSingleColorSingleMaterialObject : public StaticMeshObject<vertexCount, triangleCount, frustumCulling, meshCulling>
	{
	public:
		// Shared color/material for all fragments.
		Rgb8::color_t Color = Rgb8::WHITE;
		material_t Material{ UFRACTION8_1X, 0, 0, 0 };

	public:
		/// <summary>
		/// Constructs a static mesh object using the provided sources and sets default color/material.
		/// </summary>
		StaticMeshSingleColorSingleMaterialObject(
			const vertex16_t vertices[vertexCount],
			const triangle_face_t triangles[triangleCount],
			const vertex16_t normals[triangleCount] = nullptr)
			: StaticMeshObject<vertexCount, triangleCount, frustumCulling, meshCulling>(vertices, triangles, normals)
		{
		}

	protected:
		/// <summary>
		/// Supplies the shared color/material to the fragment.
		/// </summary>
		virtual void GetFragment(triangle_fragment_t& fragment, const uint16_t primitiveIndex)
		{
			fragment.color = Color;
			fragment.material = Material;
		}
	};

	/// <summary>
	/// Dynamic mesh object with a single color and material, supporting frustum and mesh culling.
	/// Overriding classes can modify the vertex/triangle/normal buffers directly.
	/// </summary>
	/// <typeparam name="vertexCount">Max number of vertices.</typeparam>
	/// <typeparam name="triangleCount">Max number of triangles.</typeparam>
	/// <typeparam name="frustumCulling">Frustum culling mode.</typeparam>
	/// <typeparam name="meshCulling">Mesh culling mode.</typeparam>
	/// <typeparam name="hasNormals">If true, the dynamic normals buffer is available and used.</typeparam>
	template<uint16_t vertexCount, uint16_t triangleCount,
		FrustumCullingEnum frustumCulling = FrustumCullingEnum::ObjectCulling,
		MeshCullingEnum meshCulling = MeshCullingEnum::BackfaceCullling,
		bool hasNormals = false>
	class DynamicMeshSingleColorSingleMaterialObject : public DynamicMeshObject<vertexCount, triangleCount, frustumCulling, meshCulling, hasNormals>
	{
	public:
		// Shared color/material for all fragments.
		Rgb8::color_t Color = Rgb8::WHITE;
		material_t Material{ UFRACTION8_1X, 0, 0, 0 };

	public:
		DynamicMeshSingleColorSingleMaterialObject()
			: DynamicMeshObject<vertexCount, triangleCount, frustumCulling, meshCulling, hasNormals>()
		{
		}

	protected:
		/// <summary>
		/// Supplies the shared color/material to the fragment.
		/// </summary>
		virtual void GetFragment(triangle_fragment_t& fragment, const uint16_t primitiveIndex)
		{
			fragment.color = Color;
			fragment.material = Material;
		}
	};

}
#endif