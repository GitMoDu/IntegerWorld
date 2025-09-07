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
	template<uint16_t vertexCount, uint16_t triangleCount>
	class AbstractStaticMeshObject : public AbstractTransformObject<vertexCount, triangleCount, mesh_vertex_t, mesh_world_primitive_t>
	{
		static_assert(vertexCount > 0, "vertexCount must be greater than zero.");
		static_assert(triangleCount > 0, "triangleCount must be greater than zero.");

	private:
		using Base = AbstractTransformObject<vertexCount, triangleCount, mesh_vertex_t, mesh_world_primitive_t>;

	protected:
		using Base::Vertices;

	protected:
		// Static sources (can point to PROGMEM on AVR).
		const vertex16_t* VerticesSource = nullptr;
		const triangle_face_t* TrianglesSource = nullptr;
		const vertex16_t* NormalsSource = nullptr;

	protected:
		uint16_t TriangleCount = triangleCount;

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
	/// <typeparam name="hasNormals">Build-time flag indicating a normals buffer is available.</typeparam>
	template<uint16_t vertexCount, uint16_t triangleCount, bool hasNormals>
	class AbstractDynamicMeshObject : public AbstractTransformObject<vertexCount, triangleCount, mesh_vertex_t, mesh_world_primitive_t>
	{
	private:
		using Base = AbstractTransformObject<vertexCount, triangleCount, mesh_vertex_t, mesh_world_primitive_t>;

	protected:
		using Base::Vertices;

	protected:
		// Dynamic sources stored in RAM.
		vertex16_t VerticesSource[vertexCount]{};
		triangle_face_t TrianglesSource[triangleCount]{};
		vertex16_t NormalsSource[triangleCount]{}; // Only used if hasNormals == true.

	protected:
		uint16_t TriangleCount = triangleCount;

	protected:
		/// <summary>
		/// Load object data into working buffers.
		/// </summary>
		virtual void LoadObjectData(const frustum_t& frustum) = 0;

	public:
		AbstractDynamicMeshObject() : Base() {}

		/// <summary>
		/// Override ObjectShade to load object data after base transform.
		/// </summary>
		/// <param name="frustum"></param>
		virtual void ObjectShade(const frustum_t& frustum)
		{
			Base::ObjectShade(frustum);

			LoadObjectData(frustum);
		}

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
	/// Abstract static mesh object that selects pre-bound sources by distance using Levels of Detail (LOD).
	/// - Holds up to LevelsOfDetail entries, each with its own vertex/triangle/normal buffers and counts.
	/// - Selects the first LOD whose squared distance threshold is within the camera-to-object distance.
	/// - Optionally enforces a minimum render distance before any LOD is considered.
	/// When no LOD matches, the mesh is skipped.
	/// </summary>
	/// <typeparam name="vertexCount">Compile-time maximum vertex count, typically from LoD 0.</typeparam>
	/// <typeparam name="triangleCount">Compile-time maximum triangle count, typically from LoD 0.</typeparam>
	/// <typeparam name="LevelsOfDetail">Maximum number of LOD entries that can be registered.</typeparam>
	template<uint16_t vertexCount, uint16_t triangleCount, uint8_t LevelsOfDetail>
	class AbstractStaticMeshLodObject : public AbstractStaticMeshObject<vertexCount, triangleCount>
	{
		static_assert(LevelsOfDetail > 0, "LevelsOfDetail must be greater than zero.");

	private:
		using Base = AbstractStaticMeshObject<vertexCount, triangleCount>;

	protected:
		using Base::WorldPosition;

	private:
		/// <summary>
		/// Single Level-of-Detail entry containing selection threshold and data sources.
		/// </summary>
		struct lod_t
		{
			/// <summary>
			/// Pointer to the vertex source buffer for this LOD (may point to PROGMEM on AVR).
			/// </summary>
			const vertex16_t* VerticesSource;

			/// <summary>
			/// Pointer to the triangle index source buffer for this LOD (may point to PROGMEM on AVR).
			/// </summary>
			const triangle_face_t* TrianglesSource;

			/// <summary>
			/// Optional pointer to per-triangle normal source buffer for this LOD (nullptr to compute on the fly).
			/// </summary>
			const vertex16_t* NormalsSource;

			/// <summary>
			/// Max distance threshold (squared). Above this distance this LOD is not selected.
			/// </summary>
			uint32_t SquareDistanceThreshold;

			/// <summary>
			/// Effective vertex count when this LOD is active.
			/// </summary>
			uint16_t VertexCount;

			/// <summary>
			/// Effective triangle count when this LOD is active.
			/// </summary>
			uint16_t TriangleCount;
		};

	private:
		/// <summary>
		/// Registered LOD entries. Only the first LodCount entries are valid.
		/// </summary>
		lod_t Levels[LevelsOfDetail];

		/// <summary>
		/// Minimum squared distance before any LOD is considered. Below this value the mesh is not rendered.
		/// </summary>
		uint32_t SquareMinDistance = 0;

		/// <summary>
		/// Number of active LOD entries in Levels.
		/// </summary>
		uint8_t LodCount = 0;

		/// <summary>
		/// Index of the currently selected LOD after ObjectShade.
		/// </summary>
		uint8_t CurrentLoD = 0;

	public:
		/// <summary>
		/// Constructs an empty LOD mesh. Levels must be provided via SetSourcesLevelOfDetail before rendering.
		/// </summary>
		AbstractStaticMeshLodObject() : Base() {}

		/// <summary>
		/// Object pass:
		/// - Computes squared distance from camera to object center.
		/// - If distance is below the minimum render distance, disables rendering for this frame.
		/// - Otherwise selects the first LOD whose threshold exceeds the distance, binds its sources,
		///   and sets the effective vertex/triangle counts.
		/// - If no LOD matches, disables rendering for this frame.
		/// </summary>
		/// <param name="frustum">Current view frustum used for camera origin and culling radius.</param>
		virtual void ObjectShade(const frustum_t& frustum)
		{
			Base::ObjectShade(frustum);

			// Vector from camera to object.
			const int32_t dx = int32_t(WorldPosition.x) - frustum.origin.x;
			const int32_t dy = int32_t(WorldPosition.y) - frustum.origin.y;
			const int32_t dz = int32_t(WorldPosition.z) - frustum.origin.z;

			// Determine LOD based on distance to camera.
			const uint32_t squareDistance = uint32_t(dx * dx) + uint32_t(dy * dy) + uint32_t(dz * dz);

			// Only select LOD if beyond the minimum distance.
			if (squareDistance >= SquareMinDistance)
			{
				// Select the first LOD whose threshold exceeds current distance.
				for (uint_fast8_t i = 0; i < LodCount; i++)
				{
					if (squareDistance < Levels[i].SquareDistanceThreshold)
					{
						CurrentLoD = static_cast<uint8_t>(i);
						Base::SetStaticSources(Levels[i].VerticesSource, Levels[i].TrianglesSource);
						Base::SetStaticNormals(Levels[i].NormalsSource);
						Base::VertexCount = Levels[i].VertexCount;
						Base::TriangleCount = Levels[i].TriangleCount;
						return;
					}
				}
			}

			// No LOD matched, skip rendering.
			Base::VertexCount = 0;
			Base::TriangleCount = 0;
		}

		/// <summary>
		/// Sets a minimum render distance. Below this distance (squared) no LOD is selected and the mesh is not rendered.
		/// </summary>
		/// <param name="distance">Distance in world units. The squared value is stored internally.</param>
		void SetRenderDistanceMinimum(const uint16_t distance)
		{
			SquareMinDistance = uint32_t(distance) * distance;
		}

		/// <summary>
		/// Clears all registered LODs and resets selection to zero.
		/// </summary>
		void ClearLevelsOfDetail()
		{
			LodCount = 0;
			CurrentLoD = 0;
		}

		/// <summary>
		/// Returns the number of LOD entries currently registered.
		/// </summary>
		uint8_t GetLevelsOfDetailCount() const
		{
			return LodCount;
		}

		/// <summary>
		/// Registers or updates a Level of Detail entry.
		/// Entries are appended up to LevelsOfDetail and then kept ordered by increasing distance threshold.
		/// </summary>
		/// <param name="maxDistance">Distance in world units until LOD is no longer selected.</param>
		/// <param name="verticesSource">Pointer to the vertex source buffer for this LOD.</param>
		/// <param name="vertexCount">Effective vertex count when this LOD is active.</param>
		/// <param name="trianglesSource">Pointer to the triangle index source buffer for this LOD.</param>
		/// <param name="triangleCount">Effective triangle count when this LOD is active.</param>
		/// <param name="normalsSource">Optional pointer to per-triangle normals for this LOD (nullptr to disable precomputed normals).</param>
		void SetSourcesLevelOfDetail(const uint16_t maxDistance,
			const vertex16_t* verticesSource, const uint16_t lodVertexCount,
			const triangle_face_t* trianglesSource, const uint16_t lodTriangleCount,
			const vertex16_t* normalsSource = nullptr)
		{
			if (LodCount < LevelsOfDetail)
			{
				Levels[LodCount].VerticesSource = verticesSource;
				Levels[LodCount].TrianglesSource = trianglesSource;
				Levels[LodCount].NormalsSource = normalsSource;
				Levels[LodCount].SquareDistanceThreshold = uint32_t(maxDistance) * maxDistance;
				Levels[LodCount].VertexCount = lodTriangleCount;
				Levels[LodCount].TriangleCount = lodTriangleCount;
				LodCount++;

				// Keep thresholds ordered for selection in ObjectShade.
				SortLods();
			}
		}

	private:
		/// <summary>
		/// Stable insertion sort by increasing squared distance threshold over the first LodCount entries.
		/// Ensures deterministic LOD selection when thresholds are equal (preserves relative order).
		/// </summary>
		void SortLods()
		{
			for (uint_fast8_t i = 1; i < LodCount; ++i)
			{
				lod_t key = Levels[i];
				int16_t j = int16_t(i) - 1;
				while (j >= 0 && Levels[j].SquareDistanceThreshold > key.SquareDistanceThreshold)
				{
					Levels[j + 1] = Levels[j];
					--j;
				}
				Levels[j + 1] = key;
			}
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

			const auto triangle = BaseMeshObject::GetTriangle(primitiveIndex);

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
				const auto triangle = BaseMeshObject::GetTriangle(primitiveIndex);

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
					break;
				case IntegerWorld::MeshCullingEnum::FrontfaceCullling:
					if (signedArea > 0)
					{
						Primitives[primitiveIndex].z = AverageApproximate(Vertices[triangle.v1].z, Vertices[triangle.v2].z, Vertices[triangle.v3].z);
						return false;
					}
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

			const auto triangle = BaseMeshObject::GetTriangle(primitiveIndex);
			const auto primitive = Primitives[primitiveIndex];

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
		AbstractStaticMeshObject<vertexCount, triangleCount>, frustumCulling, meshCulling>
	{
	private:
		using Base = TemplateMeshWorldObject<
			AbstractStaticMeshObject<vertexCount, triangleCount>, frustumCulling, meshCulling>;

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
	/// Static (ROM-backed) mesh world object with Level of Detail (LOD) selection.
	/// - Wraps AbstractStaticMeshLodObject and runs the world pipeline.
	/// - LODs can be registered incrementally and will be selected by squared distance at render time.
	/// </summary>
	/// <typeparam name="vertexCount">Number of vertices in working buffers.</typeparam>
	/// <typeparam name="triangleCount">Number of triangles in working buffers.</typeparam>
	/// <typeparam name="LevelsOfDetail">Maximum number of LODs that can be registered.</typeparam>
	/// <typeparam name="frustumCulling">Frustum culling mode.</typeparam>
	/// <typeparam name="meshCulling">Mesh culling mode.</typeparam>
	template<uint16_t vertexCount, uint16_t triangleCount, uint8_t LevelsOfDetail,
		FrustumCullingEnum frustumCulling = FrustumCullingEnum::ObjectCulling,
		MeshCullingEnum meshCulling = MeshCullingEnum::BackfaceCullling>
	class StaticMeshLodObject : public TemplateMeshWorldObject<
		AbstractStaticMeshLodObject<vertexCount, triangleCount, LevelsOfDetail>, frustumCulling, meshCulling>
	{
	private:
		using Base = TemplateMeshWorldObject<
			AbstractStaticMeshLodObject<vertexCount, triangleCount, LevelsOfDetail>, frustumCulling, meshCulling>;

	public:
		/// <summary>
		/// Constructs an empty LOD mesh. Use SetSourcesLevelOfDetail to register one or more LODs.
		/// </summary>
		StaticMeshLodObject()
			: Base()
		{
		}

		/// <summary>
		/// Convenience constructor that registers a single default LOD entry.
		/// </summary>
		/// <param name="maxDistance">Distance in world units until LOD is no longer selected.</param>
		/// <param name="verticesSource">Pointer to vertex source for this LOD.</param>
		/// <param name="vertexCount">Effective vertex count for this LOD.</param>
		/// <param name="trianglesSource">Pointer to triangle index source for this LOD.</param>
		/// <param name="triangleCount">Effective triangle count for this LOD.</param>
		/// <param name="normalsSource">Optional pointer to per-triangle normals for this LOD.</param>
		StaticMeshLodObject(const uint16_t maxDistance,
			const vertex16_t* verticesSource, const uint16_t lodVertexCount,
			const triangle_face_t* trianglesSource, const uint16_t lodTriangleCount,
			const vertex16_t* normalsSource = nullptr)
			: Base()
		{
			Base::SetSourcesLevelOfDetail(maxDistance, verticesSource, lodVertexCount, trianglesSource, lodTriangleCount, normalsSource);
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
		bool hasNormals = false,
		FrustumCullingEnum frustumCulling = FrustumCullingEnum::ObjectCulling,
		MeshCullingEnum meshCulling = MeshCullingEnum::BackfaceCullling>
	using DynamicMeshObject = TemplateMeshWorldObject<
		AbstractDynamicMeshObject<vertexCount, triangleCount, hasNormals>,
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
		bool hasNormals = false,
		FrustumCullingEnum frustumCulling = FrustumCullingEnum::ObjectCulling,
		MeshCullingEnum meshCulling = MeshCullingEnum::BackfaceCullling>
	class DynamicMeshSingleColorSingleMaterialObject : public DynamicMeshObject<vertexCount, triangleCount, hasNormals, frustumCulling, meshCulling>
	{
	public:
		// Shared color/material for all fragments.
		Rgb8::color_t Color = Rgb8::WHITE;
		material_t Material{ UFRACTION8_1X, 0, 0, 0 };

	public:
		DynamicMeshSingleColorSingleMaterialObject()
			: DynamicMeshObject<vertexCount, triangleCount, hasNormals, frustumCulling, meshCulling>()
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
	/// Static LOD mesh with a single color and material.
	/// Inherits LOD distance selection and mesh pipeline behavior, supplying a shared color/material for all fragments.
	/// </summary>
	/// <typeparam name="vertexCount">Number of vertices in working buffers.</typeparam>
	/// <typeparam name="triangleCount">Number of triangles in working buffers.</typeparam>
	/// <typeparam name="LevelsOfDetail">Maximum number of LODs that can be registered.</typeparam>
	/// <typeparam name="frustumCulling">Frustum culling mode.</typeparam>
	/// <typeparam name="meshCulling">Mesh culling mode.</typeparam>
	template<uint16_t vertexCount, uint16_t triangleCount, uint8_t LevelsOfDetail,
		FrustumCullingEnum frustumCulling = FrustumCullingEnum::ObjectCulling,
		MeshCullingEnum meshCulling = MeshCullingEnum::BackfaceCullling>
	class StaticMeshLodSingleColorSingleMaterialObject : public StaticMeshLodObject<vertexCount, triangleCount, LevelsOfDetail, frustumCulling, meshCulling>
	{
	public:
		// Shared color/material for all fragments.
		Rgb8::color_t Color = Rgb8::WHITE;
		material_t Material{ UFRACTION8_1X, 0, 0, 0 };

	public:
		/// <summary>
		/// Constructs an empty LOD mesh with shared color/material. Use SetSourcesLevelOfDetail to add LODs.
		/// </summary>
		StaticMeshLodSingleColorSingleMaterialObject()
			: StaticMeshLodObject<vertexCount, triangleCount, LevelsOfDetail, frustumCulling, meshCulling>()
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