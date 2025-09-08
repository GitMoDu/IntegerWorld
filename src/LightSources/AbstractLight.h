#ifndef _INTEGER_WORLD_ABSTRACT_LIGHT_h
#define _INTEGER_WORLD_ABSTRACT_LIGHT_h

#include "../Object/PointObject.h"

namespace IntegerWorld
{
	class AbstractProximityLight : public ILightSource
	{
	private:
		uint32_t RangeMin = (uint32_t)VERTEX16_UNIT * VERTEX16_UNIT;
		uint32_t RangeMax = (uint32_t)VERTEX16_RANGE * VERTEX16_RANGE;

	public:
		vertex16_t WorldPosition{};

	protected:
		const vertex16_t* CameraPosition;

	public:
		AbstractProximityLight(const vertex16_t* cameraPosition)
			: ILightSource()
			, CameraPosition(cameraPosition)
		{
		}

		void SetLightRange(const uint16_t rangeMin, const uint16_t rangeMax)
		{
			RangeMin = (uint32_t)rangeMin * rangeMin;
			RangeMax = MaxValue((uint32_t)rangeMax * rangeMax, RangeMin);
		}

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
		/// <summary>
		/// Number of bit shifts for VERTEX16_DOT.
		/// </summary>
		static constexpr uint8_t DOT_SHIFTS = GetBitShifts(VERTEX16_DOT);

		/// <summary>
		/// Bit shifts required for UFRACTION16_1X.
		/// </summary>
		static constexpr uint8_t FRAC_SHIFTS = GetBitShifts(UFRACTION16_1X);

		/// <summary>
		/// Shifts required to convert from a (positive) 32 bit dot product to 16 bit ufraction.
		/// </summary>
		static constexpr uint8_t DOT_CONVERT_SHIFTS = DOT_SHIFTS - FRAC_SHIFTS;

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