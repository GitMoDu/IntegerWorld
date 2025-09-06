#ifndef _INTEGER_WORLD_ABSTRACT_3D_DRAWER_h
#define _INTEGER_WORLD_ABSTRACT_3D_DRAWER_h

#include "Abstract2dRasterizer.h"

namespace IntegerWorld
{
	/// <summary>
	/// 3D Draw helpers layered on top of the 2D pipeline (non-shader path).
	/// - Performs z-plane (z=0) visibility tests/clipping for points, lines and triangles given as vertex16_t.
	/// - Forwards visible primitives to the 2D Draw* routines (DrawPixel/DrawLine/DrawTriangle), using x/y as screen-space.
	/// - Re-exposes 2D overloads to avoid name hiding.
	/// Note: For shaderable 3D rasterization (Raster* with per-pixel shaders), use Abstract3dRasterizer.
	/// </summary>
	/// <typeparam name="SurfaceType">The output surface type (IOutputSurface-like API).</typeparam>
	template<typename SurfaceType>
	class Abstract3dDrawer : public Abstract2dRasterizer<SurfaceType>
	{
	public:
		// Re-expose 2D overloads to prevent hiding by the 3D Draw* overloads below.
		using Abstract2dRasterizer<SurfaceType>::Fill;
		using Abstract2dRasterizer<SurfaceType>::DrawPixel;
		using Abstract2dRasterizer<SurfaceType>::DrawLine;       // 2D variant
		using Abstract2dRasterizer<SurfaceType>::DrawTriangle;   // 2D variant
		using Abstract2dRasterizer<SurfaceType>::DrawRectangle;  // 2D variant
		using Abstract2dRasterizer<SurfaceType>::RasterLine;     // 2D variant (shader path)
		using Abstract2dRasterizer<SurfaceType>::RasterTriangle; // 2D variant (shader path)
		using Abstract2dRasterizer<SurfaceType>::RasterRectangle;// 2D variant (shader path)

	public:
		/// <summary>
		/// Constructs a 3D drawer that targets the provided surface.
		/// </summary>
		/// <param name="surface">Output surface to draw to.</param>
		Abstract3dDrawer(SurfaceType& surface)
			: Abstract2dRasterizer<SurfaceType>(surface)
		{
		}

	public:
		/// <summary>
		/// Draws a point at the specified 3D vertex when z >= 0 (in front of the screen plane).
		/// Forwards to DrawPixel(x, y).
		/// </summary>
		/// <param name="color">Point color.</param>
		/// <param name="point">3D vertex with x/y in screen space and z for visibility.</param>
		void DrawPoint(const Rgb8::color_t color, const vertex16_t& point)
		{
			if (point.z >= 0)
			{
				DrawPixel(color, point.x, point.y);
			}
		}

		/// <summary>
		/// Draws a 3D line segment with z-plane clipping:
		/// - If both endpoints are in front (z >= 0), forwards to 2D DrawLine.
		/// - If both are behind, nothing is drawn.
		/// - If partially visible, intersects the segment with z=0 and draws only the visible portion.
		/// </summary>
		/// <param name="color">Line color.</param>
		/// <param name="start">Starting vertex (x/y in screen space, z for visibility).</param>
		/// <param name="end">Ending vertex (x/y in screen space, z for visibility).</param>
		void DrawLine(const Rgb8::color_t color, const vertex16_t& start, const vertex16_t& end)
		{
			if (start.z == end.z)
			{
				// Screen plane 2D line, only need to check one point for bounds.
				if (start.z >= 0)
				{
					DrawLine(color, start.x, start.y, end.x, end.y);
				}
				else
				{
					// Whole line out of z-bounds.
				}
			}
			else
			{
				uint8_t inBounds = 0;
				inBounds += start.z >= 0;
				inBounds += end.z >= 0;

				if (inBounds == 0)
				{
					// Whole line out of z-bounds.
				}
				else if (inBounds == 2)
				{
					// Whole line is in bounds.
					DrawLine(color, start.x, start.y, end.x, end.y);
				}
				else
				{
					// Decompose line on partial out-of-bounds.
					// Find which endpoint is in front
					int16_t x1, y1, z1, x2, y2, z2;
					if (start.z < end.z)
					{
						x1 = start.x; y1 = start.y; z1 = start.z;
						x2 = end.x;   y2 = end.y;   z2 = end.z;
					}
					else
					{
						x1 = end.x;   y1 = end.y;   z1 = end.z;
						x2 = start.x; y2 = start.y; z2 = start.z;
					}

					const int16_t dz = z1 - z2;
					if (dz != 0)
					{
						// Intersect with z=0 plane using integer math.
						int16_t t_num = z1;
						int16_t t_den = dz;

						const int16_t ix = x1 + (int16_t)(((int32_t)(x2 - x1) * t_num) / t_den);
						const int16_t iy = y1 + (int16_t)(((int32_t)(y2 - y1) * t_num) / t_den);

						DrawLine(color, x1, y1, ix, iy);
					}
					else
					{
						// Degenerate line.
						DrawPixel(color, x2, y2);
					}
				}
			}
		}

		/// <summary>
		/// Draws a 3D triangle with z-plane handling:
		/// - If all three vertices are on the screen plane with z > 0, forwards to 2D DrawTriangle.
		/// - If all three are behind (z <= 0), nothing is drawn.
		/// - TODO: partial visibility (1 or 2 vertices in front) can be clipped and triangulated.
		/// </summary>
		/// <param name="color">Fill color.</param>
		/// <param name="a">First vertex (x/y in screen space, z for visibility).</param>
		/// <param name="b">Second vertex (x/y in screen space, z for visibility).</param>
		/// <param name="c">Third vertex (x/y in screen space, z for visibility).</param>
		void DrawTriangle(const Rgb8::color_t color, const vertex16_t& a, const vertex16_t& b, const vertex16_t& c)
		{
			if (a.z == b.z
				&& a.z == c.z)
			{
				// Screen plane 2D triangle, only need to check one point for bounds.
				if (a.z > 0)
				{
					DrawTriangle(color, a.x, a.y, b.x, b.y, c.x, c.y);
				}
				else
				{
					// Whole triangle out of z-bounds.
				}
			}
			else
			{
				const uint8_t inBounds = (a.z > 0) + (b.z > 0) + (c.z > 0);

				switch (inBounds)
				{
				case 0:
					// Whole triangle out of z-bounds.
					break;
				case 1:
					//TODO: Triangle has 1 point in bounds.
					break;
				case 2:
					//TODO: Triangle has 2 points in bounds.
					break;
				case 3:
					// Whole triangle is in bounds.
					DrawTriangle(color, a.x, a.y, b.x, b.y, c.x, c.y);
					break;
				default:
					break;
				}
			}
		}

	protected:
		/// <summary>
		/// Returns true when the vertex is inside the 2D window and z >= 0 (in front of the screen plane).
		/// </summary>
		/// <param name="point">3D vertex with x/y in screen space and z for visibility.</param>
		/// <returns>True if visible in the current window.</returns>
		bool IsInsideWindow(const vertex16_t& point) const
		{
			return point.z >= 0 && Abstract2dRasterizer<SurfaceType>::IsInsideWindow(point.x, point.y);
		}
	};
}
#endif