#ifndef _INTEGER_WORLD_SHADERS_PRIMITIVE_LINE_SAMPLER_h
#define _INTEGER_WORLD_SHADERS_PRIMITIVE_LINE_SAMPLER_h

#include "../../Framework/Model.h"

namespace IntegerWorld
{
	namespace PrimitiveShaders
	{
		class LineSampler
		{
		private:
			int16_t Ax = 0;
			int16_t Ay = 0;
			int16_t Dx = 0;
			int16_t Dy = 0;
			int32_t LengthSquared = 0;

		public:
			bool SetLine(const vertex16_t& a, const vertex16_t& b)
			{
				Ax = a.x;
				Ay = a.y;

				Dx = static_cast<int16_t>(b.x - a.x);
				Dy = static_cast<int16_t>(b.y - a.y);

				LengthSquared = (static_cast<int32_t>(Dx) * Dx) + (static_cast<int32_t>(Dy) * Dy);

				return LengthSquared != 0;
			}

			ufraction8_t U8Fraction(const int16_t x, const int16_t y) const
			{
				const int32_t rx = static_cast<int32_t>(x) - static_cast<int32_t>(Ax);
				const int32_t ry = static_cast<int32_t>(y) - static_cast<int32_t>(Ay);
				const int32_t px = (rx * Dx) + (ry * Dy);
				if (px <= 0)
				{
					return 0;
				}
				else if (px >= LengthSquared)
				{
					return UFraction8::FRACTION_1X;
				}
				else
				{
					return UFraction8::GetScalar<uint32_t>(static_cast<uint32_t>(px), static_cast<uint32_t>(LengthSquared));
				}
			}

			ufraction16_t U16Fraction(const int16_t x, const int16_t y) const
			{
				const int32_t rx = static_cast<int32_t>(x) - static_cast<int32_t>(Ax);
				const int32_t ry = static_cast<int32_t>(y) - static_cast<int32_t>(Ay);
				const int32_t px = (rx * Dx) + (ry * Dy);

				if (px <= 0)
				{
					return 0;
				}
				else if (px >= LengthSquared)
				{
					return UFRACTION16_1X;
				}
				else
				{
					return UFraction16::GetScalar<uint32_t>(static_cast<uint32_t>(px), static_cast<uint32_t>(LengthSquared));
				}
			}
		};
	}
}
#endif