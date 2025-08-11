#ifndef _INTEGER_WORLD_VERTEX_h
#define _INTEGER_WORLD_VERTEX_h

#include <IntegerSignal.h>

namespace IntegerWorld
{
	using namespace IntegerSignal;

	/// <summary>
	/// vertex16_t axis uses int16_t.
	/// </summary>
	static constexpr int16_t VERTEX16_RANGE = INT16_MAX;

	/// <summary>
	/// Unit scale for 16-bit vertices.
	/// </summary>
	static constexpr int16_t VERTEX16_UNIT = (UINT16_MAX / 16) + 1;

	/// <summary>
	/// vertex32_t axis uses int32_t.
	/// </summary>
	static constexpr int32_t VERTEX32_RANGE = INT32_MAX;

	/// <summary>
	/// Unit scale for 32-bit vertices.
	/// </summary>
	static constexpr int32_t VERTEX32_UNIT = (UINT32_MAX / (16 * 16)) + 1;

	struct vertex16_t
	{
		int16_t x;
		int16_t y;
		int16_t z;
	};

	struct vertex32_t
	{
		int32_t x;
		int32_t y;
		int32_t z;
	};

	static void GetNormal16(const vertex16_t& a, const vertex16_t& b, const vertex16_t& c, vertex32_t& normal)
	{
		// Compute the vectors for two edges of the triangle.
		const int16_t ax = b.x - a.x;
		const int16_t ay = b.y - a.y;
		const int16_t az = b.z - a.z;
		const int16_t bx = c.x - a.x;
		const int16_t by = c.y - a.y;
		const int16_t bz = c.z - a.z;

		// Compute the cross product of the two edge vectors.
		normal.x = (int32_t(ay) * bz) - (int32_t(az) * by);
		normal.y = (int32_t(az) * bx) - (int32_t(ax) * bz);
		normal.z = (int32_t(ax) * by) - (int32_t(ay) * bx);
	}

	static void GetNormal32(const vertex32_t& a, const vertex32_t& b, const vertex32_t& c, vertex32_t& normal)
	{
		// Compute the vectors for two edges of the triangle.
		const int32_t ax = b.x - a.x;
		const int32_t ay = b.y - a.y;
		const int32_t az = b.z - a.z;
		const int32_t bx = c.x - a.x;
		const int32_t by = c.y - a.y;
		const int32_t bz = c.z - a.z;

		// Compute the cross product of the two edge vectors and scale back to int32 range.
		normal.x = int32_t(SignedRightShift(((int64_t)ay * bz) - ((int64_t)az * by), sizeof(int32_t)));
		normal.y = int32_t(SignedRightShift(((int64_t)az * bx) - ((int64_t)ax * bz), sizeof(int32_t)));
		normal.z = int32_t(SignedRightShift(((int64_t)ax * by) - ((int64_t)ay * bx), sizeof(int32_t)));
	}

	constexpr int32_t DotProduct16(const vertex16_t& vertex1, const vertex16_t& vertex2)
	{
		return ((int32_t)vertex1.x * vertex2.x) + ((int32_t)vertex1.y * vertex2.y) + ((int32_t)vertex1.z * vertex2.z);
	}

	constexpr int32_t DotProduct16(const vertex16_t& vertex1, const vertex32_t& vertex2)
	{
		return ((int32_t)vertex1.x * vertex2.x) + ((int32_t)vertex1.y * vertex2.y) + ((int32_t)vertex1.z * vertex2.z);
	}

	constexpr int32_t DotProduct16(const vertex32_t& vertex1, const vertex16_t& vertex2)
	{
		return ((int32_t)vertex1.x * vertex2.x) + ((int32_t)vertex1.y * vertex2.y) + ((int32_t)vertex1.z * vertex2.z);
	}

	constexpr int32_t DotProduct16(const vertex32_t& vertex1, const vertex32_t& vertex2)
	{
		return ((int32_t)vertex1.x * vertex2.x) + ((int32_t)vertex1.y * vertex2.y) + ((int32_t)vertex1.z * vertex2.z);
	}

	constexpr int64_t DotProduct32(const vertex32_t& vertex1, const vertex32_t& vertex2)
	{
		return ((int64_t)vertex1.x * vertex2.x) + ((int64_t)vertex1.y * vertex2.y) + ((int64_t)vertex1.z * vertex2.z);
	}

	static void NormalizeVertex16(vertex16_t& vertex)
	{
		// Calculate the squared length of the vector.
		const uint32_t lengthSquared = (uint32_t)((int32_t)vertex.x * vertex.x) +
			((int32_t)vertex.y * vertex.y) +
			((int32_t)vertex.z * vertex.z);

		if (lengthSquared != 0)
		{
			// Calculate the length using the integer square root method.
			const uint16_t magnitude = IntegerSignal::SquareRoot32(lengthSquared);

			// Normalize to VERTEX16_UNIT.
			if (magnitude != 0 && magnitude != VERTEX16_UNIT)
			{
				vertex.x = ((int32_t)vertex.x << GetBitShifts(VERTEX16_UNIT)) / magnitude;
				vertex.y = ((int32_t)vertex.y << GetBitShifts(VERTEX16_UNIT)) / magnitude;
				vertex.z = ((int32_t)vertex.z << GetBitShifts(VERTEX16_UNIT)) / magnitude;
			}
		}
	}

	static void NormalizeVertex16(vertex32_t& vertex)
	{
		// Calculate the squared length of the vector.
		const uint32_t lengthSquared = (uint32_t)((int32_t)vertex.x * vertex.x) +
			((int32_t)vertex.y * vertex.y) +
			((int32_t)vertex.z * vertex.z);

		if (lengthSquared != 0)
		{
			// Calculate the length using the integer square root method.
			const uint16_t magnitude = IntegerSignal::SquareRoot32(lengthSquared);

			// Normalize to VERTEX16_UNIT.
			if (magnitude != 0 && magnitude != VERTEX16_UNIT)
			{
				vertex.x = ((int32_t)vertex.x << GetBitShifts(VERTEX16_UNIT)) / magnitude;
				vertex.y = ((int32_t)vertex.y << GetBitShifts(VERTEX16_UNIT)) / magnitude;
				vertex.z = ((int32_t)vertex.z << GetBitShifts(VERTEX16_UNIT)) / magnitude;
			}
		}
	}

	static void NormalizeVertex32(vertex32_t& vertex)
	{
		// Calculate the squared length of the vector.
		const uint64_t lengthSquared = (uint64_t)((int64_t)vertex.x * vertex.x) +
			((int64_t)vertex.y * vertex.y) +
			((int64_t)vertex.z * vertex.z);

		if (lengthSquared != 0)
		{
			// Calculate the length using the integer square root method.
			const uint32_t magnitude = IntegerSignal::SquareRoot64(lengthSquared);

			// Normalize to VERTEX16_UNIT.
			if (magnitude != 0 && magnitude != VERTEX16_UNIT)
			{
				vertex.x = ((int64_t)vertex.x << GetBitShifts(VERTEX16_UNIT)) / magnitude;
				vertex.y = ((int64_t)vertex.y << GetBitShifts(VERTEX16_UNIT)) / magnitude;
				vertex.z = ((int64_t)vertex.z << GetBitShifts(VERTEX16_UNIT)) / magnitude;
			}
		}
	}

	static void NormalizeVertex32Fast(vertex32_t& vertex)
	{
		// Reduce to 16 bit vertex by dividing by 2 until all values are in range.
		while (vertex.x < INT16_MIN
			|| vertex.x > INT16_MAX
			|| vertex.y < INT16_MIN
			|| vertex.y > INT16_MAX
			|| vertex.z < INT16_MIN
			|| vertex.z > INT16_MAX)
		{
			vertex.x = SignedRightShift(vertex.x, 1);
			vertex.y = SignedRightShift(vertex.y, 1);
			vertex.z = SignedRightShift(vertex.z, 1);
		}

		NormalizeVertex16(vertex);
	}

	static constexpr int16_t AverageApproximate(const int16_t a, const int16_t b, const int16_t c)
	{
		// Sum as int32_t to prevent overflow, then divide by 3 using multiply and shift
		// 0x5556 is the fixed-point reciprocal of 3 for 16-bit (rounded)
		// (a + b + c) * 0x5556 >> 16 is equivalent to (a + b + c) / 3
		return int16_t(((int32_t(a) + b + c) * 0x5556) >> 16);
	}

	static uint16_t Distance16(const int16_t aX, const int16_t aY, const int16_t bX, const int16_t bY)
	{
		// Calculate squared differences for each axis
		const int32_t dx = int32_t(bX) - aX;
		const int32_t dy = int32_t(bY) - aY;
		// Compute squared distance
		const uint32_t distSquared = uint32_t(dx * dx) + uint32_t(dy * dy);
		// Use IntegerSignal's integer square root
		return IntegerSignal::SquareRoot32(distSquared);
	}

	static uint16_t Distance16(const vertex16_t& a, const vertex16_t& b)
	{
		// Calculate squared differences for each axis
		const int32_t dx = int32_t(b.x) - a.x;
		const int32_t dy = int32_t(b.y) - a.y;
		const int32_t dz = int32_t(b.z) - a.z;

		// Compute squared distance
		const uint32_t distSquared = uint32_t(dx * dx) + uint32_t(dy * dy) + uint32_t(dz * dz);

		// Use IntegerSignal's integer square root
		return IntegerSignal::SquareRoot32(distSquared);
	}
}

#endif