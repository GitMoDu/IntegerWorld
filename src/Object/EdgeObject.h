#ifndef _INTEGER_WORLD_EDGE_OBJECT_h
#define _INTEGER_WORLD_EDGE_OBJECT_h

#include "PointObject.h"

namespace IntegerWorld
{
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

	struct edge_vertex_t : base_vertex_t
	{
	};

	struct edge_primitive_t : base_primitive_t
	{
		vertex16_t worldPosition;
	};

	template<uint16_t vertexCount, uint16_t edgeCount,
		typename BaseObject = AbstractTransformObject<vertexCount, edgeCount, edge_vertex_t, edge_primitive_t>>
		class EdgeObject : public BaseObject
	{
	public:
		using BaseObject::SceneShader;

	protected:
		using BaseObject::Vertices;
		using BaseObject::Primitives;
		using BaseObject::ObjectPosition;
		using BaseObject::MeshTransform;

	public:
		IFragmentShader<edge_fragment_t>* FragmentShader = nullptr;

	private:
		const vertex16_t* VerticesSource;
		const edge_line_t* EdgesSource;

	private:
		edge_fragment_t EdgeFragment{};

	public:
		EdgeDrawModeEnum EdgeDrawMode;

	protected:
		virtual void GeometryShade(const uint16_t index) {}

	public:
		EdgeObject(const vertex16_t vertices[vertexCount], const edge_line_t edges[edgeCount], const EdgeDrawModeEnum edgeDrawMode = EdgeDrawModeEnum::NoCulling)
			: BaseObject()
			, VerticesSource(vertices)
			, EdgesSource(edges)
			, EdgeDrawMode(edgeDrawMode)
		{
		}

	public:
		bool VertexShade(const uint16_t index) final
		{
			switch (index)
			{
			case 0:
				return BaseObject::VertexShade(0);
			default:
#if defined(ARDUINO_ARCH_AVR)
				Vertices[index - 1].x = pgm_read_word(&VerticesSource[index - 1].x);
				Vertices[index - 1].y = pgm_read_word(&VerticesSource[index - 1].y);
				Vertices[index - 1].z = pgm_read_word(&VerticesSource[index - 1].z);
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

		bool PrimitiveWorldShade(const uint16_t index) final
		{
#if defined(ARDUINO_ARCH_AVR)
			const edge_line_t edge
			{
				(uint16_t)pgm_read_word(&EdgesSource[index].start),
				(uint16_t)pgm_read_word(&EdgesSource[index].end)
			};
#else
			const edge_line_t& edge = EdgesSource[index];
#endif

			// Early cull based on edge cull mode.
			bool renderFragment = true;
			switch (EdgeDrawMode)
			{
			case EdgeDrawModeEnum::CullCenterZBehind:
				renderFragment &= ObjectPosition.z >
					(int16_t)SignedRightShift(((int32_t)Vertices[edge.start].z
						+ Vertices[edge.end].z), 1);
				break;
			case EdgeDrawModeEnum::CullCenterZFront:
				renderFragment &= ObjectPosition.z <
					(int16_t)SignedRightShift(((int32_t)Vertices[edge.start].z
						+ Vertices[edge.end].z), 1);
				break;
			case EdgeDrawModeEnum::CullAllBehind:
				renderFragment &= (Vertices[edge.start].z <= ObjectPosition.z
					|| Vertices[edge.end].z <= ObjectPosition.z);
				break;
			case EdgeDrawModeEnum::CullAnyBehind:
				renderFragment &= (Vertices[edge.start].z <= ObjectPosition.z
					&& Vertices[edge.end].z <= ObjectPosition.z);
				break;
			case EdgeDrawModeEnum::NoCulling:
			default:
				break;
			}

			if (renderFragment)
			{
				Primitives[index].z = 0;
				Primitives[index].worldPosition.x = (int16_t)SignedRightShift(((int32_t)Vertices[edge.start].x + Vertices[edge.end].x), 1);
				Primitives[index].worldPosition.y = (int16_t)SignedRightShift(((int32_t)Vertices[edge.start].y + Vertices[edge.end].y), 1);
				Primitives[index].worldPosition.z = (int16_t)SignedRightShift(((int32_t)Vertices[edge.start].z + Vertices[edge.end].z), 1);
			}
			else
			{
				Primitives[index].z = -VERTEX16_RANGE;
			}

			return index >= edgeCount - 1;
		}

		bool PrimitiveScreenShade(const uint16_t index, const uint16_t boundsWidth, const uint16_t boundsHeight) final
		{
#if defined(ARDUINO_ARCH_AVR)
			const edge_line_t edge
			{
				(uint16_t)pgm_read_word(&EdgesSource[index].start),
				(uint16_t)pgm_read_word(&EdgesSource[index].end)
			};
#else
			const edge_line_t& edge = EdgesSource[index];
#endif

			if (Primitives[index].z != -VERTEX16_RANGE)
			{
				// Quick check if triangle is behind screen.
				if (Vertices[edge.start].z < 0
					&& Vertices[edge.end].z < 0
					|| (Vertices[edge.start].x < 0 && Vertices[edge.end].x < 0)
					|| (Vertices[edge.start].x >= boundsWidth && Vertices[edge.end].x >= boundsWidth)
					|| (Vertices[edge.start].y < 0 && Vertices[edge.end].y < 0)
					|| (Vertices[edge.start].y >= boundsHeight && Vertices[edge.end].y >= boundsHeight))
				{
					// Whole edge is out of bounds.
					Primitives[index].z = -VERTEX16_RANGE;
				}
			}

			return index >= edgeCount - 1;
		}

		void FragmentCollect(FragmentCollector& fragmentCollector) final
		{
			for (uint16_t i = 0; i < edgeCount; i++)
			{
#if defined(ARDUINO_ARCH_AVR)
				const edge_line_t edge
				{
					(uint16_t)pgm_read_word(&EdgesSource[i].start),
					(uint16_t)pgm_read_word(&EdgesSource[i].end)
				};
#else
				const edge_line_t& edge = EdgesSource[i];
#endif

				if (Primitives[i].z != -VERTEX16_RANGE)
				{
					Primitives[i].z = (int16_t)SignedRightShift(((int32_t)Vertices[edge.start].z + Vertices[edge.end].z), 1);
					fragmentCollector.AddFragment(i, Primitives[i].z);
				}
			}
		}

		void FragmentShade(WindowRasterizer& rasterizer, const uint16_t index) final
		{
			if (FragmentShader != nullptr)
			{
#if defined(ARDUINO_ARCH_AVR)
				const edge_line_t edge
				{
					(uint16_t)pgm_read_word(&EdgesSource[index].start),
					(uint16_t)pgm_read_word(&EdgesSource[index].end)
				};
#else
				const edge_line_t& edge = EdgesSource[index];
#endif
				const edge_primitive_t& primitive = Primitives[index];
				EdgeFragment.start = Vertices[edge.start];
				EdgeFragment.end = Vertices[edge.end];
				EdgeFragment.world = primitive.worldPosition;
				EdgeFragment.world = primitive.worldPosition;

				GetFragment(EdgeFragment, index);

				if (SceneShader != nullptr)
				{
					FragmentShader->FragmentShade(rasterizer, EdgeFragment, SceneShader);
				}
				else
				{
					FragmentShader->FragmentShade(rasterizer, EdgeFragment);
				}
			}
		}

	protected:
		virtual void GetFragment(edge_fragment_t& fragment, const uint16_t index)
		{
			fragment.color = Rgb8::WHITE;
			fragment.material.Emissive = 0;
			fragment.material.Diffuse = UFRACTION8_1X;
			fragment.material.Specular = 0;
			fragment.material.Metallic = 0;
		}
	};

	template<uint16_t vertexCount, uint16_t edgeCount,
		typename BaseObject = AbstractTransformObject<vertexCount, edgeCount, edge_vertex_t, edge_primitive_t>>
		class EdgeSingleColorSingleMaterialObject : public EdgeObject<vertexCount, edgeCount, BaseObject>
	{
	private:
		using Base = EdgeObject<vertexCount, edgeCount, BaseObject>;

	public:
		Rgb8::color_t Color = Rgb8::WHITE;
		material_t Material{ UFRACTION8_1X, 0, 0, 0 };

	public:
		EdgeSingleColorSingleMaterialObject(const vertex16_t vertices[vertexCount], const edge_line_t edges[edgeCount], const EdgeDrawModeEnum edgeDrawMode)
			: Base(vertices, edges, edgeDrawMode)
		{
		}

	protected:
		virtual void GetFragment(edge_fragment_t& fragment, const uint16_t index)
		{
			fragment.color = Color;
			fragment.material = Material;
		}
	};
}
#endif