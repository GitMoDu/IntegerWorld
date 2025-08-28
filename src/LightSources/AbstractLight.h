#ifndef _INTEGER_WORLD_ABSTRACT_LIGHT_h
#define _INTEGER_WORLD_ABSTRACT_LIGHT_h

#include "../Object/PointObject.h"

namespace IntegerWorld
{
	class AbstractProximityLight : public ILightSource, public TranslationObject
	{
	private:
		uint32_t RangeMin = (uint32_t)VERTEX16_UNIT * VERTEX16_UNIT;
		uint32_t RangeMax = (uint32_t)VERTEX16_RANGE * VERTEX16_RANGE;

	public:
		AbstractProximityLight() : ILightSource(), TranslationObject()
		{
		}

		void SetLightRange(const uint16_t rangeMin, const uint16_t rangeMax)
		{
			RangeMin = (uint32_t)rangeMin * rangeMin;
			RangeMax = MaxValue((uint32_t)rangeMax * rangeMax, RangeMin);
		}

	public:
		bool PrimitiveWorldShade(const uint16_t index) { return true; }
		bool PrimitiveScreenShade(const uint16_t index, const uint16_t boundsWidth, const uint16_t boundsHeight) { return true; }
		void FragmentCollect(FragmentCollector& fragmentCollector) {}
		void FragmentShade(WindowRasterizer& rasterizer, const uint16_t index) {}


	protected:
		ufraction16_t GetProximityFraction(const vertex32_t& vector) const
		{
			const uint32_t distance = MinValue(RangeMax, MaxValue(RangeMin,
				(uint32_t)((int32_t(vector.x) * vector.x) + (int32_t(vector.y) * vector.y) + (int32_t(vector.z) * vector.z))));

			if (distance >= RangeMax)
			{
				return 0;
			}
			else if (distance < RangeMin)
			{
				return UFRACTION16_1X;
			}
			else
			{
				return UFRACTION16_1X - Fraction::GetUFraction16(uint32_t(distance - RangeMin), uint32_t((RangeMax - RangeMin)));
			}
		}
	};

	struct AbstractLightSource
	{
		static constexpr uint8_t DOT_SHIFTS = GetBitShifts(VERTEX16_DOT);
		static constexpr uint8_t FRAC_SHIFTS = GetBitShifts(UFRACTION16_1X);
		static constexpr uint8_t DOT_CONVERT_SHIFTS = DOT_SHIFTS - FRAC_SHIFTS;

		static constexpr ufraction16_t GetDotProductFraction(const int32_t dotProduct)
		{
			return dotProduct <= 0 ? 0 : ufraction16_t(uint32_t(dotProduct) >> DOT_CONVERT_SHIFTS);
		}

		static ufraction16_t GetSpecularFraction(const vertex32_t& illuminationVector, const vertex16_t& viewVector, const vertex16_t& normal)
		{
			// Blinn–Phong: Compute the half–vector as the sum of the illumination vector and the view vector.
			vertex32_t halfVector{ SignedRightShift((int32_t)illuminationVector.x + viewVector.x, 1),
									SignedRightShift((int32_t)illuminationVector.y + viewVector.y, 1),
									SignedRightShift((int32_t)illuminationVector.z + viewVector.z, 1) };

			NormalizeVertex32Fast(halfVector);

			// Compute the dot product between the surface normal and the half–vector.
			// In the Blinn–Phong model, a higher dot product corresponds to a stronger specular highlight.
			const int32_t dotProduct = DotProduct16(normal, halfVector);

			if (dotProduct > 0)
			{
				return GetDotProductFraction(dotProduct);
			}
			else
			{
				return 0;
			}
		}

		static ufraction16_t GetAbsoluteProximity(const vertex16_t& vector, const uint8_t falloffWeight)
		{
			// Get the distance approximate.
			const uint16_t distance =
				MinValue<uint32_t>(VERTEX16_RANGE,
					(uint32_t(((int32_t)vector.x * vector.x)
						+ ((int32_t)vector.y * vector.y)
						+ ((int32_t)vector.z * vector.z))
						* falloffWeight) >> (GetBitShifts(VERTEX16_RANGE) + 1));

			return Fraction::GetUFraction16((uint16_t)VERTEX16_UNIT, (uint16_t)MaxValue(distance, (uint16_t)VERTEX16_UNIT));
		}
	};
}
#endif