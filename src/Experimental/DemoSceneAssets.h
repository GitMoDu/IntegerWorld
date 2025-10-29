#ifndef _DEMO_SCENE_ASSETS_h
#define _DEMO_SCENE_ASSETS_h

#include "../Framework/Model.h"

#if defined(INTEGER_WORLD_TEXTURED_CUBE_DEMO)
#include "CubeTexture.h"
#endif

namespace Assets
{
	using namespace IntegerWorld;

	namespace Shapes
	{
		namespace Star
		{
			static constexpr int16_t scaleFactor = SHAPE_UNIT / 128;

			static constexpr vertex16_t Vertices[] PROGMEM
			{
				{ (int16_t)((int32_t)32 * scaleFactor), (int16_t)((int32_t)-100 * scaleFactor), (int16_t)((int32_t)-27 * scaleFactor) },
				{ (int16_t)((int32_t)-17 * scaleFactor), (int16_t)((int32_t)-50 * scaleFactor), (int16_t)((int32_t)-27 * scaleFactor) },
				{ (int16_t)((int32_t)42 * scaleFactor), (int16_t)((int32_t)-31 * scaleFactor), (int16_t)((int32_t)-27 * scaleFactor) },
				{ (int16_t)((int32_t)-53 * scaleFactor), (int16_t)((int32_t)0 * scaleFactor), (int16_t)((int32_t)-27 * scaleFactor) },
				{ (int16_t)((int32_t)-17 * scaleFactor), (int16_t)((int32_t)49 * scaleFactor), (int16_t)((int32_t)-27 * scaleFactor) },
				{ (int16_t)((int32_t)-85 * scaleFactor), (int16_t)((int32_t)-62 * scaleFactor), (int16_t)((int32_t)-27 * scaleFactor) },
				{ (int16_t)((int32_t)42 * scaleFactor), (int16_t)((int32_t)30 * scaleFactor), (int16_t)((int32_t)-27 * scaleFactor) },
				{ (int16_t)((int32_t)-85 * scaleFactor), (int16_t)((int32_t)61 * scaleFactor), (int16_t)((int32_t)-27 * scaleFactor) },
				{ (int16_t)((int32_t)32 * scaleFactor), (int16_t)((int32_t)99 * scaleFactor), (int16_t)((int32_t)-27 * scaleFactor) },
				{ (int16_t)((int32_t)104 * scaleFactor), (int16_t)((int32_t)0 * scaleFactor), (int16_t)((int32_t)-27 * scaleFactor) },
				{ (int16_t)((int32_t)32 * scaleFactor), (int16_t)((int32_t)-100 * scaleFactor), (int16_t)((int32_t)26 * scaleFactor) },
				{ (int16_t)((int32_t)42 * scaleFactor), (int16_t)((int32_t)-31 * scaleFactor), (int16_t)((int32_t)26 * scaleFactor) },
				{ (int16_t)((int32_t)-17 * scaleFactor), (int16_t)((int32_t)-50 * scaleFactor), (int16_t)((int32_t)26 * scaleFactor) },
				{ (int16_t)((int32_t)42 * scaleFactor), (int16_t)((int32_t)30 * scaleFactor), (int16_t)((int32_t)26 * scaleFactor) },
				{ (int16_t)((int32_t)104 * scaleFactor), (int16_t)((int32_t)0 * scaleFactor), (int16_t)((int32_t)26 * scaleFactor) },
				{ (int16_t)((int32_t)-17 * scaleFactor), (int16_t)((int32_t)49 * scaleFactor), (int16_t)((int32_t)26 * scaleFactor) },
				{ (int16_t)((int32_t)32 * scaleFactor), (int16_t)((int32_t)99 * scaleFactor), (int16_t)((int32_t)26 * scaleFactor) },
				{ (int16_t)((int32_t)-53 * scaleFactor), (int16_t)((int32_t)0 * scaleFactor), (int16_t)((int32_t)26 * scaleFactor) },
				{ (int16_t)((int32_t)-85 * scaleFactor), (int16_t)((int32_t)61 * scaleFactor), (int16_t)((int32_t)26 * scaleFactor) },
				{ (int16_t)((int32_t)-85 * scaleFactor), (int16_t)((int32_t)-62 * scaleFactor), (int16_t)((int32_t)26 * scaleFactor) }
			};
			constexpr uint16_t VertexCount = sizeof(Vertices) / sizeof(vertex16_t);

			static constexpr triangle_face_t Triangles[] PROGMEM
			{
				{ 0, 1, 2 },
				{ 3, 4, 1 },
				{ 5, 3, 1 },
				{ 2, 1, 6 },
				{ 7, 4, 3 },
				{ 6, 1, 4 },
				{ 8, 6, 4 },
				{ 9, 2, 6 },
				{ 10, 11, 12 },
				{ 11, 13, 12 },
				{ 14, 13, 11 },
				{ 13, 15, 12 },
				{ 16, 15, 13 },
				{ 17, 12, 15 },
				{ 18, 17, 15 },
				{ 19, 12, 17 },
				{ 8, 16, 13 },
				{ 8, 13, 6 },
				{ 14, 9, 6 },
				{ 14, 6, 13 },
				{ 4, 7, 18 },
				{ 4, 18, 15 },
				{ 15, 16, 4 },
				{ 8, 4, 16 },
				{ 3, 5, 19 },
				{ 3, 19, 17 },
				{ 1, 12, 19 },
				{ 1, 19, 5 },
				{ 14, 11, 2 },
				{ 14, 2, 9 },
				{ 2, 11, 10 },
				{ 2, 10, 0 },
				{ 17, 18, 3 },
				{ 7, 3, 18 },
				{ 1, 0, 10 },
				{ 1, 10, 12 },
			};
			constexpr uint16_t TriangleCount = sizeof(Triangles) / sizeof(triangle_face_t);

			static constexpr vertex16_t Normals[TriangleCount]{
				// Normals for each triangle, calculated and normalized, scaled to VERTEX16_UNIT
				{   0, -819, -574 }, // { 0, 1, 2 }
				{ -819, -574,   0 }, // { 3, 4, 1 }
				{ -574, -819,   0 }, // { 5, 3, 1 }
				{   0, -574, -819 }, // { 2, 1, 6 }
				{ -819,   0, -574 }, // { 7, 4, 3 }
				{   0, -819, -574 }, // { 6, 1, 4 }
				{   0, -574, -819 }, // { 8, 6, 4 }
				{  819,   0, -574 }, // { 9, 2, 6 }
				{   0,  819, -574 }, // { 10, 11, 12 }
				{  819, -574,   0 }, // { 11, 13, 12 }
				{  574,  819,   0 }, // { 14, 13, 11 }
				{   0,  574,  819 }, // { 13, 15, 12 }
				{  819,   0,  574 }, // { 16, 15, 13 }
				{   0,  574,  819 }, // { 17, 12, 15 }
				{   0,  819,  574 }, // { 18, 17, 15 }
				{ -819,   0,  574 }, // { 19, 12, 17 }
				{   0,  574,  819 }, // { 8, 16, 13 }
				{   0,  574,  819 }, // { 8, 13, 6 }
				{  574,  819,   0 }, // { 14, 9, 6 }
				{  574,  819,   0 }, // { 14, 6, 13 }
				{ -574,  819,   0 }, // { 4, 7, 18 }
				{ -574,  819,   0 }, // { 4, 18, 15 }
				{   0,  574,  819 }, // { 15, 16, 4 }
				{   0,  574,  819 }, // { 8, 4, 16 }
				{ -819,   0,  574 }, // { 3, 5, 19 }
				{ -819,   0,  574 }, // { 3, 19, 17 }
				{   0, -574,  819 }, // { 1, 12, 19 }
				{   0, -574,  819 }, // { 1, 19, 5 }
				{  574, -819,   0 }, // { 14, 11, 2 }
				{  574, -819,   0 }, // { 14, 2, 9 }
				{   0, -819,  574 }, // { 2, 11, 10 }
				{   0, -819,  574 }, // { 2, 10, 0 }
				{ -574, -819,   0 }, // { 17, 18, 3 }
				{ -574, -819,   0 }, // { 7, 3, 18 }
				{   0, -819,  574 }, // { 1, 0, 10 }
				{   0, -819,  574 }, // { 1, 10, 12 }
			};
		}

		namespace Grid8x8
		{
			static constexpr vertex16_t Vertices[] =
			{
				{ -SHAPE_UNIT * 7 / 28, 0, -SHAPE_UNIT * 7 / 28},
				{ -SHAPE_UNIT * 5 / 28, 0, -SHAPE_UNIT * 7 / 28},
				{ -SHAPE_UNIT * 3 / 28, 0, -SHAPE_UNIT * 7 / 28},
				{ -SHAPE_UNIT * 1 / 28, 0, -SHAPE_UNIT * 7 / 28},
				{SHAPE_UNIT * 1 / 28, 0, -SHAPE_UNIT * 7 / 28},
				{SHAPE_UNIT * 3 / 28, 0, -SHAPE_UNIT * 7 / 28},
				{SHAPE_UNIT * 5 / 28, 0, -SHAPE_UNIT * 7 / 28},
				{SHAPE_UNIT * 7 / 28, 0, -SHAPE_UNIT * 7 / 28},

				{ -SHAPE_UNIT * 7 / 28, 0, -SHAPE_UNIT * 5 / 28},
				{ -SHAPE_UNIT * 5 / 28, 0, -SHAPE_UNIT * 5 / 28},
				{ -SHAPE_UNIT * 3 / 28, 0, -SHAPE_UNIT * 5 / 28},
				{ -SHAPE_UNIT * 1 / 28, 0, -SHAPE_UNIT * 5 / 28},
				{SHAPE_UNIT * 1 / 28, 0, -SHAPE_UNIT * 5 / 28},
				{SHAPE_UNIT * 3 / 28, 0, -SHAPE_UNIT * 5 / 28},
				{SHAPE_UNIT * 5 / 28, 0, -SHAPE_UNIT * 5 / 28},
				{SHAPE_UNIT * 7 / 28, 0, -SHAPE_UNIT * 5 / 28},

				{ -SHAPE_UNIT * 7 / 28, 0, -SHAPE_UNIT * 3 / 28},
				{ -SHAPE_UNIT * 5 / 28, 0, -SHAPE_UNIT * 3 / 28},
				{ -SHAPE_UNIT * 3 / 28, 0, -SHAPE_UNIT * 3 / 28},
				{ -SHAPE_UNIT * 1 / 28, 0, -SHAPE_UNIT * 3 / 28},
				{SHAPE_UNIT * 1 / 28, 0, -SHAPE_UNIT * 3 / 28},
				{SHAPE_UNIT * 3 / 28, 0, -SHAPE_UNIT * 3 / 28},
				{SHAPE_UNIT * 5 / 28, 0, -SHAPE_UNIT * 3 / 28},
				{SHAPE_UNIT * 7 / 28, 0, -SHAPE_UNIT * 3 / 28},

				{ -SHAPE_UNIT * 7 / 28, 0, -SHAPE_UNIT * 1 / 28},
				{ -SHAPE_UNIT * 5 / 28, 0, -SHAPE_UNIT * 1 / 28},
				{ -SHAPE_UNIT * 3 / 28, 0, -SHAPE_UNIT * 1 / 28},
				{ -SHAPE_UNIT * 1 / 28, 0, -SHAPE_UNIT * 1 / 28},
				{SHAPE_UNIT * 1 / 28, 0, -SHAPE_UNIT * 1 / 28},
				{SHAPE_UNIT * 3 / 28, 0, -SHAPE_UNIT * 1 / 28},
				{SHAPE_UNIT * 5 / 28, 0, -SHAPE_UNIT * 1 / 28},
				{SHAPE_UNIT * 7 / 28, 0, -SHAPE_UNIT * 1 / 28},

				{ -SHAPE_UNIT * 7 / 28, 0,SHAPE_UNIT * 1 / 28},
				{ -SHAPE_UNIT * 5 / 28, 0,SHAPE_UNIT * 1 / 28},
				{ -SHAPE_UNIT * 3 / 28, 0,SHAPE_UNIT * 1 / 28},
				{ -SHAPE_UNIT * 1 / 28, 0,SHAPE_UNIT * 1 / 28},
				{SHAPE_UNIT * 1 / 28, 0,SHAPE_UNIT * 1 / 28},
				{SHAPE_UNIT * 3 / 28, 0,SHAPE_UNIT * 1 / 28},
				{SHAPE_UNIT * 5 / 28, 0,SHAPE_UNIT * 1 / 28},
				{SHAPE_UNIT * 7 / 28, 0,SHAPE_UNIT * 1 / 28},

				{ -SHAPE_UNIT * 7 / 28, 0,SHAPE_UNIT * 3 / 28},
				{ -SHAPE_UNIT * 5 / 28, 0,SHAPE_UNIT * 3 / 28},
				{ -SHAPE_UNIT * 3 / 28, 0,SHAPE_UNIT * 3 / 28},
				{ -SHAPE_UNIT * 1 / 28, 0,SHAPE_UNIT * 3 / 28},
				{SHAPE_UNIT * 1 / 28, 0,SHAPE_UNIT * 3 / 28},
				{SHAPE_UNIT * 3 / 28, 0,SHAPE_UNIT * 3 / 28},
				{SHAPE_UNIT * 5 / 28, 0,SHAPE_UNIT * 3 / 28},
				{SHAPE_UNIT * 7 / 28, 0,SHAPE_UNIT * 3 / 28},

				{ -SHAPE_UNIT * 7 / 28, 0,SHAPE_UNIT * 5 / 28},
				{ -SHAPE_UNIT * 5 / 28, 0,SHAPE_UNIT * 5 / 28},
				{ -SHAPE_UNIT * 3 / 28, 0,SHAPE_UNIT * 5 / 28},
				{ -SHAPE_UNIT * 1 / 28, 0,SHAPE_UNIT * 5 / 28},
				{SHAPE_UNIT * 1 / 28, 0,SHAPE_UNIT * 5 / 28},
				{SHAPE_UNIT * 3 / 28, 0,SHAPE_UNIT * 5 / 28},
				{SHAPE_UNIT * 5 / 28, 0,SHAPE_UNIT * 5 / 28},
				{SHAPE_UNIT * 7 / 28, 0,SHAPE_UNIT * 5 / 28},

				{ -SHAPE_UNIT * 7 / 28, 0,SHAPE_UNIT * 7 / 28},
				{ -SHAPE_UNIT * 5 / 28, 0,SHAPE_UNIT * 7 / 28},
				{ -SHAPE_UNIT * 3 / 28, 0,SHAPE_UNIT * 7 / 28},
				{ -SHAPE_UNIT * 1 / 28, 0,SHAPE_UNIT * 7 / 28},
				{SHAPE_UNIT * 1 / 28, 0,SHAPE_UNIT * 7 / 28},
				{SHAPE_UNIT * 3 / 28, 0,SHAPE_UNIT * 7 / 28},
				{SHAPE_UNIT * 5 / 28, 0,SHAPE_UNIT * 7 / 28},
				{SHAPE_UNIT * 7 / 28, 0,SHAPE_UNIT * 7 / 28}
			};

			constexpr uint8_t VertexCount = sizeof(Vertices) / sizeof(vertex16_t);
		}

		namespace Sphere
		{
			static constexpr int16_t UpSize = 1;

			static constexpr int16_t DownSize = 1;

			static constexpr vertex16_t Vertices[] PROGMEM
			{
				{(UpSize * (int32_t)(-1280)) / DownSize , (UpSize * (int32_t)(0)) / DownSize , (UpSize * (int32_t)(0)) / DownSize},
				{(UpSize * (int32_t)(-1036)) / DownSize , (UpSize * (int32_t)(-396)) / DownSize , (UpSize * (int32_t)(-640)) / DownSize},
				{(UpSize * (int32_t)(0)) / DownSize , (UpSize * (int32_t)(-1089)) / DownSize , (UpSize * (int32_t)(673)) / DownSize},
				{(UpSize * (int32_t)(-640)) / DownSize , (UpSize * (int32_t)(-1036)) / DownSize , (UpSize * (int32_t)(396)) / DownSize},
				{(UpSize * (int32_t)(0)) / DownSize , (UpSize * (int32_t)(-1280)) / DownSize , (UpSize * (int32_t)(0)) / DownSize},
				{(UpSize * (int32_t)(-1089)) / DownSize , (UpSize * (int32_t)(-673)) / DownSize , (UpSize * (int32_t)(0)) / DownSize},
				{(UpSize * (int32_t)(-640)) / DownSize , (UpSize * (int32_t)(-1036)) / DownSize , (UpSize * (int32_t)(-396)) / DownSize},
				{(UpSize * (int32_t)(0)) / DownSize , (UpSize * (int32_t)(-1089)) / DownSize , (UpSize * (int32_t)(-673)) / DownSize},
				{(UpSize * (int32_t)(-1036)) / DownSize , (UpSize * (int32_t)(-396)) / DownSize , (UpSize * (int32_t)(640)) / DownSize},
				{(UpSize * (int32_t)(-673)) / DownSize , (UpSize * (int32_t)(0)) / DownSize , (UpSize * (int32_t)(-1089)) / DownSize},
				{(UpSize * (int32_t)(-396)) / DownSize , (UpSize * (int32_t)(-640)) / DownSize , (UpSize * (int32_t)(-1036)) / DownSize},
				{(UpSize * (int32_t)(673)) / DownSize , (UpSize * (int32_t)(0)) / DownSize , (UpSize * (int32_t)(1089)) / DownSize},
				{(UpSize * (int32_t)(396)) / DownSize , (UpSize * (int32_t)(-640)) / DownSize , (UpSize * (int32_t)(1036)) / DownSize},
				{(UpSize * (int32_t)(-396)) / DownSize , (UpSize * (int32_t)(640)) / DownSize , (UpSize * (int32_t)(1036)) / DownSize},
				{(UpSize * (int32_t)(-673)) / DownSize , (UpSize * (int32_t)(0)) / DownSize , (UpSize * (int32_t)(1089)) / DownSize},
				{(UpSize * (int32_t)(-396)) / DownSize , (UpSize * (int32_t)(-640)) / DownSize , (UpSize * (int32_t)(1036)) / DownSize},
				{(UpSize * (int32_t)(0)) / DownSize , (UpSize * (int32_t)(1089)) / DownSize , (UpSize * (int32_t)(673)) / DownSize},
				{(UpSize * (int32_t)(0)) / DownSize , (UpSize * (int32_t)(0)) / DownSize , (UpSize * (int32_t)(-1280)) / DownSize},
				{(UpSize * (int32_t)(396)) / DownSize , (UpSize * (int32_t)(640)) / DownSize , (UpSize * (int32_t)(-1036)) / DownSize},
				{(UpSize * (int32_t)(673)) / DownSize , (UpSize * (int32_t)(0)) / DownSize , (UpSize * (int32_t)(-1089)) / DownSize},
				{(UpSize * (int32_t)(0)) / DownSize , (UpSize * (int32_t)(1089)) / DownSize , (UpSize * (int32_t)(-673)) / DownSize},
				{(UpSize * (int32_t)(1089)) / DownSize , (UpSize * (int32_t)(673)) / DownSize , (UpSize * (int32_t)(0)) / DownSize},
				{(UpSize * (int32_t)(1036)) / DownSize , (UpSize * (int32_t)(396)) / DownSize , (UpSize * (int32_t)(640)) / DownSize},
				{(UpSize * (int32_t)(1280)) / DownSize , (UpSize * (int32_t)(0)) / DownSize , (UpSize * (int32_t)(0)) / DownSize},
				{(UpSize * (int32_t)(396)) / DownSize , (UpSize * (int32_t)(640)) / DownSize , (UpSize * (int32_t)(1036)) / DownSize},
				{(UpSize * (int32_t)(0)) / DownSize , (UpSize * (int32_t)(0)) / DownSize , (UpSize * (int32_t)(1280)) / DownSize},
				{(UpSize * (int32_t)(-640)) / DownSize , (UpSize * (int32_t)(1036)) / DownSize , (UpSize * (int32_t)(-396)) / DownSize},
				{(UpSize * (int32_t)(0)) / DownSize , (UpSize * (int32_t)(1280)) / DownSize , (UpSize * (int32_t)(0)) / DownSize},
				{(UpSize * (int32_t)(-1089)) / DownSize , (UpSize * (int32_t)(673)) / DownSize , (UpSize * (int32_t)(0)) / DownSize},
				{(UpSize * (int32_t)(-640)) / DownSize , (UpSize * (int32_t)(1036)) / DownSize , (UpSize * (int32_t)(396)) / DownSize},
				{(UpSize * (int32_t)(-1036)) / DownSize , (UpSize * (int32_t)(396)) / DownSize , (UpSize * (int32_t)(640)) / DownSize},
				{(UpSize * (int32_t)(-1036)) / DownSize , (UpSize * (int32_t)(396)) / DownSize , (UpSize * (int32_t)(-640)) / DownSize},
				{(UpSize * (int32_t)(-396)) / DownSize , (UpSize * (int32_t)(640)) / DownSize , (UpSize * (int32_t)(-1036)) / DownSize},
				{(UpSize * (int32_t)(396)) / DownSize , (UpSize * (int32_t)(-640)) / DownSize , (UpSize * (int32_t)(-1036)) / DownSize},
				{(UpSize * (int32_t)(1036)) / DownSize , (UpSize * (int32_t)(-396)) / DownSize , (UpSize * (int32_t)(640)) / DownSize},
				{(UpSize * (int32_t)(1089)) / DownSize , (UpSize * (int32_t)(-673)) / DownSize , (UpSize * (int32_t)(0)) / DownSize},
				{(UpSize * (int32_t)(1036)) / DownSize , (UpSize * (int32_t)(-396)) / DownSize , (UpSize * (int32_t)(-640)) / DownSize},
				{(UpSize * (int32_t)(640)) / DownSize , (UpSize * (int32_t)(-1036)) / DownSize , (UpSize * (int32_t)(396)) / DownSize},
				{(UpSize * (int32_t)(640)) / DownSize , (UpSize * (int32_t)(-1036)) / DownSize , (UpSize * (int32_t)(-396)) / DownSize},
				{(UpSize * (int32_t)(1036)) / DownSize , (UpSize * (int32_t)(396)) / DownSize , (UpSize * (int32_t)(-640)) / DownSize},
				{(UpSize * (int32_t)(640)) / DownSize , (UpSize * (int32_t)(1036)) / DownSize , (UpSize * (int32_t)(396)) / DownSize},
				{(UpSize * (int32_t)(640)) / DownSize , (UpSize * (int32_t)(1036)) / DownSize , (UpSize * (int32_t)(-396)) / DownSize},
			};

			constexpr auto VertexCount = sizeof(Vertices) / sizeof(vertex16_t);

			static constexpr triangle_face_t Triangles[] PROGMEM
			{
				{ 2, 3, 4 },
				{ 3, 5, 6 },
				{ 6, 7, 4 },
				{ 4, 3, 6 },
				{ 3, 8, 5 },
				{ 9, 10, 1 },
				{ 6, 10, 7 },
				{ 1, 6, 5 },
				{ 1, 10, 6 },
				{ 8, 15, 14 },
				{ 15, 3, 2 },
				{ 8, 3, 15 },
				{ 17, 18, 19 },
				{ 21, 22, 23 },
				{ 11, 24, 25 },
				{ 13, 14, 25 },
				{ 25, 24, 13 },
				{ 25, 12, 11 },
				{ 14, 15, 25 },
				{ 15, 2, 12 },
				{ 12, 25, 15 },
				{ 20, 26, 27 },
				{ 26, 28, 29 },
				{ 29, 27, 26 },
				{ 13, 30, 14 },
				{ 29, 13, 16 },
				{ 30, 29, 28 },
				{ 30, 13, 29 },
				{ 31, 32, 9 },
				{ 26, 31, 28 },
				{ 20, 32, 26 },
				{ 32, 31, 26 },
				{ 19, 33, 17 },
				{ 34, 35, 23 },
				{ 35, 36, 23 },
				{ 4, 37, 2 },
				{ 7, 38, 4 },
				{ 38, 35, 37 },
				{ 37, 4, 38 },
				{ 11, 12, 34 },
				{ 37, 12, 2 },
				{ 35, 34, 37 },
				{ 22, 11, 34 },
				{ 34, 23, 22 },
				{ 23, 39, 21 },
				{ 19, 39, 36 },
				{ 23, 36, 39 },
				{ 21, 39, 41 },
				{ 18, 39, 19 },
				{ 18, 41, 39 },
				{ 16, 13, 24 },
				{ 9, 32, 17 },
				{ 32, 20, 18 },
				{ 18, 17, 32 },
				{ 30, 28, 0 },
				{ 28, 31, 0 },
				{ 31, 9, 1 },
				{ 1, 0, 31 },
				{ 27, 29, 16 },
				{ 16, 40, 27 },
				{ 40, 21, 41 },
				{ 41, 20, 27 },
				{ 27, 40, 41 },
				{ 24, 40, 16 },
				{ 22, 24, 11 },
				{ 40, 22, 21 },
				{ 40, 24, 22 },
				{ 41, 18, 20 },
				{ 34, 12, 37 },
				{ 36, 33, 19 },
				{ 38, 36, 35 },
				{ 7, 33, 38 },
				{ 33, 36, 38 },
				{ 7, 10, 33 },
				{ 10, 9, 17 },
				{ 17, 33, 10 },
				{ 5, 8, 0 },
				{ 14, 30, 8 },
				{ 30, 0, 8 },
				{ 0, 1, 5 },
			};

			constexpr auto TriangleCount = sizeof(Triangles) / sizeof(triangle_face_t);
		}
	}

	namespace Palletes
	{
		namespace Cube
		{
			static constexpr Rgb8::color_t Albedos[]
			{
				0xFF0000,
				0x00FF00,
				0x0000FF,
				0xFFFF00,
				0x00FFFF,
				0xFF00FF
			};

			static constexpr uint8_t AlbedoPallete[Shapes::Cube::TriangleCount]
			{
				0,0,
				1,1,
				2,2,
				3,3,
				4,4,
				5,5
			};
		}
	}

	namespace RenderObjects
	{
		using namespace Shapes;
		using namespace ::IntegerWorld::RenderObjects;

		struct CubeEdgeLineObject : Edge::SimpleStaticEdgeLineObject<Cube::VertexCount, Cube::EdgeCount>
		{
			CubeEdgeLineObject()
				: Edge::SimpleStaticEdgeLineObject<Cube::VertexCount, Cube::EdgeCount>(Cube::Vertices, Cube::Edges)
			{
			}
		};

		struct CubeEdgeVertexObject : Edge::SimpleStaticEdgeVertexObject<Cube::VertexCount, Cube::EdgeCount>
		{
			CubeEdgeVertexObject()
				: Edge::SimpleStaticEdgeVertexObject<Cube::VertexCount, Cube::EdgeCount>(Cube::Vertices, Cube::Edges)
			{
			}
		};

		class CubeMeshTriangleObject
			: public Mesh::TriangleShadeObject<
			Cube::VertexCount, Cube::TriangleCount,
			PrimitiveSources::Vertex::Static::Source,
			PrimitiveSources::Triangle::Static::Source,
			FrustumCullingEnum::PrimitiveCulling,
			FaceCullingEnum::BackfaceCulling,
			PrimitiveSources::Albedo::Static::PalletedSource>
		{
		private:
			using Base = Mesh::TriangleShadeObject<
				Cube::VertexCount, Cube::TriangleCount,
				PrimitiveSources::Vertex::Static::Source,
				PrimitiveSources::Triangle::Static::Source,
				FrustumCullingEnum::PrimitiveCulling,
				FaceCullingEnum::BackfaceCulling,
				PrimitiveSources::Albedo::Static::PalletedSource>;

		private:
			PrimitiveSources::Vertex::Static::Source VerticesSource;
			PrimitiveSources::Triangle::Static::Source TrianglesSource;
			PrimitiveSources::Albedo::Static::PalletedSource AlbedosSource;

		public:
			CubeMeshTriangleObject()
				: VerticesSource(Cube::Vertices)
				, TrianglesSource(Cube::Triangles)
				, AlbedosSource(Palletes::Cube::Albedos, Palletes::Cube::AlbedoPallete)
				, Base(VerticesSource, TrianglesSource, AlbedosSource)
			{
			}
		};

#if defined(INTEGER_WORLD_TEXTURED_CUBE_DEMO)
		class CubeTexturedTriangleObject
			: public Mesh::TriangleShadeObject<
			Cube::VertexCount, Cube::TriangleCount,
			PrimitiveSources::Vertex::Static::Source,
			PrimitiveSources::Triangle::Static::Source,
			FrustumCullingEnum::PrimitiveCulling,
			FaceCullingEnum::BackfaceCulling,
			PrimitiveSources::Albedo::Static::FullSource,
			PrimitiveSources::Material::DiffuseMaterialSource,
			PrimitiveSources::Normal::Static::NoSource,
			PrimitiveSources::Uv::Static::Source>
		{
		private:
			using Base = Mesh::TriangleShadeObject<
				Cube::VertexCount, Cube::TriangleCount,
				PrimitiveSources::Vertex::Static::Source,
				PrimitiveSources::Triangle::Static::Source,
				FrustumCullingEnum::PrimitiveCulling,
				FaceCullingEnum::BackfaceCulling,
				PrimitiveSources::Albedo::Static::FullSource,
				PrimitiveSources::Material::DiffuseMaterialSource,
				PrimitiveSources::Normal::Static::NoSource,
				PrimitiveSources::Uv::Static::Source>;

		private:
			PrimitiveSources::Vertex::Static::Source VerticesSource;
			PrimitiveSources::Triangle::Static::Source TrianglesSource;
			PrimitiveSources::Uv::Static::Source UvsSource;

		public:
			CubeTexturedTriangleObject()
				: VerticesSource(Cube::Vertices)
				, TrianglesSource(Cube::Triangles)
				, UvsSource(Assets::Uvs::Cube::UVs)
				, Base(VerticesSource, TrianglesSource,
					const_cast<PrimitiveSources::Albedo::Static::FullSource&>(PrimitiveSources::Albedo::FullAlbedoSourceInstance),
					const_cast<PrimitiveSources::Material::DiffuseMaterialSource&>(PrimitiveSources::Material::DiffuseMaterialSourceInstance),
					const_cast<PrimitiveSources::Normal::Static::NoSource&>(PrimitiveSources::Normal::NormalNoSourceInstance),
					UvsSource)
			{
			}
		};

		class CubeTexturedVertexObject
			: public Mesh::VertexShadeObject<
			Cube::VertexCount, Cube::TriangleCount,
			PrimitiveSources::Vertex::Static::Source,
			PrimitiveSources::Triangle::Static::Source,
			FrustumCullingEnum::PrimitiveCulling,
			FaceCullingEnum::BackfaceCulling,
			PrimitiveSources::Albedo::Static::FullSource,
			PrimitiveSources::Material::DiffuseMaterialSource,
			PrimitiveSources::Normal::Static::NoSource,
			PrimitiveSources::Uv::Static::Source>
		{
		private:
			using Base = Mesh::VertexShadeObject<
				Cube::VertexCount, Cube::TriangleCount,
				PrimitiveSources::Vertex::Static::Source,
				PrimitiveSources::Triangle::Static::Source,
				FrustumCullingEnum::PrimitiveCulling,
				FaceCullingEnum::BackfaceCulling,
				PrimitiveSources::Albedo::Static::FullSource,
				PrimitiveSources::Material::DiffuseMaterialSource,
				PrimitiveSources::Normal::Static::NoSource,
				PrimitiveSources::Uv::Static::Source>;
		private:
			PrimitiveSources::Vertex::Static::Source VerticesSource;
			PrimitiveSources::Triangle::Static::Source TrianglesSource;
			PrimitiveSources::Uv::Static::Source UvsSource;

		public:
			CubeTexturedVertexObject()
				: VerticesSource(Cube::Vertices)
				, TrianglesSource(Cube::Triangles)
				, UvsSource(Assets::Uvs::Cube::UVs)
				, Base(VerticesSource, TrianglesSource,
					const_cast<PrimitiveSources::Albedo::Static::FullSource&>(PrimitiveSources::Albedo::FullAlbedoSourceInstance),
					const_cast<PrimitiveSources::Material::DiffuseMaterialSource&>(PrimitiveSources::Material::DiffuseMaterialSourceInstance),
					const_cast<PrimitiveSources::Normal::Static::NoSource&>(PrimitiveSources::Normal::NormalNoSourceInstance),
					UvsSource)
			{
			}
		};
#endif

		struct StarMeshTriangleObject
			: Mesh::SimpleStaticMeshTriangleObject<Star::VertexCount, Star::TriangleCount>
		{
			StarMeshTriangleObject() : Mesh::SimpleStaticMeshTriangleObject<Star::VertexCount, Star::TriangleCount>(
				Star::Vertices,
				Star::Triangles)
			{
			}
		};

		struct StarMeshVertexObject
			: Mesh::SimpleStaticMeshVertexObject<Star::VertexCount, Star::TriangleCount>
		{
			StarMeshVertexObject() : Mesh::SimpleStaticMeshVertexObject<Star::VertexCount, Star::TriangleCount>(
				Star::Vertices,
				Star::Triangles)
			{
			}
		};

		struct SphereMeshVertexObject : public Mesh::SimpleStaticMeshVertexObject<Sphere::VertexCount, Sphere::TriangleCount>
		{
			SphereMeshVertexObject() : Mesh::SimpleStaticMeshVertexObject<Sphere::VertexCount, Sphere::TriangleCount>(
				Sphere::Vertices,
				Sphere::Triangles)
			{
			}
		};

		class FloorPointCloudObject
			: public PointCloud::ShadeObject<
			Grid8x8::VertexCount,
			PrimitiveSources::Vertex::Static::Source,
			FrustumCullingEnum::PrimitiveCulling,
			FaceCullingEnum::NoCulling,
			PrimitiveSources::Albedo::Static::FullSource,
			PrimitiveSources::Material::DiffuseMaterialSource,
			PrimitiveSources::Normal::Dynamic::SingleSource>
		{
		private:
			using Base = PointCloud::ShadeObject<
				Grid8x8::VertexCount,
				PrimitiveSources::Vertex::Static::Source,
				FrustumCullingEnum::PrimitiveCulling,
				FaceCullingEnum::NoCulling,
				PrimitiveSources::Albedo::Static::FullSource,
				PrimitiveSources::Material::DiffuseMaterialSource,
				PrimitiveSources::Normal::Dynamic::SingleSource>;

		private:
			PrimitiveSources::Vertex::Static::Source VertexSource;
			PrimitiveSources::Normal::Dynamic::SingleSource NormalSource;

		public:
			FloorPointCloudObject()
				: VertexSource(Grid8x8::Vertices)
				, NormalSource()
				, Base(VertexSource,
					const_cast<PrimitiveSources::Albedo::Static::FullSource&>(PrimitiveSources::Albedo::FullAlbedoSourceInstance),
					const_cast<PrimitiveSources::Material::DiffuseMaterialSource&>(PrimitiveSources::Material::DiffuseMaterialSourceInstance),
					NormalSource)
			{
			}

			void SetNormal(const vertex16_t& normal)
			{
				NormalSource.Normal = normal;

				NormalizeVertex16(NormalSource.Normal);
			}
		};
	}

	namespace FragmentShaders
	{
		using namespace RenderObjects;

		struct FloorFragmentShader : IFragmentShader<point_cloud_fragment_t>
		{
		public:
			int16_t Radius = 1;

		public:
			void FragmentShade(WindowRasterizer& rasterizer, const point_cloud_fragment_t& fragment) final
			{
				const Rgb8::color_t color = Rgb8::Color(fragment.red, fragment.green, fragment.blue);
				rasterizer.DrawLine(color, fragment.x - Radius, fragment.y, fragment.x + Radius, fragment.y);
				rasterizer.DrawLine(color, fragment.x, fragment.y - Radius, fragment.x, fragment.y + Radius);
			}
		};

		struct LightBillboardFragmentShader : IFragmentShader<billboard_fragment_t>
		{
			void FragmentShade(WindowRasterizer& rasterizer, const billboard_fragment_t& fragment) final
			{
				if (fragment.red == 0 && fragment.green == 0 && fragment.blue == 0)
					return;

				const ufraction16_t nearFraction = UFRACTION16_1X - UFraction16::GetScalar<int16_t>(LimitValue<int16_t>(fragment.z, 0, VERTEX16_RANGE), VERTEX16_RANGE);
				const uint8_t innerComponentMax = Fraction(nearFraction, Rgb8::COMPONENT_MAX);

				Rgb8::component_t r = Fraction(nearFraction, fragment.red);
				Rgb8::component_t g = Fraction(nearFraction, fragment.green);
				Rgb8::component_t b = Fraction(nearFraction, fragment.blue);

				int_fast16_t radius = SignedRightShift(fragment.bottomRightY - fragment.topLeftY, 1);

				if (radius < 1)
					return;

				uint32_t radiusPower = int32_t(radius) * radius;

				// Limit radius to prevent overflow.
				if (radiusPower > INT16_MAX)
				{
					radiusPower = INT16_MAX;
					radius = SquareRoot16(INT16_MAX);
				}

				const int_fast16_t centerX = SignedRightShift(fragment.bottomRightX + fragment.topLeftX, 1);
				const int_fast16_t centerY = SignedRightShift(fragment.bottomRightY + fragment.topLeftY, 1);

				Rgb8::color_t color = 0;
				for (int_fast16_t y = 0; y < radius; y++)
				{
					for (int_fast16_t x = 0; x < radius; x++)
					{
						const uint32_t distancePower = ((uint32_t(x) * (x)) + (uint_fast16_t(y) * (y)));
						const ufraction8_t distanceFraction = UFraction8::GetScalar(distancePower, radiusPower);
						const ufraction8_t proximityFraction = UFRACTION8_1X - distanceFraction;

						if (distanceFraction < UFRACTION8_1X)
						{
							const uint8_t innerComponent = Fraction(proximityFraction, innerComponentMax);
							color = Rgb8::Color(
								Fraction(proximityFraction, static_cast<uint8_t>(UINT8_MAX)),
								static_cast<uint8_t>(Fraction(proximityFraction, MinValue<uint16_t>(UINT8_MAX, static_cast<uint16_t>(r) + innerComponent))),
								static_cast<uint8_t>(Fraction(proximityFraction, MinValue<uint16_t>(UINT8_MAX, static_cast<uint16_t>(g) + innerComponent))),
								static_cast<uint8_t>(Fraction(proximityFraction, MinValue<uint16_t>(UINT8_MAX, static_cast<uint16_t>(b) + innerComponent))));

							rasterizer.BlendPixel<pixel_blend_mode_t::Add>(color, centerX + x, centerY + y);
							rasterizer.BlendPixel<pixel_blend_mode_t::Add>(color, centerX - x, centerY + y);
							if (y != 0)
							{
								rasterizer.BlendPixel<pixel_blend_mode_t::Add>(color, centerX + x, centerY - y);
								rasterizer.BlendPixel<pixel_blend_mode_t::Add>(color, centerX - x, centerY - y);
							}
						}
					}
				}
			}
		};

#if defined(INTEGER_WORLD_TEXTURED_CUBE_DEMO)

#if defined(INTEGER_WORLD_TEXTURED_CUBE_HIGH_QUALITY)
		using UvInterpolatorType = typename PrimitiveShaders::UvInterpolatorPerspectiveCorrect;
#else
		using UvInterpolatorType = typename PrimitiveShaders::UvInterpolatorFast;
#endif

		using TextureTriangleLitFunctor = Mesh::FragmentShaders::Functors::Texture::TriangleLitFunctor<
			PrimitiveSources::Texture::Static::Source,
			UvInterpolatorType>;

		class CubeTexturedTriangleLitShader
			: public Mesh::FragmentShaders::TriangleShade::TemplateTextureShader<
			PrimitiveSources::Texture::Static::Source,
			TextureTriangleLitFunctor>
		{
		private:
			PrimitiveSources::Texture::Static::Source TextureSource;

		public:
			CubeTexturedTriangleLitShader()
				: TextureSource(Texture::Cube::Atlas, Texture::Cube::Width)
				, Mesh::FragmentShaders::TriangleShade::TemplateTextureShader<
				PrimitiveSources::Texture::Static::Source,
				TextureTriangleLitFunctor>(TextureSource)
			{
			}
		};

		using TextureVertexLitFunctor = Mesh::FragmentShaders::Functors::Texture::VertexLitFunctor<
			PrimitiveSources::Texture::Static::Source,
			UvInterpolatorType>;

		class CubeTexturedVertexLitShader
			: public Mesh::FragmentShaders::VertexShade::TemplateTextureShader<
			PrimitiveSources::Texture::Static::Source,
			TextureVertexLitFunctor>
		{
		private:
			PrimitiveSources::Texture::Static::Source TextureSource;

		public:
			CubeTexturedVertexLitShader()
				: TextureSource(Texture::Cube::Atlas, Texture::Cube::Width)
				, Mesh::FragmentShaders::VertexShade::TemplateTextureShader<
				PrimitiveSources::Texture::Static::Source,
				TextureVertexLitFunctor>(TextureSource)
			{
			}
		};
#endif
	}
}
#endif