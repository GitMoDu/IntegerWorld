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
			constexpr uint8_t VertexCount = sizeof(Vertices) / sizeof(vertex16_t);

			static constexpr triangle_face_t Triangles[] PROGMEM
			{
				{0, 2, 1}, {0, 3, 2},
				{5, 7, 4}, {5, 6, 7},
				{4, 3, 0}, {4, 7, 3},
				{1, 6, 5}, {1, 2, 6},
				{3, 6, 2}, {3, 7, 6},
				{0, 5, 4}, {0, 1, 5}
			};

			constexpr uint8_t TriangleCount = sizeof(Triangles) / sizeof(triangle_face_t);

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

			constexpr uint8_t EdgeCount = sizeof(Edges) / sizeof(edge_line_t);
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
			constexpr uint8_t VertexCount = sizeof(Vertices) / sizeof(vertex16_t);

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
			constexpr uint8_t TriangleCount = sizeof(Triangles) / sizeof(triangle_face_t);

			static constexpr edge_line_t Edges[] PROGMEM
			{
				{0, 2}, {0, 3}, {0, 4}, {0, 5},
				{1, 2}, {1, 3}, {1, 4}, {1, 5},
				{2, 4}, {2, 5}, {3, 4}, {3, 5}
			};
			constexpr uint8_t EdgeCount = sizeof(Edges) / sizeof(edge_line_t);
		}

		namespace Icosahedron
		{
			constexpr int16_t X = ((int32_t)SHAPE_UNIT * 525731) / 1250000;
			constexpr int16_t Z = ((int32_t)SHAPE_UNIT * 850651) / 1250000;

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
			constexpr auto VertexCount = sizeof(Vertices) / sizeof(vertex16_t);

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
			constexpr auto TriangleCount = sizeof(Triangles) / sizeof(triangle_face_t);

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
			constexpr auto EdgeCount = sizeof(Edges) / sizeof(edge_line_t);
		}

		namespace Sphere
		{
			static constexpr int16_t UpSize = SHAPE_UNIT / 14;

			static constexpr int16_t DownSize = 128;

			static constexpr vertex16_t Vertices[] PROGMEM
			{
				{(UpSize * (int32_t)(0)) / DownSize , (UpSize * (int32_t)(-1208)) / DownSize , (UpSize * (int32_t)(-746)) / DownSize},
				{(UpSize * (int32_t)(-439)) / DownSize , (UpSize * (int32_t)(-710)) / DownSize , (UpSize * (int32_t)(-1149)) / DownSize},
				{(UpSize * (int32_t)(439)) / DownSize , (UpSize * (int32_t)(-710)) / DownSize , (UpSize * (int32_t)(-1149)) / DownSize},
				{(UpSize * (int32_t)(-746)) / DownSize , (UpSize * (int32_t)(0)) / DownSize , (UpSize * (int32_t)(-1208)) / DownSize},
				{(UpSize * (int32_t)(0)) / DownSize , (UpSize * (int32_t)(0)) / DownSize , (UpSize * (int32_t)(-1420)) / DownSize},
				{(UpSize * (int32_t)(-1149)) / DownSize , (UpSize * (int32_t)(-439)) / DownSize , (UpSize * (int32_t)(-710)) / DownSize},
				{(UpSize * (int32_t)(-710)) / DownSize , (UpSize * (int32_t)(-1149)) / DownSize , (UpSize * (int32_t)(-439)) / DownSize},
				{(UpSize * (int32_t)(439)) / DownSize , (UpSize * (int32_t)(710)) / DownSize , (UpSize * (int32_t)(1149)) / DownSize},
				{(UpSize * (int32_t)(0)) / DownSize , (UpSize * (int32_t)(1208)) / DownSize , (UpSize * (int32_t)(746)) / DownSize},
				{(UpSize * (int32_t)(-439)) / DownSize , (UpSize * (int32_t)(710)) / DownSize , (UpSize * (int32_t)(1149)) / DownSize},
				{(UpSize * (int32_t)(-710)) / DownSize , (UpSize * (int32_t)(1149)) / DownSize , (UpSize * (int32_t)(439)) / DownSize},
				{(UpSize * (int32_t)(-1149)) / DownSize , (UpSize * (int32_t)(-439)) / DownSize , (UpSize * (int32_t)(710)) / DownSize},
				{(UpSize * (int32_t)(-439)) / DownSize , (UpSize * (int32_t)(-710)) / DownSize , (UpSize * (int32_t)(1149)) / DownSize},
				{(UpSize * (int32_t)(-746)) / DownSize , (UpSize * (int32_t)(0)) / DownSize , (UpSize * (int32_t)(1208)) / DownSize},
				{(UpSize * (int32_t)(1208)) / DownSize , (UpSize * (int32_t)(746)) / DownSize , (UpSize * (int32_t)(0)) / DownSize},
				{(UpSize * (int32_t)(710)) / DownSize , (UpSize * (int32_t)(1149)) / DownSize , (UpSize * (int32_t)(-439)) / DownSize},
				{(UpSize * (int32_t)(710)) / DownSize , (UpSize * (int32_t)(1149)) / DownSize , (UpSize * (int32_t)(439)) / DownSize},
				{(UpSize * (int32_t)(1149)) / DownSize , (UpSize * (int32_t)(439)) / DownSize , (UpSize * (int32_t)(710)) / DownSize},
				{(UpSize * (int32_t)(1149)) / DownSize , (UpSize * (int32_t)(439)) / DownSize , (UpSize * (int32_t)(-710)) / DownSize},
				{(UpSize * (int32_t)(-1149)) / DownSize , (UpSize * (int32_t)(439)) / DownSize , (UpSize * (int32_t)(-710)) / DownSize},
				{(UpSize * (int32_t)(0)) / DownSize , (UpSize * (int32_t)(1420)) / DownSize , (UpSize * (int32_t)(0)) / DownSize},
				{(UpSize * (int32_t)(-710)) / DownSize , (UpSize * (int32_t)(1149)) / DownSize , (UpSize * (int32_t)(-439)) / DownSize},
				{(UpSize * (int32_t)(0)) / DownSize , (UpSize * (int32_t)(1208)) / DownSize , (UpSize * (int32_t)(-746)) / DownSize},
				{(UpSize * (int32_t)(-1208)) / DownSize , (UpSize * (int32_t)(746)) / DownSize , (UpSize * (int32_t)(0)) / DownSize},
				{(UpSize * (int32_t)(-1149)) / DownSize , (UpSize * (int32_t)(439)) / DownSize , (UpSize * (int32_t)(710)) / DownSize},
				{(UpSize * (int32_t)(1208)) / DownSize , (UpSize * (int32_t)(-746)) / DownSize , (UpSize * (int32_t)(0)) / DownSize},
				{(UpSize * (int32_t)(1149)) / DownSize , (UpSize * (int32_t)(-439)) / DownSize , (UpSize * (int32_t)(-710)) / DownSize},
				{(UpSize * (int32_t)(1420)) / DownSize , (UpSize * (int32_t)(0)) / DownSize , (UpSize * (int32_t)(0)) / DownSize},
				{(UpSize * (int32_t)(710)) / DownSize , (UpSize * (int32_t)(-1149)) / DownSize , (UpSize * (int32_t)(-439)) / DownSize},
				{(UpSize * (int32_t)(710)) / DownSize , (UpSize * (int32_t)(-1149)) / DownSize , (UpSize * (int32_t)(439)) / DownSize},
				{(UpSize * (int32_t)(1149)) / DownSize , (UpSize * (int32_t)(-439)) / DownSize , (UpSize * (int32_t)(710)) / DownSize},
				{(UpSize * (int32_t)(746)) / DownSize , (UpSize * (int32_t)(0)) / DownSize , (UpSize * (int32_t)(-1208)) / DownSize},
				{(UpSize * (int32_t)(439)) / DownSize , (UpSize * (int32_t)(710)) / DownSize , (UpSize * (int32_t)(-1149)) / DownSize},
				{(UpSize * (int32_t)(-710)) / DownSize , (UpSize * (int32_t)(-1149)) / DownSize , (UpSize * (int32_t)(439)) / DownSize},
				{(UpSize * (int32_t)(-1208)) / DownSize , (UpSize * (int32_t)(-746)) / DownSize , (UpSize * (int32_t)(0)) / DownSize},
				{(UpSize * (int32_t)(-439)) / DownSize , (UpSize * (int32_t)(710)) / DownSize , (UpSize * (int32_t)(-1149)) / DownSize},
				{(UpSize * (int32_t)(0)) / DownSize , (UpSize * (int32_t)(-1420)) / DownSize , (UpSize * (int32_t)(0)) / DownSize},
				{(UpSize * (int32_t)(-1420)) / DownSize , (UpSize * (int32_t)(0)) / DownSize , (UpSize * (int32_t)(0)) / DownSize},
				{(UpSize * (int32_t)(0)) / DownSize , (UpSize * (int32_t)(-1208)) / DownSize , (UpSize * (int32_t)(746)) / DownSize},
				{(UpSize * (int32_t)(439)) / DownSize , (UpSize * (int32_t)(-710)) / DownSize , (UpSize * (int32_t)(1149)) / DownSize},
				{(UpSize * (int32_t)(746)) / DownSize , (UpSize * (int32_t)(0)) / DownSize , (UpSize * (int32_t)(1208)) / DownSize},
				{(UpSize * (int32_t)(0)) / DownSize , (UpSize * (int32_t)(0)) / DownSize , (UpSize * (int32_t)(1420)) / DownSize},
			};
			constexpr uint8_t VertexCount = sizeof(Vertices) / sizeof(vertex16_t);

			static constexpr triangle_face_t Triangles[] PROGMEM
			{
				{ 0, 1, 2 },
				{ 3, 4, 1 },
				{ 4, 2, 1 },
				{ 5, 1, 6 },
				{ 7, 8, 9 },
				{ 10, 9, 8 },
				{ 11, 12, 13 },
				{ 14, 15, 16 },
				{ 16, 17, 14 },
				{ 7, 17, 16 },
				{ 14, 18, 15 },
				{ 5, 19, 3 },
				{ 20, 10, 8 },
				{ 21, 20, 22 },
				{ 23, 10, 21 },
				{ 10, 20, 21 },
				{ 10, 23, 24 },
				{ 25, 26, 27 },
				{ 28, 25, 29 },
				{ 2, 26, 28 },
				{ 29, 25, 30 },
				{ 2, 31, 26 },
				{ 28, 26, 25 },
				{ 0, 2, 28 },
				{ 27, 14, 17 },
				{ 27, 18, 14 },
				{ 26, 31, 18 },
				{ 27, 26, 18 },
				{ 32, 18, 31 },
				{ 33, 34, 6 },
				{ 32, 15, 18 },
				{ 3, 19, 35 },
				{ 36, 33, 6 },
				{ 19, 23, 21 },
				{ 21, 22, 35 },
				{ 35, 19, 21 },
				{ 16, 20, 8 },
				{ 34, 11, 37 },
				{ 8, 7, 16 },
				{ 37, 5, 34 },
				{ 4, 32, 31 },
				{ 5, 6, 34 },
				{ 20, 15, 22 },
				{ 38, 36, 29 },
				{ 20, 16, 15 },
				{ 32, 22, 15 },
				{ 33, 36, 38 },
				{ 39, 38, 29 },
				{ 35, 4, 3 },
				{ 29, 30, 39 },
				{ 32, 35, 22 },
				{ 33, 11, 34 },
				{ 38, 12, 33 },
				{ 35, 32, 4 },
				{ 12, 11, 33 },
				{ 37, 24, 23 },
				{ 37, 11, 24 },
				{ 37, 23, 19 },
				{ 19, 5, 37 },
				{ 36, 6, 0 },
				{ 1, 5, 3 },
				{ 6, 1, 0 },
				{ 31, 2, 4 },
				{ 0, 28, 36 },
				{ 28, 29, 36 },
				{ 24, 9, 10 },
				{ 24, 13, 9 },
				{ 39, 40, 41 },
				{ 13, 12, 41 },
				{ 12, 38, 39 },
				{ 39, 41, 12 },
				{ 27, 30, 25 },
				{ 40, 7, 41 },
				{ 41, 9, 13 },
				{ 7, 9, 41 },
				{ 17, 40, 30 },
				{ 17, 30, 27 },
				{ 11, 13, 24 },
				{ 17, 7, 40 },
				{ 40, 39, 30 },
			};
			constexpr uint8_t TriangleCount = sizeof(Triangles) / sizeof(triangle_face_t);
		}
	}
}

#endif