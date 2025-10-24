#ifndef _INTEGER_WORLD_VERTEX_h
#define _INTEGER_WORLD_VERTEX_h

#include <IntegerSignal.h>

namespace IntegerWorld
{
	using namespace IntegerSignal;

	/// <summary>
	/// Represents a 3D vertex with signed 16-bit integer coordinates.
	/// </summary>
	struct vertex16_t
	{
		int16_t x;
		int16_t y;
		int16_t z;
	};

	/// <summary>
	/// vertex16_t axis uses int16_t.
	/// </summary>
	static constexpr int16_t VERTEX16_RANGE = INT16_MAX;

	/// <summary>
	/// Unit scale for 16-bit vertices.
	/// </summary>
	static constexpr int16_t VERTEX16_UNIT = (UINT16_MAX / 8) + 1;

	/// <summary>
	/// Dot product range for 16-bit vertex units.
	/// </summary>
	static constexpr int32_t VERTEX16_DOT = int32_t(VERTEX16_UNIT) * VERTEX16_UNIT;

	/// <summary>
	/// Maximum draw distance for 16-bit vertices.
	/// </summary>
	static constexpr uint16_t DRAW_DISTANCE_MAX = ((uint32_t)VERTEX16_UNIT) * (VERTEX16_RANGE / VERTEX16_UNIT);

	/// <summary>
	/// Reciprocal approximation of 1/3 in fixed-point (int16_t).
	/// </summary>
	static constexpr int16_t ThreePointReciprocal = 0x5556;

	/// <summary>
	/// Largest sum that keeps (sum * ThreePointReciprocal) within int32_t.
	/// </summary>
	static constexpr int32_t ThreePointSumMax = INT32_MAX / ThreePointReciprocal;

	/// <summary>
	/// Calculates the dot product of two vertex16_t vectors.
	/// </summary>
	/// <param name="vertexA">The first vertex.</param>
	/// <param name="vertexB">The second vertex.</param>
	/// <returns>The dot product of the two vertices.</returns>
	static constexpr int32_t DotProduct16(const vertex16_t& vertexA, const vertex16_t& vertexB)
	{
		return (static_cast<int32_t>(vertexA.x) * vertexB.x) +
			(static_cast<int32_t>(vertexA.y) * vertexB.y) +
			(static_cast<int32_t>(vertexA.z) * vertexB.z);
	}

	/// <summary>
	/// Normalizes vertex in place to have length VERTEX16_UNIT.
	/// If the input length is zero or already equal to VERTEX16_UNIT, the vertex is left unchanged.
	/// </summary>
	/// <param name="vertex">Reference to  vertex16_t to normalize.</param>
	static void NormalizeVertex16(vertex16_t& vertex)
	{
		// Calculate the squared length of the vector.
		const uint32_t lengthSquared = static_cast<uint32_t>(static_cast<int32_t>(vertex.x) * vertex.x) +
			static_cast<uint32_t>(static_cast<int32_t>(vertex.y) * vertex.y) +
			static_cast<uint32_t>(static_cast<int32_t>(vertex.z) * vertex.z);

		if (lengthSquared != 0)
		{
			// Calculate the length using the integer square root method.
			const uint16_t magnitude = IntegerSignal::SquareRoot32(lengthSquared);

			// Normalize to VERTEX16_UNIT.
			if (magnitude != 0 && magnitude != VERTEX16_UNIT)
			{
				vertex.x = SignedLeftShift(static_cast<int32_t>(vertex.x), GetBitShifts(VERTEX16_UNIT)) / magnitude;
				vertex.y = SignedLeftShift(static_cast<int32_t>(vertex.y), GetBitShifts(VERTEX16_UNIT)) / magnitude;
				vertex.z = SignedLeftShift(static_cast<int32_t>(vertex.z), GetBitShifts(VERTEX16_UNIT)) / magnitude;
			}
		}
	}

	static vertex16_t GetNormal16(const vertex16_t& vectorA, const vertex16_t& vectorB)
	{
		int32_t normalX = (static_cast<int32_t>(vectorA.y) * vectorB.z)
			- (static_cast<int32_t>(vectorA.z) * vectorB.y);
		int32_t normalY = (static_cast<int32_t>(vectorA.z) * vectorB.x)
			- (static_cast<int32_t>(vectorA.x) * vectorB.z);
		int32_t normalZ = (static_cast<int32_t>(vectorA.x) * vectorB.y)
			- (static_cast<int32_t>(vectorA.y) * vectorB.x);

		// Reduce to 16 bit vertex by dividing by 2 until all values are in range.
		while (normalX < INT16_MIN
			|| normalX > INT16_MAX
			|| normalY < INT16_MIN
			|| normalY > INT16_MAX
			|| normalZ < INT16_MIN
			|| normalZ > INT16_MAX)
		{
			normalX = SignedRightShift(normalX, 1);
			normalY = SignedRightShift(normalY, 1);
			normalZ = SignedRightShift(normalZ, 1);
		}

		return vertex16_t{
			static_cast<int16_t>(normalX),
			static_cast<int16_t>(normalY),
			static_cast<int16_t>(normalZ)
		};
	}

	static vertex16_t GetNormal16(const vertex16_t& vertexA, const vertex16_t& vertexB, const vertex16_t& vertexC)
	{
		return GetNormal16(
			{ static_cast<int16_t>(vertexB.x - vertexA.x),
			  static_cast<int16_t>(vertexB.y - vertexA.y),
			  static_cast<int16_t>(vertexB.z - vertexA.z) },
			{ static_cast<int16_t>(vertexC.x - vertexA.x),
			  static_cast<int16_t>(vertexC.y - vertexA.y),
			  static_cast<int16_t>(vertexC.z - vertexA.z) });
	}

	static uint16_t Distance16(const vertex16_t& a, const vertex16_t& b)
	{
		// Calculate squared differences for each axis.
		const int16_t dx = b.x - a.x;
		const int16_t dy = b.y - a.y;
		const int16_t dz = b.z - a.z;

		// Compute squared distance.
		const uint32_t distSquared = static_cast<uint32_t>(static_cast<int32_t>(dx) * dx)
			+ static_cast<uint32_t>(static_cast<int32_t>(dy) * dy)
			+ static_cast<uint32_t>(static_cast<int32_t>(dz) * dz);

		// Use IntegerSignal's integer square root.
		return IntegerSignal::SquareRoot32(distSquared);
	}

	/// <summary>
	/// Computes an approximate average of three signed 16-bit integers using scaled fixed-point arithmetic to avoid overflow.
	/// </summary>
	/// <param name="a">The first signed 16-bit input value.</param>
	/// <param name="b">The second signed 16-bit input value.</param>
	/// <param name="c">The third signed 16-bit input value.</param>
	/// <returns>A signed 16-bit value containing the truncated, approximate average of the three inputs.
	/// The result is computed using scaling and a fixed-point divide-by-3 approximation, so it may differ slightly from the exact arithmetic mean.</returns>
	static int16_t AverageApproximate(const int16_t a, const int16_t b, const int16_t c)
	{
		// 3 value sum.
		int32_t sum = static_cast<int32_t>(a) + b + c;
		const uint8_t shifts = (AbsValue(sum) > ThreePointSumMax) ? 1 : 0;

		// Scale down sum if it exceeds safe maximum.
		sum = SignedRightShift(sum, shifts);

		// Truncating fixed-point divide-by-3: (sum * 0x5556) >> (16 - shifts).
		return static_cast<int16_t>(SignedRightShift(sum * ThreePointReciprocal, 16 - shifts));
	}
}

#endif