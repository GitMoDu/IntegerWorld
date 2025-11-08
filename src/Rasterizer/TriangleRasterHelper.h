#ifndef _INTEGER_WORLD_TRIANGLE_RASTER_HELPER_h
#define _INTEGER_WORLD_TRIANGLE_RASTER_HELPER_h

#include "Abstract2dDrawer.h"
#include <stdint.h>

namespace IntegerWorld
{
	namespace TriangleRasterHelper
	{
		struct point2d_t
		{
			int16_t x;
			int16_t y;
		};

		enum class ClipEdgeEnum : uint8_t
		{
			Left,
			Right,
			Top,
			Bottom
		};

		/// <summary>
		/// Determines if a point (x, y) lies inside the triangle defined by (x1, y1), (x2, y2), and (x3, y3).
		/// </summary>
		/// <returns>True if the point (x, y) is inside the triangle (edges inclusive); otherwise, false.</returns>
		static bool PointInTriangle(const int16_t x, const  int16_t y,
			const int16_t x1, const  int16_t y1,
			const int16_t x2, const  int16_t y2,
			const int16_t x3, const  int16_t y3)
		{
			const int32_t d1 = int32_t(x - x2) * (y1 - y2) - int32_t(x1 - x2) * (y - y2);
			const int32_t d2 = int32_t(x - x3) * (y2 - y3) - int32_t(x2 - x3) * (y - y3);
			const int32_t d3 = int32_t(x - x1) * (y3 - y1) - int32_t(x3 - x1) * (y - y1);

			const bool has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
			const bool has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

			// Inside (or on edge) if not both positive and negative areas present.
			return !(has_neg && has_pos);
		}

		// Exact integer floor/ceil division helpers
		static int32_t FloorDiv32(int32_t a, int16_t b)
		{
			// NB: b must be != 0
			int32_t q = a / b;
			int16_t r = a % b;
			if (r != 0 && ((r < 0) != (b < 0))) --q;
			return q;
		}

		static int32_t CeilDiv32(int32_t a, int16_t b)
		{
			// NB: b must be != 0
			int32_t q = a / b;
			int16_t r = a % b;
			if (r != 0 && ((r > 0) == (b > 0))) ++q;
			return q;
		}

		static bool IsInsideEdge(const point2d_t& p, const ClipEdgeEnum edge, const int16_t w, const int16_t h)
		{
			switch (edge)
			{
			case ClipEdgeEnum::Left:
				return p.x >= 0;        // inclusive
			case ClipEdgeEnum::Right:
				return p.x <= (w - 1);  // effectively x < w
			case ClipEdgeEnum::Top:
				return p.y >= 0;        // inclusive
			case ClipEdgeEnum::Bottom:
				return p.y <= (h - 1);  // effectively y < h
			default:
				break;
			}
			return false;
		}

		static point2d_t IntersectWithEdge(const point2d_t& s, const point2d_t& e, ClipEdgeEnum edge, const int16_t w, const int16_t h)
		{
			point2d_t i = s;
			const int_fast16_t dx = e.x - s.x;
			const int_fast16_t dy = e.y - s.y;

			switch (edge)
			{
			case ClipEdgeEnum::Left:
				// x = 0, left/top edges are inclusive -> ceil
				if (dx != 0)
				{
					const int32_t t_num = int32_t(-s.x) * dy;
					const int32_t t = CeilDiv32(t_num, dx);
					i.y = int16_t(int32_t(s.y) + t);
				}
				i.x = 0;
				break;
			case ClipEdgeEnum::Right:
				// x = w-1, right/bottom edges are exclusive -> floor
				if (dx != 0)
				{
					const int32_t t_num = int32_t((w - 1) - s.x) * dy;
					const int32_t t = FloorDiv32(t_num, dx);
					i.y = int16_t(int32_t(s.y) + t);
				}
				i.x = int16_t(w - 1);
				break;
			case ClipEdgeEnum::Top:
				// y = 0, left/top edges are inclusive -> ceil
				if (dy != 0)
				{
					const int32_t t_num = int32_t(-s.y) * dx;
					const int32_t t = CeilDiv32(t_num, dy);
					i.x = int16_t(int32_t(s.x) + t);
				}
				i.y = 0;
				break;
			case ClipEdgeEnum::Bottom:
				// y = h-1, right/bottom edges are exclusive -> floor
				if (dy != 0)
				{
					const int32_t t_num = int32_t((h - 1) - s.y) * dx;
					const int32_t t = FloorDiv32(t_num, dy);
					i.x = int16_t(int32_t(s.x) + t);
				}
				i.y = int16_t(h - 1);
				break;
			}

			return i;
		}

		static void PushUnique(point2d_t* out, uint8_t& outCount, const point2d_t& p)
		{
			if (outCount == 0 || out[outCount - 1].x != p.x || out[outCount - 1].y != p.y)
			{
				out[outCount++] = p;
			}
		}

		static uint8_t ClipAgainstEdge(const point2d_t* inPts, const uint8_t inCount, point2d_t* outPts, ClipEdgeEnum edge, const int16_t w, const int16_t h)
		{
			uint8_t outCount = 0;
			if (inCount == 0)
				return 0;

			point2d_t prev = inPts[inCount - 1];
			bool prevInside = IsInsideEdge(prev, edge, w, h);

			for (uint8_t i = 0; i < inCount; ++i)
			{
				const point2d_t curr = inPts[i];
				const bool currInside = IsInsideEdge(curr, edge, w, h);

				if (prevInside && currInside)
				{
					PushUnique(outPts, outCount, curr);
				}
				else if (prevInside && !currInside)
				{
					point2d_t inter = IntersectWithEdge(prev, curr, edge, w, h);
					PushUnique(outPts, outCount, inter);
				}
				else if (!prevInside && currInside)
				{
					point2d_t inter = IntersectWithEdge(prev, curr, edge, w, h);
					PushUnique(outPts, outCount, inter);
					PushUnique(outPts, outCount, curr);
				}
				// else both outside: skip

				prev = curr;
				prevInside = currInside;
			}

			// remove closing duplicate if any
			if (outCount >= 2 && outPts[0].x == outPts[outCount - 1].x && outPts[0].y == outPts[outCount - 1].y)
			{
				outCount--;
			}

			return outCount;
		}

		
	}
}
#endif