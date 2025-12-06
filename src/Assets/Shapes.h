#ifndef _INTEGER_WORLD_ASSETS_SHAPES_h
#define _INTEGER_WORLD_ASSETS_SHAPES_h

#include "../Framework/Model.h"

namespace Assets
{
	using namespace IntegerWorld;

	namespace Shapes
	{
		static constexpr int16_t SHAPE_UNIT = VERTEX16_UNIT / 4;

		namespace Cube
		{
			static constexpr vertex16_t Vertices[] PROGMEM
			{
				{-SHAPE_UNIT / 2, -SHAPE_UNIT / 2, -SHAPE_UNIT / 2},
				{ SHAPE_UNIT / 2, -SHAPE_UNIT / 2, -SHAPE_UNIT / 2},
				{ SHAPE_UNIT / 2,  SHAPE_UNIT / 2, -SHAPE_UNIT / 2},
				{-SHAPE_UNIT / 2,  SHAPE_UNIT / 2, -SHAPE_UNIT / 2},
				{-SHAPE_UNIT / 2, -SHAPE_UNIT / 2,  SHAPE_UNIT / 2},
				{ SHAPE_UNIT / 2, -SHAPE_UNIT / 2,  SHAPE_UNIT / 2},
				{ SHAPE_UNIT / 2,  SHAPE_UNIT / 2,  SHAPE_UNIT / 2},
				{-SHAPE_UNIT / 2,  SHAPE_UNIT / 2,  SHAPE_UNIT / 2}
			};
			static constexpr uint8_t VertexCount = sizeof(Vertices) / sizeof(vertex16_t);

			static constexpr triangle_face_t Triangles[] PROGMEM
			{
				{0, 2, 1}, {0, 3, 2},
				{5, 7, 4}, {5, 6, 7},
				{4, 3, 0}, {4, 7, 3},
				{1, 6, 5}, {1, 2, 6},
				{3, 6, 2}, {3, 7, 6},
				{0, 5, 4}, {0, 1, 5}
			};

			static constexpr uint8_t TriangleCount = sizeof(Triangles) / sizeof(triangle_face_t);

			static constexpr vertex16_t Normals[TriangleCount] PROGMEM
			{
					{  0,  0, -VERTEX16_UNIT }, {  0,  0, -VERTEX16_UNIT },
					{  0,  0,  VERTEX16_UNIT }, {  0,  0,  VERTEX16_UNIT },
					{ -VERTEX16_UNIT,  0,  0 }, { -VERTEX16_UNIT,  0,  0 },
					{  VERTEX16_UNIT,  0,  0 }, {  VERTEX16_UNIT,  0,  0 },
					{  0,  VERTEX16_UNIT,  0 }, {  0,  VERTEX16_UNIT,  0 },
					{  0, -VERTEX16_UNIT,  0 }, {  0, -VERTEX16_UNIT,  0 }
			};

			static constexpr edge_line_t Edges[] PROGMEM
			{
				{0, 1}, {1, 2}, {2, 3}, {3, 0},
				{4, 5}, {5, 6}, {6, 7}, {7, 4},
				{0, 4}, {1, 5}, {2, 6}, {3, 7}
			};

			static constexpr uint8_t EdgeCount = sizeof(Edges) / sizeof(edge_line_t);
		}

		namespace Octahedron
		{
			static constexpr vertex16_t Vertices[] PROGMEM
			{
				{ 0, ((SHAPE_UNIT * 4) / 4), 0},
				{ 0, -((SHAPE_UNIT * 4) / 4), 0},
				{ ((SHAPE_UNIT * 3) / 4), 0, 0},
				{-((SHAPE_UNIT * 3) / 4), 0, 0},
				{ 0, 0, ((SHAPE_UNIT * 3) / 4)},
				{ 0, 0, -((SHAPE_UNIT * 3) / 4)}
			};
			static constexpr uint8_t VertexCount = sizeof(Vertices) / sizeof(vertex16_t);

			static constexpr triangle_face_t Triangles[] PROGMEM
			{
				{0, 4, 2},
				{0, 3, 4},
				{0, 5, 3},
				{0, 2, 5},
				{1, 2, 4},
				{1, 4, 3},
				{1, 3, 5},
				{1, 5, 2}
			};
			static constexpr uint8_t TriangleCount = sizeof(Triangles) / sizeof(triangle_face_t);

			static constexpr edge_line_t Edges[] PROGMEM
			{
				{0, 2}, {0, 3}, {0, 4}, {0, 5},
				{1, 2}, {1, 3}, {1, 4}, {1, 5},
				{2, 4}, {2, 5}, {3, 4}, {3, 5}
			};
			static constexpr uint8_t EdgeCount = sizeof(Edges) / sizeof(edge_line_t);
		}

		namespace Icosahedron
		{
			static constexpr int16_t X = ((int32_t)SHAPE_UNIT * 525731) / 1250000;
			static constexpr int16_t Z = ((int32_t)SHAPE_UNIT * 850651) / 1250000;

			static constexpr vertex16_t Vertices[] PROGMEM
			{
				{ -X,  0,  Z },
				{  X,  0,  Z },
				{ -X,  0, -Z },
				{  X,  0, -Z },
				{  0,  Z,  X },
				{  0,  Z, -X },
				{  0, -Z,  X },
				{  0, -Z, -X },
				{  Z,  X,  0 },
				{ -Z,  X,  0 },
				{  Z, -X,  0 },
				{ -Z, -X,  0 }
			};
			static constexpr auto VertexCount = sizeof(Vertices) / sizeof(vertex16_t);

			static constexpr triangle_face_t Triangles[] PROGMEM
			{
				{0, 1, 4},
				{0, 4, 9},
				{9, 4, 5},
				{4, 8, 5},
				{4, 1, 8},
				{8, 1, 10},
				{8, 10, 3},
				{5, 8, 3},
				{5, 3, 2},
				{2, 3, 7},
				{7, 3, 10},
				{7, 10, 6},
				{7, 6, 11},
				{11, 6, 0},
				{0, 6, 1},
				{6, 10, 1},
				{9, 11, 0},
				{9, 2, 11},
				{9, 5, 2},
				{7, 11, 2}
			};
			static constexpr auto TriangleCount = sizeof(Triangles) / sizeof(triangle_face_t);

			static constexpr edge_line_t Edges[] PROGMEM
			{
				{0, 1}, {0, 4}, {0, 6}, {0, 9}, {0, 11},
				{1, 4}, {1, 6}, {1, 8}, {1, 10},
				{2, 3}, {2, 5}, {2, 7}, {2, 9}, {2, 11},
				{3, 5}, {3, 7}, {3, 8}, {3, 10},
				{4, 5}, {4, 8}, {4, 9},
				{5, 8}, {5, 9},
				{6, 7}, {6, 10}, {6, 11},
				{7, 10}, {7, 11},
				{8, 10},
				{9, 11}
			};
			static constexpr auto EdgeCount = sizeof(Edges) / sizeof(edge_line_t);
		}
	}
}

#endif