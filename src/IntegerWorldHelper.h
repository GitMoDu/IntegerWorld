#ifndef _INTEGER_WORLD_TASKS_INCLUDE_h
#define _INTEGER_WORLD_TASKS_INCLUDE_h

#include "IntegerWorld.h"

namespace IntegerWorld
{
	static void GenerateNormalMap(const vertex16_t* verticesSource, const triangle_face_t* trianglesSource, const uint16_t triangleCount)
	{
		Serial.println(F("static constexpr vertex16_t Normals[TriangleCount] PROGMEM"));
		Serial.println('{');
		for (uint32_t i = 0; i < triangleCount; i++)
		{
			const triangle_face_t& triangle = trianglesSource[i];

			vertex32_t normal{};
			GetNormal16(verticesSource[triangle.v1], verticesSource[triangle.v2], verticesSource[triangle.v3], normal);
			NormalizeVertex32(normal);

			Serial.print('{');
			Serial.print(normal.x);
			Serial.print(',');
			Serial.print(normal.y);
			Serial.print(',');
			Serial.print(normal.z);
			Serial.print('}');
			if (i < triangleCount - 1)
			{
				Serial.println(',');
			}
			else
			{
				Serial.println();
			}
		}
		Serial.println(F("};"));
	}
}

#endif