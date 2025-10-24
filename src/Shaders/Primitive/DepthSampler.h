#ifndef _INTEGER_WORLD_SHADERS_PRIMITIVE_DEPTH_SAMPLER_h
#define _INTEGER_WORLD_SHADERS_PRIMITIVE_DEPTH_SAMPLER_h

#include "../../Framework/Model.h"

namespace IntegerWorld
{
	namespace PrimitiveShaders
	{
		namespace DepthSampler
		{
			static constexpr int16_t RangeMin = 1024;
			static constexpr int16_t RangeMax = DRAW_DISTANCE_MAX;

			static uint8_t ZDepth8(const int16_t z)
			{
				if (z >= RangeMax)
				{
					return 0;
				}
				else if (z <= RangeMin)
				{
					return UINT8_MAX;
				}
				else
				{
					return Curves::Power2U8<>::Get(UINT8_MAX - ((uint32_t(z - RangeMin) * UINT8_MAX) / uint32_t(RangeMax - RangeMin)));
				}
			}

			static uint16_t ZDepth16(const int16_t z)
			{
				if (z >= RangeMax)
				{
					return 0;
				}
				else if (z <= RangeMin)
				{
					return UINT16_MAX;
				}
				else
				{
					return Curves::Power2U16<>::Get(UINT16_MAX - ((uint32_t(z - RangeMin) * UINT16_MAX) / uint32_t(RangeMax - RangeMin)));
				}
			}
		}
	}
}
#endif