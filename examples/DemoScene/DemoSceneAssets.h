#ifndef _DEMO_SCENE_ASSETS_h
#define _DEMO_SCENE_ASSETS_h

#include <IntegerWorld.h>

namespace Assets
{
	using namespace IntegerWorld;

	namespace Palletes
	{
		namespace Cube
		{
			static constexpr Rgb8::color_t Pallete[Shapes::Cube::TriangleCount]
			{
				0xFF0000,
				0x00FF00,
				0x0000FF,
				0xFFFF00,
				0x00FFFF,
				0xFF00FF
			};

			constexpr uint8_t PalleteSize = sizeof(Pallete) / sizeof(Pallete[0]);
		}
	}

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
				{-SHAPE_UNIT / 28, -SHAPE_UNIT / 28, 0},
				{ SHAPE_UNIT / 28, -SHAPE_UNIT / 28, 0},
				{ SHAPE_UNIT * 3 / 28, -SHAPE_UNIT / 28, 0},
				{ SHAPE_UNIT * 5 / 28, -SHAPE_UNIT / 28, 0},
				{ SHAPE_UNIT * 7 / 28, -SHAPE_UNIT / 28, 0},
				{ SHAPE_UNIT * 9 / 28, -SHAPE_UNIT / 28, 0},
				{ SHAPE_UNIT * 11 / 28, -SHAPE_UNIT / 28, 0},
				{ SHAPE_UNIT * 13 / 28, -SHAPE_UNIT / 28, 0},
				{-SHAPE_UNIT / 28,  SHAPE_UNIT / 28, 0},
				{ SHAPE_UNIT / 28,  SHAPE_UNIT / 28, 0},
				{ SHAPE_UNIT * 3 / 28,  SHAPE_UNIT / 28, 0},
				{ SHAPE_UNIT * 5 / 28,  SHAPE_UNIT / 28, 0},
				{ SHAPE_UNIT * 7 / 28,  SHAPE_UNIT / 28, 0},
				{ SHAPE_UNIT * 9 / 28,  SHAPE_UNIT / 28, 0},
				{ SHAPE_UNIT * 11 / 28,  SHAPE_UNIT / 28, 0},
				{ SHAPE_UNIT * 13 / 28,  SHAPE_UNIT / 28, 0},
				{-SHAPE_UNIT / 28,  SHAPE_UNIT * 3 / 28, 0},
				{ SHAPE_UNIT / 28,  SHAPE_UNIT * 3 / 28, 0},
				{ SHAPE_UNIT * 3 / 28,  SHAPE_UNIT * 3 / 28, 0},
				{ SHAPE_UNIT * 5 / 28,  SHAPE_UNIT * 3 / 28, 0},
				{ SHAPE_UNIT * 7 / 28,  SHAPE_UNIT * 3 / 28, 0},
				{ SHAPE_UNIT * 9 / 28,  SHAPE_UNIT * 3 / 28, 0},
				{ SHAPE_UNIT * 11 / 28,  SHAPE_UNIT * 3 / 28, 0},
				{ SHAPE_UNIT * 13 / 28,  SHAPE_UNIT * 3 / 28, 0},
				{-SHAPE_UNIT / 28,  SHAPE_UNIT * 5 / 28, 0},
				{ SHAPE_UNIT / 28,  SHAPE_UNIT * 5 / 28, 0},
				{ SHAPE_UNIT * 3 / 28,  SHAPE_UNIT * 5 / 28, 0},
				{ SHAPE_UNIT * 5 / 28,  SHAPE_UNIT * 5 / 28, 0},
				{ SHAPE_UNIT * 7 / 28,  SHAPE_UNIT * 5 / 28, 0},
				{ SHAPE_UNIT * 9 / 28,  SHAPE_UNIT * 5 / 28, 0},
				{ SHAPE_UNIT * 11 / 28,  SHAPE_UNIT * 5 / 28, 0},
				{ SHAPE_UNIT * 13 / 28,  SHAPE_UNIT * 5 / 28, 0},
				{-SHAPE_UNIT / 28,  SHAPE_UNIT * 7 / 28, 0},
				{ SHAPE_UNIT / 28,  SHAPE_UNIT * 7 / 28, 0},
				{ SHAPE_UNIT * 3 / 28,  SHAPE_UNIT * 7 / 28, 0},
				{ SHAPE_UNIT * 5 / 28,  SHAPE_UNIT * 7 / 28, 0},
				{ SHAPE_UNIT * 7 / 28,  SHAPE_UNIT * 7 / 28, 0},
				{ SHAPE_UNIT * 9 / 28,  SHAPE_UNIT * 7 / 28, 0},
				{ SHAPE_UNIT * 11 / 28,  SHAPE_UNIT * 7 / 28, 0},
				{ SHAPE_UNIT * 13 / 28,  SHAPE_UNIT * 7 / 28, 0},
				{-SHAPE_UNIT / 28,  SHAPE_UNIT * 9 / 28, 0},
				{ SHAPE_UNIT / 28,  SHAPE_UNIT * 9 / 28, 0},
				{ SHAPE_UNIT * 3 / 28,  SHAPE_UNIT * 9 / 28, 0},
				{ SHAPE_UNIT * 5 / 28,  SHAPE_UNIT * 9 / 28, 0},
				{ SHAPE_UNIT * 7 / 28,  SHAPE_UNIT * 9 / 28, 0},
				{ SHAPE_UNIT * 9 / 28,  SHAPE_UNIT * 9 / 28, 0},
				{ SHAPE_UNIT * 11 / 28,  SHAPE_UNIT * 9 / 28, 0},
				{ SHAPE_UNIT * 13 / 28,  SHAPE_UNIT * 9 / 28, 0},
				{-SHAPE_UNIT / 28,  SHAPE_UNIT * 11 / 28, 0},
				{ SHAPE_UNIT / 28,  SHAPE_UNIT * 11 / 28, 0},
				{ SHAPE_UNIT * 3 / 28,  SHAPE_UNIT * 11 / 28, 0},
				{ SHAPE_UNIT * 5 / 28,  SHAPE_UNIT * 11 / 28, 0},
				{ SHAPE_UNIT * 7 / 28,  SHAPE_UNIT * 11 / 28, 0},
				{ SHAPE_UNIT * 9 / 28,  SHAPE_UNIT * 11 / 28, 0},
				{ SHAPE_UNIT * 11 / 28,  SHAPE_UNIT * 11 / 28, 0},
				{ SHAPE_UNIT * 13 / 28,  SHAPE_UNIT * 11 / 28, 0},
				{-SHAPE_UNIT / 28,  SHAPE_UNIT * 13 / 28, 0},
				{ SHAPE_UNIT / 28,  SHAPE_UNIT * 13 / 28, 0},
				{ SHAPE_UNIT * 3 / 28,  SHAPE_UNIT * 13 / 28, 0},
				{ SHAPE_UNIT * 5 / 28,  SHAPE_UNIT * 13 / 28, 0},
				{ SHAPE_UNIT * 7 / 28,  SHAPE_UNIT * 13 / 28, 0},
				{ SHAPE_UNIT * 9 / 28,  SHAPE_UNIT * 13 / 28, 0},
				{ SHAPE_UNIT * 11 / 28,  SHAPE_UNIT * 13 / 28, 0},
				{ SHAPE_UNIT * 13 / 28,  SHAPE_UNIT * 13 / 28, 0}
			};

			constexpr uint8_t VertexCount = sizeof(Vertices) / sizeof(vertex16_t);
		}

		namespace SphereLod
		{
			namespace LoD0
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

			namespace LoD1
			{
				static constexpr int16_t UpSize = 105;

				static constexpr int16_t DownSize = 100;

				static constexpr vertex16_t Vertices[] PROGMEM
				{
					{(UpSize * (int32_t)(0)) / DownSize , (UpSize * (int32_t)(1089)) / DownSize , (UpSize * (int32_t)(544)) / DownSize},
					{(UpSize * (int32_t)(-1036)) / DownSize , (UpSize * (int32_t)(337)) / DownSize , (UpSize * (int32_t)(544)) / DownSize},
					{(UpSize * (int32_t)(0)) / DownSize , (UpSize * (int32_t)(0)) / DownSize , (UpSize * (int32_t)(1217)) / DownSize},
					{(UpSize * (int32_t)(1036)) / DownSize , (UpSize * (int32_t)(337)) / DownSize , (UpSize * (int32_t)(544)) / DownSize},
					{(UpSize * (int32_t)(640)) / DownSize , (UpSize * (int32_t)(-881)) / DownSize , (UpSize * (int32_t)(544)) / DownSize},
					{(UpSize * (int32_t)(-640)) / DownSize , (UpSize * (int32_t)(-881)) / DownSize , (UpSize * (int32_t)(544)) / DownSize},
					{(UpSize * (int32_t)(0)) / DownSize , (UpSize * (int32_t)(0)) / DownSize , (UpSize * (int32_t)(-1217)) / DownSize},
					{(UpSize * (int32_t)(1036)) / DownSize , (UpSize * (int32_t)(-337)) / DownSize , (UpSize * (int32_t)(-544)) / DownSize},
					{(UpSize * (int32_t)(0)) / DownSize , (UpSize * (int32_t)(-1089)) / DownSize , (UpSize * (int32_t)(-544)) / DownSize},
					{(UpSize * (int32_t)(-1036)) / DownSize , (UpSize * (int32_t)(-337)) / DownSize , (UpSize * (int32_t)(-544)) / DownSize},
					{(UpSize * (int32_t)(-640)) / DownSize , (UpSize * (int32_t)(881)) / DownSize , (UpSize * (int32_t)(-544)) / DownSize},
					{(UpSize * (int32_t)(640)) / DownSize , (UpSize * (int32_t)(881)) / DownSize , (UpSize * (int32_t)(-544)) / DownSize},
				};

				constexpr auto VertexCount = sizeof(Vertices) / sizeof(vertex16_t);

				static constexpr triangle_face_t Triangles[] PROGMEM
				{
					{ 0, 1, 2 },
					{ 3, 0, 2 },
					{ 2, 4, 3 },
					{ 2, 5, 4 },
					{ 1, 5, 2 },
					{ 6, 7, 8 },
					{ 8, 9, 6 },
					{ 9, 10, 6 },
					{ 10, 11, 6 },
					{ 11, 7, 6 },
					{ 11, 0, 3 },
					{ 1, 10, 9 },
					{ 3, 7, 11 },
					{ 1, 0, 10 },
					{ 3, 4, 7 },
					{ 0, 11, 10 },
					{ 4, 8, 7 },
					{ 4, 5, 8 },
					{ 5, 9, 8 },
					{ 5, 1, 9 },
				};

				constexpr auto TriangleCount = sizeof(Triangles) / sizeof(triangle_face_t);
			}
		}
	}

	namespace Shaders
	{
		struct LightBillboardFragmentShader : IFragmentShader<billboard_fragment_t>
		{
			ILightSource* LightSource = nullptr;

			void FragmentShade(WindowRasterizer& rasterizer, const billboard_fragment_t& fragment, ISceneShader* sceneShader) final
			{
				FragmentShade(rasterizer, fragment);
			}

			void FragmentShade(WindowRasterizer& rasterizer, const billboard_fragment_t& fragment) final
			{
				Rgb8::color_t lightColor{};
				if (LightSource == nullptr)
					return;
				else
					LightSource->GetLightColor(lightColor);

				if (lightColor == 0)
					return;

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
						const ufraction8_t distanceFraction = Fraction::GetUFraction8(distancePower, radiusPower);
						const ufraction8_t proximityFraction = UFRACTION8_1X - distanceFraction;

						if (distanceFraction < UFRACTION8_1X)
						{
							/*const uint8_t innerComponent = Fraction::Scale(proximityFraction, Rgb8::COMPONENT_MAX);
							color = Rgb8::Color(
								Fraction::Scale(proximityFraction, uint8_t(UINT8_MAX)),
								uint8_t(Fraction::Scale(distanceFraction, Rgb8::Red(lightColor)) + innerComponent),
								uint8_t(Fraction::Scale(distanceFraction, Rgb8::Green(lightColor)) + innerComponent),
								uint8_t(Fraction::Scale(distanceFraction, Rgb8::Blue(lightColor)) + innerComponent));
							rasterizer.BlendPixel<pixel_blend_mode_t::Alpha>(color, centerX + x, centerY + y);
							rasterizer.BlendPixel<pixel_blend_mode_t::Alpha>(color, centerX - x, centerY + y);
							rasterizer.BlendPixel<pixel_blend_mode_t::Alpha>(color, centerX + x, centerY - y);
							rasterizer.BlendPixel<pixel_blend_mode_t::Alpha>(color, centerX - x, centerY - y);*/

							const uint8_t innerComponent = Fraction::Scale(proximityFraction, Rgb8::COMPONENT_MAX);
							color = Rgb8::Color(
								Fraction::Scale(proximityFraction, uint8_t(Fraction::Scale(distanceFraction, Rgb8::Red(lightColor)) + innerComponent)),
								Fraction::Scale(proximityFraction, uint8_t(Fraction::Scale(distanceFraction, Rgb8::Green(lightColor)) + innerComponent)),
								Fraction::Scale(proximityFraction, uint8_t(Fraction::Scale(distanceFraction, Rgb8::Blue(lightColor)) + innerComponent)));
							rasterizer.BlendPixel<pixel_blend_mode_t::Add>(color, centerX + x, centerY + y);
							rasterizer.BlendPixel<pixel_blend_mode_t::Add>(color, centerX - x, centerY + y);
							rasterizer.BlendPixel<pixel_blend_mode_t::Add>(color, centerX + x, centerY - y);
							rasterizer.BlendPixel<pixel_blend_mode_t::Add>(color, centerX - x, centerY - y);
						}
					}
				}
			}
		};

		struct FloorFragmentShader : IFragmentShader<point_fragment_t>
		{
		private:
			world_position_shade_t Shade{};
			Rgb8::color_t FragmentColor{};

		public:
			int16_t Radius = 1;

		public:
			void FragmentShade(WindowRasterizer& rasterizer, const point_fragment_t& fragment, ISceneShader* sceneShader) final
			{
				FragmentColor = fragment.color;
				Shade.positionWorld = fragment.world;
				sceneShader->Shade(FragmentColor, fragment.material, Shade);
				rasterizer.DrawLine(FragmentColor, fragment.screen.x - Radius, fragment.screen.y, fragment.screen.x + Radius, fragment.screen.y);
				rasterizer.DrawLine(FragmentColor, fragment.screen.x, fragment.screen.y - Radius, fragment.screen.x, fragment.screen.y + Radius);
			}

			void FragmentShade(WindowRasterizer& rasterizer, const point_fragment_t& fragment) final
			{
				FragmentColor = fragment.color;
				rasterizer.DrawLine(FragmentColor, fragment.screen.x - Radius, fragment.screen.y, fragment.screen.x + Radius, fragment.screen.y);
				rasterizer.DrawLine(FragmentColor, fragment.screen.x, fragment.screen.y - Radius, fragment.screen.x, fragment.screen.y + Radius);
			}
		};
	}

	namespace Objects
	{
		using namespace Shapes;

		struct StarMeshObject : public StaticMeshSingleColorSingleMaterialObject<Star::VertexCount, Star::TriangleCount, FrustumCullingEnum::PrimitiveCulling>
		{
			StarMeshObject() : StaticMeshSingleColorSingleMaterialObject<Star::VertexCount, Star::TriangleCount, FrustumCullingEnum::PrimitiveCulling>(
				Star::Vertices,
				Star::Triangles) {
			}
		};

		struct CubeMeshObject : public StaticMeshObject<Cube::VertexCount, Cube::TriangleCount, FrustumCullingEnum::PrimitiveCulling>
		{
			material_t Material{ 0, UFRACTION8_1X, 0, 0 };

			CubeMeshObject() : StaticMeshObject<Cube::VertexCount, Cube::TriangleCount, FrustumCullingEnum::PrimitiveCulling>(
				Cube::Vertices,
				Cube::Triangles) {
			}

		protected:
			virtual void GetFragment(triangle_fragment_t& fragment, const uint16_t index)
			{
				fragment.color = Palletes::Cube::Pallete[(index / 2) % Palletes::Cube::PalleteSize];
				fragment.material = Material;
			}
		};

		class SphereMeshLodObject : public StaticMeshLodSingleColorSingleMaterialObject<Assets::Shapes::SphereLod::LoD0::VertexCount,
			Assets::Shapes::SphereLod::LoD0::TriangleCount, 2, FrustumCullingEnum::PrimitiveCulling>
		{
		private:
			using Base = StaticMeshLodSingleColorSingleMaterialObject<Assets::Shapes::SphereLod::LoD0::VertexCount,
				Assets::Shapes::SphereLod::LoD0::TriangleCount, 2, FrustumCullingEnum::PrimitiveCulling>;

		public:
			SphereMeshLodObject() : Base()
			{
				Base::SetSourcesLevelOfDetail(16000,
					Assets::Shapes::SphereLod::LoD0::Vertices, Assets::Shapes::SphereLod::LoD0::VertexCount,
					Assets::Shapes::SphereLod::LoD0::Triangles, Assets::Shapes::SphereLod::LoD0::TriangleCount);

				Base::SetSourcesLevelOfDetail(VERTEX16_RANGE,
					Assets::Shapes::SphereLod::LoD1::Vertices, Assets::Shapes::SphereLod::LoD1::VertexCount,
					Assets::Shapes::SphereLod::LoD1::Triangles, Assets::Shapes::SphereLod::LoD1::TriangleCount);

				SetRenderDistanceMinimum(0);
			}
		};

		struct FloorPointCloudObject : public FlatPointCloudObject<Grid8x8::VertexCount, FrustumCullingEnum::PrimitiveCulling>
		{
			FloorPointCloudObject() : FlatPointCloudObject<Grid8x8::VertexCount,
				FrustumCullingEnum::PrimitiveCulling>
				(Grid8x8::Vertices)
			{
			}
		};
	}

	namespace Lights
	{
		template<typename ShaderType>
		struct TemplateOnOffShader : ShaderType
		{
		private:
			bool Enabled = true;

		public:
			void FragmentShade(WindowRasterizer& rasterizer, const billboard_fragment_t& fragment, ISceneShader* sceneShader)
			{
				if (Enabled)
					ShaderType::FragmentShade(rasterizer, fragment, sceneShader);
			}

			void FragmentShade(WindowRasterizer& rasterizer, const billboard_fragment_t& fragment)
			{
				if (Enabled)
					ShaderType::FragmentShade(rasterizer, fragment);
			}

			void SetEnabled(const bool enabled)
			{
				Enabled = enabled;
			}
		};
	}
}

#endif