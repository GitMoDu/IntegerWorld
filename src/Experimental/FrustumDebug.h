#ifndef _INTEGER_WORLD_ASSETS_FRUSTUM_DEBUG_h
#define _INTEGER_WORLD_ASSETS_FRUSTUM_DEBUG_h

#if defined(INTEGER_WORLD_FRUSTUM_DEBUG)
#include <IntegerWorld.h>

namespace Assets
{
	using namespace IntegerWorld;

	namespace Shapes
	{
		namespace Camera
		{
			static constexpr int16_t UpSize = 1;

			static constexpr int16_t DownSize = 8;

			static constexpr int16_t ZShift = (-195 * DownSize) / UpSize;

			static constexpr vertex16_t Vertices[] PROGMEM
			{
				{(UpSize * (int32_t)(-305)) / DownSize , (UpSize * (int32_t)(-453)) / DownSize , (UpSize * (int32_t)(833 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(-305)) / DownSize , (UpSize * (int32_t)(443)) / DownSize , (UpSize * (int32_t)(833 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(-305)) / DownSize , (UpSize * (int32_t)(-453)) / DownSize , (UpSize * (int32_t)(-819 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(-305)) / DownSize , (UpSize * (int32_t)(443)) / DownSize , (UpSize * (int32_t)(-819 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(318)) / DownSize , (UpSize * (int32_t)(443)) / DownSize , (UpSize * (int32_t)(-819 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(318)) / DownSize , (UpSize * (int32_t)(-453)) / DownSize , (UpSize * (int32_t)(-819 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(454)) / DownSize , (UpSize * (int32_t)(-338)) / DownSize , (UpSize * (int32_t)(1485 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(454)) / DownSize , (UpSize * (int32_t)(329)) / DownSize , (UpSize * (int32_t)(1485 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(328)) / DownSize , (UpSize * (int32_t)(-241)) / DownSize , (UpSize * (int32_t)(1485 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(328)) / DownSize , (UpSize * (int32_t)(231)) / DownSize , (UpSize * (int32_t)(1485 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(318)) / DownSize , (UpSize * (int32_t)(-453)) / DownSize , (UpSize * (int32_t)(833 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(318)) / DownSize , (UpSize * (int32_t)(443)) / DownSize , (UpSize * (int32_t)(833 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(-122)) / DownSize , (UpSize * (int32_t)(-453)) / DownSize , (UpSize * (int32_t)(-277 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(-122)) / DownSize , (UpSize * (int32_t)(-456)) / DownSize , (UpSize * (int32_t)(-776 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(-122)) / DownSize , (UpSize * (int32_t)(-842)) / DownSize , (UpSize * (int32_t)(-1088 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(-122)) / DownSize , (UpSize * (int32_t)(-1321)) / DownSize , (UpSize * (int32_t)(-978 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(-122)) / DownSize , (UpSize * (int32_t)(-1531)) / DownSize , (UpSize * (int32_t)(-528 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(-122)) / DownSize , (UpSize * (int32_t)(-1315)) / DownSize , (UpSize * (int32_t)(-78 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(134)) / DownSize , (UpSize * (int32_t)(-1315)) / DownSize , (UpSize * (int32_t)(-78 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(134)) / DownSize , (UpSize * (int32_t)(-1531)) / DownSize , (UpSize * (int32_t)(-528 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(134)) / DownSize , (UpSize * (int32_t)(-1321)) / DownSize , (UpSize * (int32_t)(-978 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(134)) / DownSize , (UpSize * (int32_t)(-842)) / DownSize , (UpSize * (int32_t)(-1088 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(134)) / DownSize , (UpSize * (int32_t)(-456)) / DownSize , (UpSize * (int32_t)(-776 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(134)) / DownSize , (UpSize * (int32_t)(-453)) / DownSize , (UpSize * (int32_t)(-277 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(-122)) / DownSize , (UpSize * (int32_t)(-880)) / DownSize , (UpSize * (int32_t)(24 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(-122)) / DownSize , (UpSize * (int32_t)(-827)) / DownSize , (UpSize * (int32_t)(27 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(-122)) / DownSize , (UpSize * (int32_t)(-835)) / DownSize , (UpSize * (int32_t)(1137 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(-122)) / DownSize , (UpSize * (int32_t)(-453)) / DownSize , (UpSize * (int32_t)(826 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(-122)) / DownSize , (UpSize * (int32_t)(-456)) / DownSize , (UpSize * (int32_t)(327 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(-122)) / DownSize , (UpSize * (int32_t)(-1321)) / DownSize , (UpSize * (int32_t)(125 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(-122)) / DownSize , (UpSize * (int32_t)(-1531)) / DownSize , (UpSize * (int32_t)(575 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(-122)) / DownSize , (UpSize * (int32_t)(-1315)) / DownSize , (UpSize * (int32_t)(1025 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(134)) / DownSize , (UpSize * (int32_t)(-880)) / DownSize , (UpSize * (int32_t)(24 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(134)) / DownSize , (UpSize * (int32_t)(-827)) / DownSize , (UpSize * (int32_t)(27 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(134)) / DownSize , (UpSize * (int32_t)(-453)) / DownSize , (UpSize * (int32_t)(826 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(134)) / DownSize , (UpSize * (int32_t)(-835)) / DownSize , (UpSize * (int32_t)(1137 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(134)) / DownSize , (UpSize * (int32_t)(-1315)) / DownSize , (UpSize * (int32_t)(1025 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(134)) / DownSize , (UpSize * (int32_t)(-1531)) / DownSize , (UpSize * (int32_t)(575 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(134)) / DownSize , (UpSize * (int32_t)(-1321)) / DownSize , (UpSize * (int32_t)(125 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(134)) / DownSize , (UpSize * (int32_t)(-456)) / DownSize , (UpSize * (int32_t)(327 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(167)) / DownSize , (UpSize * (int32_t)(-153)) / DownSize , (UpSize * (int32_t)(844 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(167)) / DownSize , (UpSize * (int32_t)(144)) / DownSize , (UpSize * (int32_t)(844 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(293)) / DownSize , (UpSize * (int32_t)(242)) / DownSize , (UpSize * (int32_t)(844 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(293)) / DownSize , (UpSize * (int32_t)(-251)) / DownSize , (UpSize * (int32_t)(844 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(-442)) / DownSize , (UpSize * (int32_t)(329)) / DownSize , (UpSize * (int32_t)(1485 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(-442)) / DownSize , (UpSize * (int32_t)(-338)) / DownSize , (UpSize * (int32_t)(1485 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(-154)) / DownSize , (UpSize * (int32_t)(-153)) / DownSize , (UpSize * (int32_t)(844 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(-154)) / DownSize , (UpSize * (int32_t)(144)) / DownSize , (UpSize * (int32_t)(844 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(-316)) / DownSize , (UpSize * (int32_t)(-241)) / DownSize , (UpSize * (int32_t)(1485 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(-316)) / DownSize , (UpSize * (int32_t)(231)) / DownSize , (UpSize * (int32_t)(1485 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(-280)) / DownSize , (UpSize * (int32_t)(-251)) / DownSize , (UpSize * (int32_t)(844 + ZShift)) / DownSize},
				{(UpSize * (int32_t)(-280)) / DownSize , (UpSize * (int32_t)(242)) / DownSize , (UpSize * (int32_t)(844 + ZShift)) / DownSize},
			};

			static constexpr vertex16_t Vertices2[] PROGMEM
			{
				{(UpSize * (int32_t)(-305)) / DownSize , (UpSize * (int32_t)(-453)) / DownSize , ((UpSize * (int32_t)(833)) / DownSize) - 190},
				{(UpSize * (int32_t)(-305)) / DownSize , (UpSize * (int32_t)(443)) / DownSize , ((UpSize * (int32_t)(833)) / DownSize) - 190},
				{(UpSize * (int32_t)(-305)) / DownSize , (UpSize * (int32_t)(-453)) / DownSize , ((UpSize * (int32_t)(-819)) / DownSize) - 190},
				{(UpSize * (int32_t)(-305)) / DownSize , (UpSize * (int32_t)(443)) / DownSize , ((UpSize * (int32_t)(-819)) / DownSize) - 190},
				{(UpSize * (int32_t)(318)) / DownSize , (UpSize * (int32_t)(443)) / DownSize , ((UpSize * (int32_t)(-819)) / DownSize) - 190},
				{(UpSize * (int32_t)(318)) / DownSize , (UpSize * (int32_t)(-453)) / DownSize , ((UpSize * (int32_t)(-819)) / DownSize) - 190},
				{(UpSize * (int32_t)(454)) / DownSize , (UpSize * (int32_t)(-338)) / DownSize , ((UpSize * (int32_t)(1485)) / DownSize) - 190},
				{(UpSize * (int32_t)(454)) / DownSize , (UpSize * (int32_t)(329)) / DownSize , ((UpSize * (int32_t)(1485)) / DownSize) - 190},
				{(UpSize * (int32_t)(328)) / DownSize , (UpSize * (int32_t)(-241)) / DownSize , ((UpSize * (int32_t)(1485)) / DownSize) - 190},
				{(UpSize * (int32_t)(328)) / DownSize , (UpSize * (int32_t)(231)) / DownSize , ((UpSize * (int32_t)(1485)) / DownSize) - 190},
				{(UpSize * (int32_t)(318)) / DownSize , (UpSize * (int32_t)(-453)) / DownSize , ((UpSize * (int32_t)(833)) / DownSize) - 190},
				{(UpSize * (int32_t)(318)) / DownSize , (UpSize * (int32_t)(443)) / DownSize , ((UpSize * (int32_t)(833)) / DownSize) - 190},
				{(UpSize * (int32_t)(-122)) / DownSize , (UpSize * (int32_t)(-453)) / DownSize , ((UpSize * (int32_t)(-277)) / DownSize) - 190},
				{(UpSize * (int32_t)(-122)) / DownSize , (UpSize * (int32_t)(-456)) / DownSize , ((UpSize * (int32_t)(-776)) / DownSize) - 190},
				{(UpSize * (int32_t)(-122)) / DownSize , (UpSize * (int32_t)(-842)) / DownSize , ((UpSize * (int32_t)(-1088)) / DownSize) - 190},
				{(UpSize * (int32_t)(-122)) / DownSize , (UpSize * (int32_t)(-1321)) / DownSize , ((UpSize * (int32_t)(-978)) / DownSize) - 190},
				{(UpSize * (int32_t)(-122)) / DownSize , (UpSize * (int32_t)(-1531)) / DownSize , ((UpSize * (int32_t)(-528)) / DownSize) - 190},
				{(UpSize * (int32_t)(-122)) / DownSize , (UpSize * (int32_t)(-1315)) / DownSize , ((UpSize * (int32_t)(-78)) / DownSize) - 190},
				{(UpSize * (int32_t)(134)) / DownSize , (UpSize * (int32_t)(-1315)) / DownSize , ((UpSize * (int32_t)(-78)) / DownSize) - 190},
				{(UpSize * (int32_t)(134)) / DownSize , (UpSize * (int32_t)(-1531)) / DownSize , ((UpSize * (int32_t)(-528)) / DownSize) - 190},
				{(UpSize * (int32_t)(134)) / DownSize , (UpSize * (int32_t)(-1321)) / DownSize , ((UpSize * (int32_t)(-978)) / DownSize) - 190},
				{(UpSize * (int32_t)(134)) / DownSize , (UpSize * (int32_t)(-842)) / DownSize , ((UpSize * (int32_t)(-1088)) / DownSize) - 190},
				{(UpSize * (int32_t)(134)) / DownSize , (UpSize * (int32_t)(-456)) / DownSize , ((UpSize * (int32_t)(-776)) / DownSize) - 190},
				{(UpSize * (int32_t)(134)) / DownSize , (UpSize * (int32_t)(-453)) / DownSize , ((UpSize * (int32_t)(-277)) / DownSize) - 190},
				{(UpSize * (int32_t)(-122)) / DownSize , (UpSize * (int32_t)(-880)) / DownSize , ((UpSize * (int32_t)(24)) / DownSize) - 190},
				{(UpSize * (int32_t)(-122)) / DownSize , (UpSize * (int32_t)(-827)) / DownSize , ((UpSize * (int32_t)(27)) / DownSize) - 190},
				{(UpSize * (int32_t)(-122)) / DownSize , (UpSize * (int32_t)(-835)) / DownSize , ((UpSize * (int32_t)(1137)) / DownSize) - 190},
				{(UpSize * (int32_t)(-122)) / DownSize , (UpSize * (int32_t)(-453)) / DownSize , ((UpSize * (int32_t)(826)) / DownSize) - 190},
				{(UpSize * (int32_t)(-122)) / DownSize , (UpSize * (int32_t)(-456)) / DownSize , ((UpSize * (int32_t)(327)) / DownSize) - 190},
				{(UpSize * (int32_t)(-122)) / DownSize , (UpSize * (int32_t)(-1321)) / DownSize , ((UpSize * (int32_t)(125)) / DownSize) - 190},
				{(UpSize * (int32_t)(-122)) / DownSize , (UpSize * (int32_t)(-1531)) / DownSize , ((UpSize * (int32_t)(575)) / DownSize) - 190},
				{(UpSize * (int32_t)(-122)) / DownSize , (UpSize * (int32_t)(-1315)) / DownSize , ((UpSize * (int32_t)(1025)) / DownSize) - 190},
				{(UpSize * (int32_t)(134)) / DownSize , (UpSize * (int32_t)(-880)) / DownSize , ((UpSize * (int32_t)(24)) / DownSize) - 190},
				{(UpSize * (int32_t)(134)) / DownSize , (UpSize * (int32_t)(-827)) / DownSize , ((UpSize * (int32_t)(27)) / DownSize) - 190},
				{(UpSize * (int32_t)(134)) / DownSize , (UpSize * (int32_t)(-453)) / DownSize , ((UpSize * (int32_t)(826)) / DownSize) - 190},
				{(UpSize * (int32_t)(134)) / DownSize , (UpSize * (int32_t)(-835)) / DownSize , ((UpSize * (int32_t)(1137)) / DownSize) - 190},
				{(UpSize * (int32_t)(134)) / DownSize , (UpSize * (int32_t)(-1315)) / DownSize , ((UpSize * (int32_t)(1025)) / DownSize) - 190},
				{(UpSize * (int32_t)(134)) / DownSize , (UpSize * (int32_t)(-1531)) / DownSize , ((UpSize * (int32_t)(575)) / DownSize) - 190},
				{(UpSize * (int32_t)(134)) / DownSize , (UpSize * (int32_t)(-1321)) / DownSize , ((UpSize * (int32_t)(125)) / DownSize) - 190},
				{(UpSize * (int32_t)(134)) / DownSize , (UpSize * (int32_t)(-456)) / DownSize , ((UpSize * (int32_t)(327)) / DownSize) - 190},
				{(UpSize * (int32_t)(167)) / DownSize , (UpSize * (int32_t)(-153)) / DownSize , ((UpSize * (int32_t)(844)) / DownSize) - 190},
				{(UpSize * (int32_t)(167)) / DownSize , (UpSize * (int32_t)(144)) / DownSize , ((UpSize * (int32_t)(844)) / DownSize) - 190},
				{(UpSize * (int32_t)(293)) / DownSize , (UpSize * (int32_t)(242)) / DownSize , ((UpSize * (int32_t)(844)) / DownSize) - 190},
				{(UpSize * (int32_t)(293)) / DownSize , (UpSize * (int32_t)(-251)) / DownSize , ((UpSize * (int32_t)(844)) / DownSize) - 190},
				{(UpSize * (int32_t)(-442)) / DownSize , (UpSize * (int32_t)(329)) / DownSize , ((UpSize * (int32_t)(1485)) / DownSize) - 190},
				{(UpSize * (int32_t)(-442)) / DownSize , (UpSize * (int32_t)(-338)) / DownSize , ((UpSize * (int32_t)(1485)) / DownSize) - 190},
				{(UpSize * (int32_t)(-154)) / DownSize , (UpSize * (int32_t)(-153)) / DownSize , ((UpSize * (int32_t)(844)) / DownSize) - 190},
				{(UpSize * (int32_t)(-154)) / DownSize , (UpSize * (int32_t)(144)) / DownSize , ((UpSize * (int32_t)(844)) / DownSize) - 190},
				{(UpSize * (int32_t)(-316)) / DownSize , (UpSize * (int32_t)(-241)) / DownSize , ((UpSize * (int32_t)(1485)) / DownSize) - 190},
				{(UpSize * (int32_t)(-316)) / DownSize , (UpSize * (int32_t)(231)) / DownSize , ((UpSize * (int32_t)(1485)) / DownSize) - 190},
				{(UpSize * (int32_t)(-280)) / DownSize , (UpSize * (int32_t)(-251)) / DownSize , ((UpSize * (int32_t)(844)) / DownSize) - 190},
				{(UpSize * (int32_t)(-280)) / DownSize , (UpSize * (int32_t)(242)) / DownSize , ((UpSize * (int32_t)(844)) / DownSize) - 190},
			};

			constexpr auto VertexCount = sizeof(Vertices) / sizeof(vertex16_t);

			static constexpr triangle_face_t Triangles[] PROGMEM
			{
				{ 7, 8, 6 },
				{ 44, 49, 7 },
				{ 8, 7, 9 },
				{ 47, 42, 41 },
				{ 43, 40, 42 },
				{ 42, 7, 43 },
				{ 6, 43, 7 },
				{ 40, 41, 42 },
				{ 8, 48, 6 },
				{ 40, 46, 48 },
				{ 50, 46, 43 },
				{ 40, 48, 8 },
				{ 51, 42, 47 },
				{ 45, 6, 48 },
				{ 50, 43, 45 },
				{ 6, 45, 43 },
				{ 45, 44, 50 },
				{ 51, 50, 44 },
				{ 47, 41, 49 },
				{ 9, 49, 41 },
				{ 44, 45, 48 },
				{ 44, 48, 49 },
				{ 9, 7, 49 },
				{ 51, 44, 42 },
				{ 7, 42, 44 },
				{ 41, 40, 9 },
				{ 8, 9, 40 },
				{ 47, 49, 46 },
				{ 48, 46, 49 },
				{ 47, 46, 51 },
				{ 40, 43, 46 },
				{ 46, 50, 51 },
				{ 15, 16, 14 },
				{ 13, 14, 16 },
				{ 12, 13, 16 },
				{ 17, 24, 16 },
				{ 20, 21, 19 },
				{ 21, 22, 19 },
				{ 23, 33, 22 },
				{ 18, 19, 22 },
				{ 32, 22, 33 },
				{ 16, 19, 18 },
				{ 16, 18, 17 },
				{ 15, 20, 19 },
				{ 15, 19, 16 },
				{ 14, 21, 20 },
				{ 14, 20, 15 },
				{ 21, 14, 13 },
				{ 21, 13, 22 },
				{ 12, 23, 22 },
				{ 12, 22, 13 },
				{ 16, 24, 12 },
				{ 26, 27, 28 },
				{ 26, 28, 25 },
				{ 25, 12, 24 },
				{ 26, 25, 24 },
				{ 26, 24, 29 },
				{ 26, 29, 30 },
				{ 26, 30, 31 },
				{ 18, 22, 32 },
				{ 34, 35, 36 },
				{ 34, 36, 37 },
				{ 34, 37, 38 },
				{ 34, 38, 32 },
				{ 34, 32, 33 },
				{ 34, 33, 39 },
				{ 31, 36, 35 },
				{ 31, 35, 26 },
				{ 30, 37, 36 },
				{ 30, 36, 31 },
				{ 29, 38, 37 },
				{ 29, 37, 30 },
				{ 32, 24, 18 },
				{ 17, 18, 24 },
				{ 24, 32, 38 },
				{ 24, 38, 29 },
				{ 25, 33, 23 },
				{ 25, 23, 12 },
				{ 33, 28, 39 },
				{ 28, 33, 25 },
				{ 27, 34, 39 },
				{ 27, 39, 28 },
				{ 35, 34, 27 },
				{ 35, 27, 26 },
				{ 2, 3, 4 },
				{ 2, 4, 5 },
				{ 0, 1, 3 },
				{ 4, 10, 5 },
				{ 11, 10, 4 },
				{ 5, 10, 23 },
				{ 2, 5, 12 },
				{ 23, 12, 5 },
				{ 0, 12, 27 },
				{ 2, 12, 0 },
				{ 23, 10, 34 },
				{ 23, 27, 12 },
				{ 0, 34, 10 },
				{ 34, 27, 23 },
				{ 0, 27, 34 },
				{ 1, 11, 3 },
				{ 11, 4, 3 },
				{ 0, 3, 2 },
				{ 11, 1, 0 },
				{ 0, 10, 11 },
			};

			constexpr auto TriangleCount = sizeof(Triangles) / sizeof(triangle_face_t);

			static constexpr uint8_t Group[TriangleCount] PROGMEM
			{
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
				1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
				1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
				1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
				2, 2, 2, 2, 2, 2, 2, 2,
			};

			enum class FrustumVertex : uint8_t
			{
				NearBottomRight = 0,
				NearBottomLeft = 1,
				NearTopLeft = 2,
				NearTopRight = 3,
				FarBottomRight = 4,
				FarBottomLeft = 5,
				FarTopLeft = 6,
				FarTopRight = 7,
				Origin = 8,
				Count
			};

			enum class FrustumEdge : uint8_t
			{
				NearBottom = 0,
				NearLeft = 1,
				NearTop = 2,
				NearRight = 3,
				FarBottom = 4,
				FarLeft = 5,
				FarTop = 6,
				FarRight = 7,
				LeftBottom = 8,
				LeftTop = 9,
				RightTop = 10,
				RightBottom = 11,
				NearTopLeftOrigin = 12,
				NearTopRightOrigin = 13,
				NearBottomLeftOrigin = 14,
				NearBottomRightOrigin = 15,
				Count
			};
		}
	}

	namespace Objects
	{
		using namespace ::IntegerWorld::RenderObjects;
		class CameraMeshObject
			: public Mesh::SimpleStaticMeshTriangleObject<
			Shapes::Camera::VertexCount,
			Shapes::Camera::TriangleCount,
			FrustumCullingEnum::NoCulling>
		{
		private:
			using Base = Mesh::SimpleStaticMeshTriangleObject<
				Shapes::Camera::VertexCount,
				Shapes::Camera::TriangleCount,
				FrustumCullingEnum::NoCulling>;

		private:
			bool Trigger = false;

		public:
			CameraMeshObject()
				: Base(Shapes::Camera::Vertices, Shapes::Camera::Triangles)
			{
			}

			void TriggerLock()
			{
				Trigger = true;
			}

			virtual void ObjectShade(const frustum_t& frustum) final
			{
				if (Trigger)
				{
					Trigger = false;
					Rotation = frustum.rotation;
					Translation = frustum.origin;
				}

				Base::ObjectShade(frustum);
			}
		};


		class FrustumEdgeObject : public Edge::LineShadeObject<
			uint8_t(Shapes::Camera::FrustumVertex::Count),
			uint8_t(Shapes::Camera::FrustumEdge::Count),
			PrimitiveSources::Vertex::Dynamic::Source,
			PrimitiveSources::Edge::Dynamic::Source>
		{
		public:
			static constexpr uint8_t PrimitiveCount = 16;

		private:
			using Base = Edge::LineShadeObject<
				uint8_t(Shapes::Camera::FrustumVertex::Count),
				uint8_t(Shapes::Camera::FrustumEdge::Count),
				PrimitiveSources::Vertex::Dynamic::Source,
				PrimitiveSources::Edge::Dynamic::Source>;

			using FrustumVertex = Shapes::Camera::FrustumVertex;
			using FrustumEdge = Shapes::Camera::FrustumEdge;

		private:

		protected:
			PrimitiveSources::Vertex::Dynamic::Source VertexSource;
			PrimitiveSources::Edge::Dynamic::Source EdgeSource;

			vertex16_t VerticesSource[uint8_t(Shapes::Camera::FrustumVertex::Count)]{};
			edge_line_t EdgesSource[uint8_t(Shapes::Camera::FrustumEdge::Count)]{};

		private:
			uint8_t FarScale = 0;
			bool Trigger = false;

		public:
			FrustumEdgeObject()
				: VertexSource(VerticesSource)
				, EdgeSource(EdgesSource)
				, Base(VertexSource, EdgeSource)
			{
				EdgesSource[uint8_t(FrustumEdge::NearBottom)] = { uint8_t(FrustumVertex::NearBottomLeft), uint8_t(FrustumVertex::NearBottomRight) };
				EdgesSource[uint8_t(FrustumEdge::NearLeft)] = { uint8_t(FrustumVertex::NearTopLeft), uint8_t(FrustumVertex::NearBottomLeft) };
				EdgesSource[uint8_t(FrustumEdge::NearTop)] = { uint8_t(FrustumVertex::NearTopRight), uint8_t(FrustumVertex::NearTopLeft) };
				EdgesSource[uint8_t(FrustumEdge::NearRight)] = { uint8_t(FrustumVertex::NearBottomRight), uint8_t(FrustumVertex::NearTopRight) };
				EdgesSource[uint8_t(FrustumEdge::FarBottom)] = { uint8_t(FrustumVertex::FarBottomRight), uint8_t(FrustumVertex::FarBottomLeft) };
				EdgesSource[uint8_t(FrustumEdge::FarLeft)] = { uint8_t(FrustumVertex::FarTopLeft), uint8_t(FrustumVertex::FarBottomLeft) };
				EdgesSource[uint8_t(FrustumEdge::FarTop)] = { uint8_t(FrustumVertex::FarTopRight), uint8_t(FrustumVertex::FarTopLeft) };
				EdgesSource[uint8_t(FrustumEdge::FarRight)] = { uint8_t(FrustumVertex::FarBottomRight), uint8_t(FrustumVertex::FarTopRight) };

				EdgesSource[uint8_t(FrustumEdge::LeftBottom)] = { uint8_t(FrustumVertex::NearBottomLeft), uint8_t(FrustumVertex::FarBottomLeft) };
				EdgesSource[uint8_t(FrustumEdge::LeftTop)] = { uint8_t(FrustumVertex::NearTopLeft), uint8_t(FrustumVertex::FarTopLeft) };
				EdgesSource[uint8_t(FrustumEdge::RightTop)] = { uint8_t(FrustumVertex::NearTopRight), uint8_t(FrustumVertex::FarTopRight) };
				EdgesSource[uint8_t(FrustumEdge::RightBottom)] = { uint8_t(FrustumVertex::NearBottomRight), uint8_t(FrustumVertex::FarBottomRight) };

				EdgesSource[uint8_t(FrustumEdge::NearTopLeftOrigin)] = { uint8_t(FrustumVertex::NearTopLeft), uint8_t(FrustumVertex::Origin) };
				EdgesSource[uint8_t(FrustumEdge::NearTopRightOrigin)] = { uint8_t(FrustumVertex::NearTopRight), uint8_t(FrustumVertex::Origin) };
				EdgesSource[uint8_t(FrustumEdge::NearBottomLeftOrigin)] = { uint8_t(FrustumVertex::NearBottomLeft), uint8_t(FrustumVertex::Origin) };
				EdgesSource[uint8_t(FrustumEdge::NearBottomRightOrigin)] = { uint8_t(FrustumVertex::NearBottomRight), uint8_t(FrustumVertex::Origin) };
			}

			void TriggerLock(const uint8_t farScale = 8)
			{
				Trigger = true;
				FarScale = farScale;
			}

			virtual void ObjectShade(const frustum_t& frustum)
			{
				Base::ObjectShade(frustum);

				if (Trigger)
				{
					Trigger = false;

					VerticesSource[uint8_t(FrustumVertex::Origin)] = frustum.origin;

					// Near corners: compute intersection of the frustum planes.
					PlaneIntersection(frustum.cullingNearPlane, frustum.cullingLeftPlane, frustum.cullingBottomPlane,
						VerticesSource[uint8_t(FrustumVertex::NearBottomLeft)]);
					PlaneIntersection(frustum.cullingNearPlane, frustum.cullingRightPlane, frustum.cullingBottomPlane,
						VerticesSource[uint8_t(FrustumVertex::NearBottomRight)]);
					PlaneIntersection(frustum.cullingNearPlane, frustum.cullingLeftPlane, frustum.cullingTopPlane,
						VerticesSource[uint8_t(FrustumVertex::NearTopLeft)]);
					PlaneIntersection(frustum.cullingNearPlane, frustum.cullingRightPlane, frustum.cullingTopPlane,
						VerticesSource[uint8_t(FrustumVertex::NearTopRight)]);

					// Far corners: extrude each near corner away from the origin.
					for (int i = 0; i < 4; ++i)
					{
						const uint8_t nearIdx = uint8_t(FrustumVertex::NearBottomRight) + i;
						const uint8_t farIdx = uint8_t(FrustumVertex::FarBottomRight) + i;

						const vertex16_t& nc = VerticesSource[nearIdx];
						vertex16_t& fc = VerticesSource[farIdx];

						const int32_t dx = int32_t(nc.x) - frustum.origin.x;
						const int32_t dy = int32_t(nc.y) - frustum.origin.y;
						const int32_t dz = int32_t(nc.z) - frustum.origin.z;

						fc.x = static_cast<int16_t>(frustum.origin.x + (dx * FarScale));
						fc.y = static_cast<int16_t>(frustum.origin.y + (dy * FarScale));
						fc.z = static_cast<int16_t>(frustum.origin.z + (dz * FarScale));
					}
				}
			}

		private:
			// Correct intersection of three planes:
			// plane: n.x * X + n.y * Y + n.z * Z + d = 0, normals are normalized to VERTEX16_UNIT scale.
			static void PlaneIntersection(const plane16_t& p1, const plane16_t& p2, const plane16_t& p3, vertex16_t& result)
			{
				// To keep all intermediate results in 32-bit signed range, we need to
				// downscale the cross products if any component exceeds this limit.
				static constexpr int32_t limit = INT32_MAX / (3 * INT16_MAX); // ~21845

				// Extract components.
				const int16_t a1 = p1.x, b1 = p1.y, c1 = p1.z, d1 = p1.distance;
				const int16_t a2 = p2.x, b2 = p2.y, c2 = p2.z, d2 = p2.distance;
				const int16_t a3 = p3.x, b3 = p3.y, c3 = p3.z, d3 = p3.distance;

				// Cross products (int32).
				int32_t cx23x = (int32_t(b2) * c3) - (int32_t(c2) * b3);
				int32_t cx23y = (int32_t(c2) * a3) - (int32_t(a2) * c3);
				int32_t cx23z = (int32_t(a2) * b3) - (int32_t(b2) * a3);

				int32_t cx31x = (int32_t(b3) * c1) - (int32_t(c3) * b1);
				int32_t cx31y = (int32_t(c3) * a1) - (int32_t(a3) * c1);
				int32_t cx31z = (int32_t(a3) * b1) - (int32_t(b3) * a1);

				int32_t cx12x = (int32_t(b1) * c2) - (int32_t(c1) * b2);
				int32_t cx12y = (int32_t(c1) * a2) - (int32_t(a1) * c2);
				int32_t cx12z = (int32_t(a1) * b2) - (int32_t(b1) * a2);

				// Find maximum absolute component.
				uint32_t maxAbs = MaxValue(MaxValue(MaxValue(AbsValue(cx23x), AbsValue(cx23y)),
					AbsValue(cx23z)), MaxValue(MaxValue(MaxValue(AbsValue(cx31x), AbsValue(cx31y)),
						AbsValue(cx31z)), MaxValue(MaxValue(AbsValue(cx12x), AbsValue(cx12y)), AbsValue(cx12z))));

				// Power-of-two downscale for cross products (keep sums safe in 32-bit).
				while (maxAbs > limit)
				{
					cx23x = SignedRightShift(cx23x, 1);
					cx23y = SignedRightShift(cx23y, 1);
					cx23z = SignedRightShift(cx23z, 1);

					cx31x = SignedRightShift(cx31x, 1);
					cx31y = SignedRightShift(cx31y, 1);
					cx31z = SignedRightShift(cx31z, 1);

					cx12x = SignedRightShift(cx12x, 1);
					cx12y = SignedRightShift(cx12y, 1);
					cx12z = SignedRightShift(cx12z, 1);

					maxAbs >>= 1;
				}

				// Denominator, in 32-bit divided by VERTEX16_UNIT, to match the scaled cross products.
				const int32_t den = SignedRightShift((int32_t(a1) * cx23x) + (int32_t(b1) * cx23y) + (int32_t(c1) * cx23z), GetBitShifts(VERTEX16_UNIT));
				if (den == 0)
				{
					result.x = 0;
					result.y = 0;
					result.z = 0;
					return;
				}

				// Numerators with distances negated, in 64-bit.
				const int32_t nd1 = -int32_t(d1);
				const int32_t nd2 = -int32_t(d2);
				const int32_t nd3 = -int32_t(d3);

				const int32_t numx = (nd1 * cx23x) + (nd2 * cx31x) + (nd3 * cx12x);
				const int32_t numy = (nd1 * cx23y) + (nd2 * cx31y) + (nd3 * cx12y);
				const int32_t numz = (nd1 * cx23z) + (nd2 * cx31z) + (nd3 * cx12z);

				// Divide with clamping to int16_t range.
				result.x = LimitValue<int32_t, INT16_MIN, INT16_MAX>(numx / den);
				result.y = LimitValue<int32_t, INT16_MIN, INT16_MAX>(numy / den);
				result.z = LimitValue<int32_t, INT16_MIN, INT16_MAX>(numz / den);
			}
		};
	}

	namespace Debug
	{
		class FrustumDebugger
		{
		public:
			static constexpr uint8_t ObjectCount = 2;
			static constexpr uint16_t PrimitiveCount =
				Objects::FrustumEdgeObject::PrimitiveCount +
				Shapes::Camera::TriangleCount;

		private:
			Objects::FrustumEdgeObject ObjectFrustumEdge{};
			RenderObjects::Edge::FragmentShaders::LineShade::FillShader EdgeShader{};


			Objects::CameraMeshObject ObjectCamera{};
			RenderObjects::Mesh::FragmentShaders::TriangleShade::FillShader MeshShader{};

		public:
			bool Start(IEngineRenderer& engineRenderer)
			{
				// Camera test objects.
				if (!engineRenderer.AddObject(&ObjectCamera)
					|| !engineRenderer.AddObject(&ObjectFrustumEdge))
				{
					return false;
				}

				// Setup shaders.
				ObjectCamera.FragmentShader = &MeshShader;
				ObjectFrustumEdge.FragmentShader = &EdgeShader;

				CaptureViewFrustum();

				return true;
			}

			// Call to capture the current view frustum from the camera.
			void CaptureViewFrustum()
			{
				ObjectCamera.TriggerLock();
				ObjectFrustumEdge.TriggerLock();
			}
		};
	}
}
#endif
#endif