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
		int16_t z;
		vertex16_t worldPosition;
		vertex16_t worldNormal;
	};

	template<uint16_t vertexCount, uint16_t triangleCount,
		typename BaseObject = AbstractOrderedTransformObject<vertexCount, triangleCount, mesh_vertex_t, mesh_primitive_t>>
		class MeshObject : public BaseObject
	{
	public:
		using BaseObject::SceneShader;

	protected:
		using BaseObject::Vertices;
		using BaseObject::Primitives;
		using BaseObject::MeshTransform;
		using BaseObject::OrderedPrimitives;

	public:
		IFragmentShader<triangle_fragment_t>* FragmentShader = nullptr;

	private:
		const vertex16_t* VerticesSource;
		const triangle_face_t* TrianglesSource;
		const vertex16_t* NormalsSource;

	private:
		triangle_fragment_t TriangleFragment{};

	protected:
		virtual void GeometryShade(const uint16_t index) {}

	public:
		MeshObject(const vertex16_t vertices[vertexCount], const triangle_face_t triangles[triangleCount], const vertex16_t normals[triangleCount] = nullptr)
			: BaseObject()
			, VerticesSource(vertices)
			, TrianglesSource(triangles)
			, NormalsSource(normals)
		{
		}

	public:
		virtual bool VertexShade(const uint16_t index)
		{
			switch (index)
			{
			case 0:
				BaseObject::VertexShade(0);
				break;
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

		virtual bool PrimitiveWorldShade(const uint16_t index)
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

				// Rotating a pre-computed normal is slower than computing the normal from the triangle.
				if (NormalsSource != nullptr)
				{
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
					NormalizeVertex16(Primitives[index].worldNormal);
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

		virtual bool PrimitiveScreenShade(const uint16_t boundsWidth, const uint16_t boundsHeight, const uint16_t index)
		{
			if (index < triangleCount)
			{
				OrderedPrimitives[index] = index;
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

				// Check flagged fragments.
				if (Primitives[index].z != VERTEX16_RANGE)
				{
					// Quick check if triangle is behind screen.
					if (Vertices[triangle.v1].z <= 0
						&& Vertices[triangle.v1].z <= 0
						&& Vertices[triangle.v1].z <= 0)
					{
						// Whole triangle is out of bounds.
						Primitives[index].z = VERTEX16_RANGE;
					}
					else
					{
						// Back face culling after projection.
						const int32_t signedArea = (int32_t(Vertices[triangle.v2].x - Vertices[triangle.v1].x) * (Vertices[triangle.v3].y - Vertices[triangle.v1].y)) - (int32_t(Vertices[triangle.v2].y - Vertices[triangle.v1].y) * (Vertices[triangle.v3].x - Vertices[triangle.v1].x));
						if (signedArea <= 0)
						{
							Primitives[index].z = AverageApproximate(Vertices[triangle.v1].z, Vertices[triangle.v2].z, Vertices[triangle.v3].z);

							if (Primitives[index].z < 0)
							{
								// Triangle centroid is behind the screen.
								Primitives[index].z = VERTEX16_RANGE;
							}
						}
						else
						{
							Primitives[index].z = VERTEX16_RANGE;
						}
					}
				}
			}

			return index >= triangleCount - 1;
		}

		virtual bool FragmentShade(WindowRasterizer& rasterizer, const uint16_t index)
		{
			const uint16_t orderedIndex = OrderedPrimitives[index];
			const mesh_primitive_t& primitive = Primitives[orderedIndex];

			if (FragmentShader != nullptr && primitive.z != VERTEX16_RANGE)
			{
				TriangleFragment.normal.x = primitive.worldNormal.x;
				TriangleFragment.normal.y = primitive.worldNormal.y;
				TriangleFragment.normal.z = primitive.worldNormal.z;
				TriangleFragment.world = primitive.worldPosition;

#if defined(ARDUINO_ARCH_AVR)
				const triangle_face_t triangle
				{
					(uint16_t)pgm_read_word(&TrianglesSource[orderedIndex].v1),
					(uint16_t)pgm_read_word(&TrianglesSource[orderedIndex].v2),
					(uint16_t)pgm_read_word(&TrianglesSource[orderedIndex].v3)
				};
#else
				const triangle_face_t& triangle = TrianglesSource[orderedIndex];
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

				GetFragment(TriangleFragment, orderedIndex);

				if (SceneShader != nullptr)
				{
					FragmentShader->FragmentShade(rasterizer, TriangleFragment, SceneShader);
				}
				else
				{
					FragmentShader->FragmentShade(rasterizer, TriangleFragment);
				}
			}

			return index >= triangleCount - 1;
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

	template<uint16_t vertexCount, uint16_t triangleCount,
		typename BaseObject = AbstractOrderedTransformObject<vertexCount, triangleCount, mesh_vertex_t, mesh_primitive_t>>
		class MeshSingleColorSingleMaterialObject : public MeshObject<vertexCount, triangleCount, BaseObject>
	{
	private:
		using Base = MeshObject<vertexCount, triangleCount, BaseObject>;

	public:
		color_fraction16_t Color{ UFRACTION16_1X, UFRACTION16_1X ,UFRACTION16_1X };
		material_t Material{ UFRACTION8_1X, 0, 0, 0 };

	public:
		MeshSingleColorSingleMaterialObject(const vertex16_t vertices[vertexCount], const triangle_face_t triangles[triangleCount], const vertex16_t normals[triangleCount] = nullptr)
			: Base(vertices, triangles, normals)
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