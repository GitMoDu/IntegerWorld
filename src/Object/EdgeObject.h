#ifndef _INTEGER_WORLD_EDGE_OBJECT_h
#define _INTEGER_WORLD_EDGE_OBJECT_h

#include "PointObject.h"

namespace IntegerWorld
{
	/// <summary>
	/// Specifies the modes for drawing or culling edges based on their position relative to the object's origin.
	/// Applied after projection using the edge endpoints and the object's screen-space position.
	/// </summary>
	enum class EdgeDrawModeEnum
	{
		/// <summary>
		/// Draw all edges, regardless of orientation or position.
		/// </summary>
		NoCulling,

		/// <summary>
		/// Cull edges whose center point Z is behind the object origin (simple back-face culling for edges).
		/// Only edges with center point Z greater than the object's Z are drawn.
		/// </summary>
		CullCenterZBehind,

		/// <summary>
		/// Cull edges whose center point Z is in front the object origin (simple back-face culling for edges).
		/// Only edges with center point Z smaller than the object's Z are drawn.
		/// </summary>
		CullCenterZFront,

		/// <summary>
		/// Draw edges only if at least one endpoint is in front of the object origin (partial visibility).
		/// </summary>
		CullAllBehind,

		/// <summary>
		/// Draw edges only if both endpoints are in front of the object origin (strict visibility).
		/// </summary>
		CullAnyBehind,
	};

	/// <summary>
	/// Edge vertex used in the working buffer.
	/// </summary>
	struct edge_vertex_t : base_vertex_t
	{
	};

	/// <summary>
	/// Per-edge primitive data produced during shading/pipeline passes.
	/// </summary>
	struct edge_primitive_t : base_primitive_t
	{
		/// <summary>Edge world position (average of endpoints in world space).</summary>
		vertex16_t worldPosition;
	};

	/// <summary>
	/// Abstract static edge object with a fixed number of vertices and edges.
	/// Provides loader utilities (vertex/edge access) with AVR PROGMEM support.
	/// </summary>
	/// <typeparam name="vertexCount">The number of vertices in the object.</typeparam>
	/// <typeparam name="edgeCount">The number of edges in the object.</typeparam>
	template<uint16_t vertexCount, uint16_t edgeCount>
	class AbstractStaticEdgeObject : public AbstractTransformObject<vertexCount, edgeCount, edge_vertex_t, edge_primitive_t>
	{
	private:
		using Base = AbstractTransformObject<vertexCount, edgeCount, edge_vertex_t, edge_primitive_t>;

	protected:
		using Base::Vertices;

	protected:
		// Static sources (can point to PROGMEM on AVR).
		const vertex16_t* VerticesSource = nullptr;
		const edge_line_t* EdgesSource = nullptr;

	protected:
		static constexpr uint16_t EdgeCount = edgeCount;

	public:
		AbstractStaticEdgeObject() : Base() {}

		/// <summary>
		/// Binds static vertex and edge arrays as data sources.
		/// </summary>
		void SetStaticSources(const vertex16_t vertices[vertexCount], const edge_line_t edges[edgeCount])
		{
			VerticesSource = vertices;
			EdgesSource = edges;
		}

	protected:
#if defined(ARDUINO_ARCH_AVR)
		/// <summary>
		/// Reads an edge from PROGMEM on AVR; otherwise returns by value.
		/// </summary>
		const edge_line_t GetEdge(const uint16_t index) const
		{
			return edge_line_t
			{
				(uint16_t)pgm_read_word(&EdgesSource[index].start),
				(uint16_t)pgm_read_word(&EdgesSource[index].end)
			};
		}
#else
		/// <summary>
		/// Returns a reference to an edge from RAM.
		/// </summary>
		const edge_line_t& GetEdge(const uint16_t index) const
		{
			return EdgesSource[index];
		}
#endif

		/// <summary>
		/// Loads a source vertex into the working vertex buffer.
		/// On AVR reads from PROGMEM, on other platforms copies directly.
		/// </summary>
		void LoadVertex(const uint16_t index)
		{
#if defined(ARDUINO_ARCH_AVR)
			Vertices[index].x = pgm_read_word(&VerticesSource[index].x);
			Vertices[index].y = pgm_read_word(&VerticesSource[index].y);
			Vertices[index].z = pgm_read_word(&VerticesSource[index].z);
#else
			Vertices[index].x = VerticesSource[index].x;
			Vertices[index].y = VerticesSource[index].y;
			Vertices[index].z = VerticesSource[index].z;
#endif
		}
	};

	/// <summary>
	/// Abstract object with dynamic edges and vertices.
	/// Provides an in-RAM source and loader API identical to the static variant.
	/// </summary>
	/// <typeparam name="vertexCount">The number of vertices in the object.</typeparam>
	/// <typeparam name="edgeCount">The number of edges in the object.</typeparam>
	template<uint16_t vertexCount, uint16_t edgeCount>
	class AbstractDynamicEdgeObject : public AbstractTransformObject<vertexCount, edgeCount, edge_vertex_t, edge_primitive_t>
	{
	private:
		using Base = AbstractTransformObject<vertexCount, edgeCount, edge_vertex_t, edge_primitive_t>;

	protected:
		using Base::Vertices;

	protected:
		// Dynamic sources stored in RAM.
		vertex16_t VerticesSource[vertexCount]{};
		edge_line_t EdgesSource[edgeCount]{};

	protected:
		uint16_t EdgeCount;

	public:
		AbstractDynamicEdgeObject()
			: Base()
			, EdgeCount(edgeCount)
		{
		}

	protected:
		/// <summary>
		/// Returns a reference to an edge from the dynamic source.
		/// </summary>
		const edge_line_t& GetEdge(const uint16_t index) const
		{
			return EdgesSource[index];
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
	/// Extends a base edge object and manages edge primitives, their transformations, and rendering logic.
	/// Implements frustum culling, edge drawing modes, fragment collection and shading.
	/// Parameterized Base Class Pattern allows code-reuse for static and dynamic edge objects with zero overhead.
	/// </summary>
	/// <typeparam name="BaseEdgeObject">AbstractDynamicEdgeObject or AbstractStaticEdgeObject.</typeparam>
	/// <typeparam name="frustumCulling">Specifies the frustum culling mode (default: ObjectCulling).</typeparam>
	/// <typeparam name="edgeDrawMode">Specifies the edge drawing mode (default: NoCulling).</typeparam>
	template<typename BaseEdgeObject,
		FrustumCullingEnum frustumCulling = FrustumCullingEnum::ObjectCulling,
		EdgeDrawModeEnum edgeDrawMode = EdgeDrawModeEnum::NoCulling>
	class TemplateEdgeObject : public BaseEdgeObject
	{
	public:
		using BaseEdgeObject::SceneShader;

	protected:
		using BaseEdgeObject::Vertices;
		using BaseEdgeObject::Primitives;
		using BaseEdgeObject::WorldPosition;
		using BaseEdgeObject::VertexCount;
		using BaseEdgeObject::EdgeCount;

	public:
		/// <summary>
		/// Edge fragment shader (optional). If null, no fragments are shaded.
		/// </summary>
		IFragmentShader<edge_fragment_t>* FragmentShader = nullptr;

	protected:
		/// <summary>
		/// Screen space object position used for culling decisions.
		/// </summary>
		vertex16_t ScreenPosition{};

	private:
		// Reusable edge fragment for edge shading.
		edge_fragment_t EdgeFragment{};

	public:
		TemplateEdgeObject() : BaseEdgeObject() {}

		/// <summary>
		/// Object pass:
		/// - Applies object-level frustum culling and initializes primitive z flags.
		/// - Loads source vertices into the working buffer if the object is inside the frustum.
		/// - Caches object ScreenPosition for later culling.
		/// </summary>
		virtual void ObjectShade(const frustum_t& frustum)
		{
			BaseEdgeObject::ObjectShade(frustum);

			ScreenPosition = WorldPosition;

			int16_t zFlag = 0;

			switch (frustumCulling)
			{
			case FrustumCullingEnum::ObjectAndPrimitiveCulling:
			case FrustumCullingEnum::ObjectCulling:
				if (!frustum.IsPointInside(WorldPosition))
				{
					zFlag = -VERTEX16_UNIT;
				}
				for (uint_fast16_t i = 0; i < EdgeCount; i++)
				{
					Primitives[i].z = zFlag;
				}
				break;
			case FrustumCullingEnum::PrimitiveCulling:
				for (uint_fast16_t i = 0; i < EdgeCount; i++)
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
					BaseEdgeObject::LoadVertex(i);
				}
			}
		}

		/// <summary>
		/// World pass:
		/// - Optionally culls primitives against the frustum (object/primitive modes).
		/// - Computes primitive world position as the average of both edge endpoints.
		/// </summary>
		virtual bool WorldShade(const frustum_t& frustum, const uint16_t primitiveIndex)
		{
			if (primitiveIndex >= EdgeCount)
				return true;

			auto edge = BaseEdgeObject::GetEdge(primitiveIndex);

			switch (frustumCulling)
			{
			case FrustumCullingEnum::ObjectAndPrimitiveCulling:
			case FrustumCullingEnum::PrimitiveCulling:
				if (Primitives[primitiveIndex].z >= 0)
				{
					Primitives[primitiveIndex].z = -VERTEX16_UNIT * !frustum.IsPointInside(Primitives[primitiveIndex].worldPosition);
				}
				break;
			case FrustumCullingEnum::NoCulling:
			default:
				Primitives[primitiveIndex].z = 0;
				break;
			};

			if (Primitives[primitiveIndex].z >= 0)
			{
				// Cache primitive world position as average of both edge vertices.
				Primitives[primitiveIndex].worldPosition.x = (int16_t)SignedRightShift(((int32_t)Vertices[edge.start].x + Vertices[edge.end].x), 1);
				Primitives[primitiveIndex].worldPosition.y = (int16_t)SignedRightShift(((int32_t)Vertices[edge.start].y + Vertices[edge.end].y), 1);
				Primitives[primitiveIndex].worldPosition.z = (int16_t)SignedRightShift(((int32_t)Vertices[edge.start].z + Vertices[edge.end].z), 1);
			}

			return false;
		}

		/// <summary>
		/// Camera pass:
		/// - Transforms ScreenPosition with the camera.
		/// - Delegates per-vertex transform to the base object.
		/// </summary>
		virtual bool CameraTransform(const transform16_camera_t& transform, const uint16_t vertexIndex)
		{
			if (vertexIndex == 0)
				ApplyCameraTransform(transform, ScreenPosition);

			return BaseEdgeObject::CameraTransform(transform, vertexIndex);
		}

		/// <summary>
		/// Projection pass:
		/// - Projects ScreenPosition once for the object.
		/// - Delegates per-vertex projection to the base object.
		/// </summary>
		virtual bool ScreenProject(ViewportProjector& screenProjector, const uint16_t vertexIndex)
		{
			if (vertexIndex == 0)
				screenProjector.Project(ScreenPosition);

			return BaseEdgeObject::ScreenProject(screenProjector, vertexIndex);
		}

		/// <summary>
		/// Screen pass:
		/// - Early rejects edges fully behind the camera.
		/// - Applies edge draw mode culling using endpoints and ScreenPosition.
		/// </summary>
		virtual bool ScreenShade(const uint16_t primitiveIndex)
		{
			if (primitiveIndex >= EdgeCount)
				return true;

			if (Primitives[primitiveIndex].z < 0)
				return false;

			auto edge = BaseEdgeObject::GetEdge(primitiveIndex);

			if (Primitives[primitiveIndex].z >= 0)
			{
				// Cull edges with both vertices behind the camera.
				if (Vertices[edge.start].z < 0
					&& Vertices[edge.end].z < 0)
				{
					// Whole edge is out of bounds.
					Primitives[primitiveIndex].z = -VERTEX16_UNIT;
					return false;
				}

				// Cull based on edge cull mode.
				bool renderFragment = true;
				switch (edgeDrawMode)
				{
				case EdgeDrawModeEnum::CullCenterZBehind:
					renderFragment = ScreenPosition.z >
						(int16_t)SignedRightShift(((int32_t)Vertices[edge.start].z
							+ Vertices[edge.end].z), 1);
					break;
				case EdgeDrawModeEnum::CullCenterZFront:
					renderFragment = ScreenPosition.z <
						(int16_t)SignedRightShift(((int32_t)Vertices[edge.start].z
							+ Vertices[edge.end].z), 1);
					break;
				case EdgeDrawModeEnum::CullAllBehind:
					renderFragment = (Vertices[edge.start].z <= ScreenPosition.z
						|| Vertices[edge.end].z <= ScreenPosition.z);
					break;
				case EdgeDrawModeEnum::CullAnyBehind:
					renderFragment = (Vertices[edge.start].z <= ScreenPosition.z
						&& Vertices[edge.end].z <= ScreenPosition.z);
					break;
				case EdgeDrawModeEnum::NoCulling:
				default:
					break;
				}

				if (!renderFragment)
				{
					Primitives[primitiveIndex].z = -VERTEX16_UNIT;
					return false;
				}
			}

			return false;
		}

		/// <summary>
		/// Collects visible edges and pushes them to the fragment collector with a depth key
		/// computed as the average z of the endpoints.
		/// </summary>
		void FragmentCollect(FragmentCollector& fragmentCollector) final
		{
			for (uint_fast16_t i = 0; i < EdgeCount; i++)
			{
				if (Primitives[i].z >= 0)
				{
					auto edge = BaseEdgeObject::GetEdge(i);

					if (Primitives[i].z >= 0)
					{
						Primitives[i].z = (int16_t)SignedRightShift(((int32_t)Vertices[edge.start].z + Vertices[edge.end].z), 1);
						fragmentCollector.AddFragment(i, Primitives[i].z);
					}
				}
			}
		}

		/// <summary>
		/// Produces an edge fragment and calls the fragment shader.
		/// </summary>
		virtual void FragmentShade(WindowRasterizer& rasterizer, const uint16_t primitiveIndex)
		{
			if (FragmentShader == nullptr)
				return;

			auto edge = BaseEdgeObject::GetEdge(primitiveIndex);
			const edge_primitive_t& primitive = Primitives[primitiveIndex];

			EdgeFragment.start = Vertices[edge.start];
			EdgeFragment.end = Vertices[edge.end];
			EdgeFragment.world = primitive.worldPosition;

			GetFragment(EdgeFragment, primitiveIndex);

			if (SceneShader != nullptr)
			{
				FragmentShader->FragmentShade(rasterizer, EdgeFragment, SceneShader);
			}
			else
			{
				FragmentShader->FragmentShade(rasterizer, EdgeFragment);
			}
		}

	protected:
		/// <summary>
		/// Default fragment initializer. Override to set color/material per edge.
		/// </summary>
		virtual void GetFragment(edge_fragment_t& fragment, const uint16_t primitiveIndex)
		{
			fragment.color = Rgb8::WHITE;
			fragment.material.Emissive = 0;
			fragment.material.Diffuse = UFRACTION8_1X;
			fragment.material.Specular = 0;
			fragment.material.Metallic = 0;
		}
	};

	/// <summary>
	/// Dynamic edge object, supporting customizable vertex and edge counts, frustum culling, and edge draw modes.
	/// </summary>
	/// <typeparam name="vertexCount">The max number of vertices in the edge object.</typeparam>
	/// <typeparam name="edgeCount">The max number of edges in the edge object.</typeparam>
	/// <typeparam name="frustumCulling">The frustum culling mode to use, defaulting to ObjectCulling (FrustumCullingEnum).</typeparam>
	/// <typeparam name="edgeDrawMode">The edge draw mode to use, defaulting to NoCulling (EdgeDrawModeEnum).</typeparam>
	template<uint16_t vertexCount, uint16_t edgeCount,
		FrustumCullingEnum frustumCulling = FrustumCullingEnum::ObjectCulling,
		EdgeDrawModeEnum edgeDrawMode = EdgeDrawModeEnum::NoCulling>
	using DynamicEdgeObject = TemplateEdgeObject<AbstractDynamicEdgeObject<vertexCount, edgeCount>, frustumCulling, edgeDrawMode>;

	/// <summary>
	/// Static edge object, supporting frustum culling and edge draw modes.
	/// </summary>
	/// <typeparam name="vertexCount">The number of vertices in the object .</typeparam>
	/// <typeparam name="edgeCount">The number of edges in the object.</typeparam>
	/// <typeparam name="frustumCulling">The frustum culling mode to use. Defaults to ObjectCulling.</typeparam>
	/// <typeparam name="edgeDrawMode">The edge drawing mode to use. Defaults to NoCulling.</typeparam>
	template<uint16_t vertexCount, uint16_t edgeCount,
		FrustumCullingEnum frustumCulling = FrustumCullingEnum::ObjectCulling,
		EdgeDrawModeEnum edgeDrawMode = EdgeDrawModeEnum::NoCulling>
	class StaticEdgeObject : public TemplateEdgeObject<AbstractStaticEdgeObject<vertexCount, edgeCount>, frustumCulling, edgeDrawMode>
	{
	private:
		using Base = TemplateEdgeObject<AbstractStaticEdgeObject<vertexCount, edgeCount>, frustumCulling, edgeDrawMode>;

	public:
		/// <summary>
		/// Constructs a static edge object using the provided vertex/edge sources.
		/// </summary>
		StaticEdgeObject(const vertex16_t vertices[vertexCount], const edge_line_t edges[edgeCount])
			: Base()
		{
			Base::SetStaticSources(vertices, edges);
		}
	};

	/// <summary>
	/// Dynamic edge object with a single color and material, supporting customizable vertex and edge counts, frustum culling, and edge draw modes.
	/// </summary>
	/// <typeparam name="vertexCount">The number of vertices in the object.</typeparam>
	/// <typeparam name="edgeCount">The number of edges in the object.</typeparam>
	/// <typeparam name="frustumCulling">The frustum culling mode to use (default: ObjectCulling).</typeparam>
	/// <typeparam name="edgeDrawMode">The edge draw mode to use (default: NoCulling).</typeparam>
	template<uint16_t vertexCount, uint16_t edgeCount,
		FrustumCullingEnum frustumCulling = FrustumCullingEnum::ObjectCulling,
		EdgeDrawModeEnum edgeDrawMode = EdgeDrawModeEnum::NoCulling>
	class DynamicEdgeSingleColorSingleMaterialObject : public DynamicEdgeObject<vertexCount, edgeCount, frustumCulling, edgeDrawMode>
	{
	private:
		using Base = DynamicEdgeObject<vertexCount, edgeCount, frustumCulling, edgeDrawMode>;

	public:
		Rgb8::color_t Color = Rgb8::WHITE;
		material_t Material{ UFRACTION8_1X, 0, 0, 0 };

	public:
		DynamicEdgeSingleColorSingleMaterialObject() : Base() {}

	protected:
		/// <summary>
		/// Supplies the shared color/material to the edge fragment.
		/// </summary>
		virtual void GetFragment(edge_fragment_t& fragment, const uint16_t primitiveIndex) final
		{
			fragment.color = Color;
			fragment.material = Material;
		}
	};

	/// <summary>
	/// Static edge object with a single color and material, supporting frustum culling and edge draw modes.
	/// </summary>
	/// <typeparam name="vertexCount">The number of vertices in the object.</typeparam>
	/// <typeparam name="edgeCount">The number of edges in the object.</typeparam>
	/// <typeparam name="frustumCulling">The frustum culling mode (default: ObjectCulling).</typeparam>
	/// <typeparam name="edgeDrawMode">The edge draw mode (default: NoCulling).</typeparam>
	template<uint16_t vertexCount, uint16_t edgeCount,
		FrustumCullingEnum frustumCulling = FrustumCullingEnum::ObjectCulling,
		EdgeDrawModeEnum edgeDrawMode = EdgeDrawModeEnum::NoCulling>
	class StaticEdgeSingleColorSingleMaterialObject : public StaticEdgeObject<vertexCount, edgeCount, frustumCulling, edgeDrawMode>
	{
	private:
		using Base = StaticEdgeObject<vertexCount, edgeCount, frustumCulling, edgeDrawMode>;

	public:
		Rgb8::color_t Color = Rgb8::WHITE;
		material_t Material{ UFRACTION8_1X, 0, 0, 0 };

	public:
		StaticEdgeSingleColorSingleMaterialObject(
			const vertex16_t vertices[vertexCount],
			const edge_line_t edges[edgeCount])
			: Base(vertices, edges)
		{
		}

		/// <summary>
		/// Supplies the shared color/material to the edge fragment.
		/// </summary>
		virtual void GetFragment(edge_fragment_t& fragment, const uint16_t primitiveIndex) final
		{
			fragment.color = Color;
			fragment.material = Material;
		}
	};
}
#endif