#ifndef _INTEGER_WORLD_WINDOW_RASTERIZER_h
#define _INTEGER_WORLD_WINDOW_RASTERIZER_h

#include "ColorFraction.h"
#include "IOutputSurface.h"

namespace IntegerWorld
{
	class WindowRasterizer
	{
	private:
		static constexpr uint8_t BRESENHAM_SCALE = 16;

	protected:
		int16_t SurfaceWidth;
		int16_t SurfaceHeight;

	public:
		IOutputSurface* Surface;

		uint16_t Width() const
		{
			return SurfaceWidth;
		}

		uint16_t Height() const
		{
			return SurfaceHeight;
		}

	public:
		/// <summary>
		/// Features:
		/// - Direct Mode 2D/3D Drawing API.
		/// - Draws to generic IOutputSurface
		/// - Clipped Draw Window. 
		/// </summary>
		/// <param name="framebufferDrawer">IOutputSurface instance.</param>
		/// <param name="startFullscreen">Set the window to fullscreen by default. Requires framebufferDrawer at constructor.</param>
		WindowRasterizer(IOutputSurface* framebufferDrawer, const bool startFullscreen = true)
			: Surface(framebufferDrawer)
		{
			if (startFullscreen && Surface != nullptr)
			{
				int16_t width, height;
				uint8_t colorDepth;
				Surface->GetSurfaceDimensions(width, height, colorDepth);

				SetSize(width, height);
			}
		}

	public:
		void SetSize(const int16_t width, const int16_t height)
		{
			SurfaceWidth = width;
			SurfaceHeight = height;
		}

	public:// Screen drawing interface.
		/// <summary>
		/// Draws a point at the specified coordinates with the given color, if the point is inside the window.
		/// </summary>
		/// <param name="color">The color to use for the point, represented as a color_fraction16_t value.</param>
		/// <param name="x">The x-coordinate of the point.</param>
		/// <param name="y">The y-coordinate of the point.</param>
		void DrawPoint(const color_fraction16_t color, const int16_t x, const int16_t y)
		{
			if (IsInsideWindow(x, y))
			{
				Surface->Pixel(color, x, y);
			}
		}

		/// <summary>
		/// Draws a point at the specified 3D vertex if its z-coordinate is non-negative.
		/// </summary>
		/// <param name="color">The color to use when drawing the point.</param>
		/// <param name="point">The 3D vertex specifying the location of the point.</param>
		void DrawPoint(const color_fraction16_t color, const vertex16_t point)
		{
			if (point.z >= 0)
			{
				DrawPoint(color, point.x, point.y);
			}
		}

		/// <summary>
		/// Draws a line between two points, clipping the line to the window boundaries if necessary.
		/// </summary>
		/// <param name="color">The color to use for the line, specified as a color_fraction16_t value.</param>
		/// <param name="x1">The x-coordinate of the starting point of the line.</param>
		/// <param name="y1">The y-coordinate of the starting point of the line.</param>
		/// <param name="x2">The x-coordinate of the ending point of the line.</param>
		/// <param name="y2">The y-coordinate of the ending point of the line.</param>
		void DrawLine(const color_fraction16_t color, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2)
		{
			const bool in1 = IsInsideWindow(x1, y1);
			const bool in2 = IsInsideWindow(x2, y2);

			int16_t x1c(x1), y1c(y1), x2c(x2), y2c(y2);

			if (in1 && in2)
			{
				// Whole line is inside window.
			}
			else if (in1)
			{
				ClipEndpointToWindow(x2c, y2c, x1c, y1c);
				if (!IsInsideWindow(x2c, y2c))
				{
					return;
				}
			}
			else if (in2)
			{
				ClipEndpointToWindow(x1c, y1c, x2c, y2c);
				if (!IsInsideWindow(x1c, y1c))
				{
					return;
				}
			}
			else // Both endpoints are outside the window.
			{
				// Trivial reject: both endpoints outside on the same side of the window
				uint8_t out1 = 0, out2 = 0;
				if (x1 < 0) out1 |= 1; else if (x1 >= SurfaceWidth) out1 |= 2;
				if (y1 < 0) out1 |= 4; else if (y1 >= SurfaceHeight) out1 |= 8;
				if (x2 < 0) out2 |= 1; else if (x2 >= SurfaceWidth) out2 |= 2;
				if (y2 < 0) out2 |= 4; else if (y2 >= SurfaceHeight) out2 |= 8;
				if (out1 & out2)
					return; // Both endpoints are outside on the same side

				// Try to clip remaining endpoints to the window.
				ClipEndpointToWindow(x1c, y1c, x2, y2);
				ClipEndpointToWindow(x2c, y2c, x1, y1);

				// If either endpoint is not inside, or the segment is degenerate, reject.
				if (!IsInsideWindow(x1c, y1c) || !IsInsideWindow(x2c, y2c))
					return; // No visible segment after clipping
			}

			if (x1c == x2c && y1c == y2c)
			{
				Surface->Pixel(color, x1c, y1c);
			}
			else
			{
				Surface->Line(color, x1c, y1c, x2c, y2c);
			}
		}

		/// <summary>
		/// Draws a 3D line segment projected onto a 2D screen, handling cases where endpoints may be out of bounds in the z-direction.
		/// </summary>
		/// <param name="color">The color to use when drawing the line, specified as a color_fraction16_t value.</param>
		/// <param name="start">The starting vertex of the line, specified as a vertex16_t structure containing x, y, and z coordinates.</param>
		/// <param name="end">The ending vertex of the line, specified as a vertex16_t structure containing x, y, and z coordinates.</param>
		void DrawLine(const color_fraction16_t color, const vertex16_t start, const vertex16_t end)
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
						// Interpolate intersection with z=0 plane using integer math
						int16_t t_num = z1;
						int16_t t_den = dz;

						const int16_t ix = x1 + (int16_t)(((int32_t)(x2 - x1) * t_num) / t_den);
						const int16_t iy = y1 + (int16_t)(((int32_t)(y2 - y1) * t_num) / t_den);

						DrawLine(color, x1, y1, ix, iy);
					}
					else
					{
						// Degenerate line.
						DrawPoint(color, x2, y2);
					}
				}
			}
		}

		/// <summary>
		/// Draws a filled triangle with the specified color and vertex coordinates, clipping it to the window boundaries if necessary.
		/// </summary>
		/// <param name="color">The color to fill the triangle, specified as a color_fraction16_t value.</param>
		/// <param name="x1">The x-coordinate of the first vertex of the triangle.</param>
		/// <param name="y1">The y-coordinate of the first vertex of the triangle.</param>
		/// <param name="x2">The x-coordinate of the second vertex of the triangle.</param>
		/// <param name="y2">The y-coordinate of the second vertex of the triangle.</param>
		/// <param name="x3">The x-coordinate of the third vertex of the triangle.</param>
		/// <param name="y3">The y-coordinate of the third vertex of the triangle.</param>
		void DrawTriangle(const color_fraction16_t color, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2, const int16_t x3, const int16_t y3)
		{
			// Count how many vertices are inside the window
			const bool in1 = IsInsideWindow(x1, y1);
			const bool in2 = IsInsideWindow(x2, y2);
			const bool in3 = IsInsideWindow(x3, y3);

			const uint8_t insideCount = (uint8_t)in1 + (uint8_t)in2 + (uint8_t)in3;
			if (insideCount == 3)
			{
				// The whole triangle fits in the window.
				Surface->TriangleFill(color,
					x1, y1,
					x2, y2,
					x3, y3);
			}
			else if (insideCount == 2)
			{
				// Find the two inside vertices and the outside vertex
				int16_t ax, ay, bx, by, cx, cy;
				if (!in1)
				{
					// x1,y1 is outside
					ax = x2; ay = y2; bx = x3; by = y3; cx = x1; cy = y1;
				}
				else if (!in2)
				{
					// x2,y2 is outside
					ax = x1; ay = y1; bx = x3; by = y3; cx = x2; cy = y2;
				}
				else
				{
					// x3,y3 is outside
					ax = x1; ay = y1; bx = x2; by = y2; cx = x3; cy = y3;
				}

				// Clip edge AC
				int16_t pcx1 = cx, pcy1 = cy;
				ClipEndpointToWindow(pcx1, pcy1, ax, ay);

				// Clip edge BC
				int16_t pcx2 = cx, pcy2 = cy;
				ClipEndpointToWindow(pcx2, pcy2, bx, by);

				// The clipped polygon is a quad: A, B, pcx2/pcy2, pcx1/pcy1
				// Split into two triangles: (A, B, pcx2/pcy2) and (A, pcx2/pcy2, pcx1/pcy1)
				Surface->TriangleFill(
					color,
					ax, ay,
					bx, by,
					pcx2, pcy2
				);
				Surface->TriangleFill(
					color,
					ax, ay,
					pcx2, pcy2,
					pcx1, pcy1
				);
			}
			else if (insideCount == 1)
			{
				// Find the inside vertex and the two outside vertices
				int16_t ax, ay, bx, by, cx, cy;
				if (in1)
				{
					ax = x1; ay = y1; bx = x2; by = y2; cx = x3; cy = y3;
				}
				else if (in2)
				{
					ax = x2; ay = y2; bx = x3; by = y3; cx = x1; cy = y1;
				}
				else // in3
				{
					ax = x3; ay = y3; bx = x1; by = y1; cx = x2; cy = y2;
				}

				// Clip edge AB
				int16_t pbx = bx, pby = by;
				ClipEndpointToWindow(pbx, pby, ax, ay);

				// Clip edge AC
				int16_t pcx = cx, pcy = cy;
				ClipEndpointToWindow(pcx, pcy, ax, ay);

				// Draw the clipped triangle
				Surface->TriangleFill(color,
					ax, ay,
					pbx, pby,
					pcx, pcy);
			}
			else  // insideCount == 0, !TriangleCoversWindow
			{
				// All vertices are outside the window.
				//TODO: Check if triangle intersects the window and draw remaining are with polygon sub-division.
			}
		}

		/// <summary>
		/// Draws a triangle defined by three 3D vertices and a color, handling cases where the triangle is partially or fully out of the z-bounds.
		/// </summary>
		/// <param name="color">The color to use for filling the triangle.</param>
		/// <param name="a">The first vertex of the triangle, including x, y, and z coordinates.</param>
		/// <param name="b">The second vertex of the triangle, including x, y, and z coordinates.</param>
		/// <param name="c">The third vertex of the triangle, including x, y, and z coordinates.</param>
		void DrawTriangle(const color_fraction16_t color, const vertex16_t a, const vertex16_t b, const vertex16_t c)
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

				if (inBounds == 3)
				{
					// Whole triangle is in bounds.
					DrawTriangle(color, a.x, a.y, b.x, b.y, c.x, c.y);
				}
				else if (inBounds == 0)
				{
					// Whole triangle out of z-bounds.
				}
				else if (inBounds == 2)
				{
					//TODO: Triangle has 2 points in bounds.
				}
				else
				{
					//TODO: Triangle has 1 point in bounds.
				}
			}
		}

		/// <summary>
		/// Draws a filled rectangle, cropping it to the visible window if necessary.
		/// </summary>
		/// <param name="color">The color to use for filling the rectangle.</param>
		/// <param name="x1">The x-coordinate of the first corner of the rectangle.</param>
		/// <param name="y1">The y-coordinate of the first corner of the rectangle.</param>
		/// <param name="x2">The x-coordinate of the opposite corner of the rectangle.</param>
		/// <param name="y2">The y-coordinate of the opposite corner of the rectangle.</param>
		void DrawRectangle(const color_fraction16_t color, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2)
		{
			uint8_t inCount = IsInsideWindow(x1, y1);
			inCount += IsInsideWindow(x1, y2);
			inCount += IsInsideWindow(x2, y1);
			inCount += IsInsideWindow(x2, y2);

			if (inCount > 0)
			{
				const int16_t x1c = LimitValue(x1, int16_t(0), int16_t(SurfaceWidth - 1));
				const int16_t x2c = LimitValue(x2, int16_t(0), int16_t(SurfaceWidth - 1));
				const int16_t y1c = LimitValue(y1, int16_t(0), int16_t(SurfaceHeight - 1));
				const int16_t y2c = LimitValue(y2, int16_t(0), int16_t(SurfaceHeight - 1));

				if (x1c == x2c)
				{
					if (y1c == y2c)
					{
						// Degenerate rectangle, only draw a single pixel.
						Surface->Pixel(color, x1c, y1c);
					}
					else
					{
						// Degenerate rectangle, only draw a line.
						Surface->Line(color, x1c, y1c, x1c, y2c);
					}
				}
				else if (y1c == y2c)
				{
					// Degenerate rectangle, only draw a line.
					Surface->Line(color, x1c, y1c, x2c, y1c);
				}
				else
				{
					// Cropped rectangle.
					Surface->RectangleFill(color, x1c, y1c, x2c, y2c);
				}
			}
			else
			{
				// Whole rectangle is out of bounds.
			}
		}

		/// <summary>
		/// Fills the entire drawing surface with the specified color.
		/// </summary>
		/// <param name="color">The color to use for filling surface</param>
		void FillSurface(const color_fraction16_t color)
		{
			Surface->RectangleFill(color, 0, 0, SurfaceWidth - 1, SurfaceHeight - 1);
		}

	public: // 3D Projection drawing (raster) interface.
		/// <summary>
		/// Draws a line between two points on the framebuffer, applying a custom pixel shader to each pixel.
		/// The line is clipped to the window boundaries as needed.
		/// </summary>
		/// <typeparam name="PixelShader">The type of the pixel shader callable, which must be invocable with (color_fraction16_t&, int, int) and return a bool indicating whether to draw the pixel.</typeparam>
		/// <param name="x1">The x-coordinate of the starting point of the line.</param>
		/// <param name="y1">The y-coordinate of the starting point of the line.</param>
		/// <param name="x2">The x-coordinate of the ending point of the line.</param>
		/// <param name="y2">The y-coordinate of the ending point of the line.</param>
		/// <param name="pixelShader">A callable object or function that determines the color and visibility of each pixel along the line. It is called with the color, relative x offset, and relative y offset.</param>
		template<typename PixelShader>
		void RasterLine(const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2, PixelShader&& pixelShader)
		{
			const bool in1 = IsInsideWindow(x1, y1);
			const bool in2 = IsInsideWindow(x2, y2);

			int16_t x1c(x1), y1c(y1), x2c(x2), y2c(y2);

			if (in1 && in2)
			{
				// Whole line is inside window.
			}
			else if (in1)
			{
				ClipEndpointToWindow(x2c, y2c, x1c, y1c);
			}
			else if (in2)
			{
				ClipEndpointToWindow(x1c, y1c, x2c, y2c);
			}
			else // Both endpoints are outside the window.
			{
				// Trivial reject: both endpoints outside on the same side of the window
				uint8_t out1 = 0, out2 = 0;
				if (x1 < 0) out1 |= 1; else if (x1 >= SurfaceWidth) out1 |= 2;
				if (y1 < 0) out1 |= 4; else if (y1 >= SurfaceHeight) out1 |= 8;
				if (x2 < 0) out2 |= 1; else if (x2 >= SurfaceWidth) out2 |= 2;
				if (y2 < 0) out2 |= 4; else if (y2 >= SurfaceHeight) out2 |= 8;
				if (out1 & out2)
					return;

				// Try to clip remaining endpoints to the window.
				ClipEndpointToWindow(x1c, y1c, x2, y2);
				ClipEndpointToWindow(x2c, y2c, x1, y1);

				// If either endpoint is not inside, or the segment is degenerate, reject.
				if (!IsInsideWindow(x1c, y1c) || !IsInsideWindow(x2c, y2c))
					return; // No visible segment after clipping.
			}

			const int16_t width = (x2c - x1c);
			const int16_t height = (y2c - y1c);

			if (width == 0 && height == 0)
			{
				// Degenerate line, only draw a single pixel.
				color_fraction16_t color{};
				if (pixelShader(color, x1c - x1, y1c - y1))
				{
					Surface->Pixel(color, x1c, y1c);
				}
			}
			else if (height == 0)
			{
				// Horizontal line.
				color_fraction16_t color{};
				for (int_fast16_t x = 0; x <= width; x++)
				{
					if (pixelShader(color, x + x1c - x1, y1c - y1))
					{
						Surface->Pixel(color, x1c + x, y1c);
					}
				}
			}
			else if (width == 0)
			{
				// Vertical line.
				color_fraction16_t color{};
				for (int_fast16_t y = 0; y <= height; y++)
				{
					if (pixelShader(color, x1c - x1, y + y1c - y1))
					{
						Surface->Pixel(color, x1c, y1c + y);
					}
				}
			}
			else
			{
				// Diagonal line. Unroll to appropriate Bresenham line rasterizer.
				BresenhamLineShade(x1c, y1c, x2c, y2c, x1, y1, pixelShader);
			}
		}

		/// <summary>
		/// Rasterizes a 3D line segment with Z-plane clipping and applies a pixel shader to each pixel along the line.
		/// The line is clipped to the window boundaries as needed.
		/// </summary>
		/// <typeparam name="PixelShader">The type of the pixel shader callable, which must be invocable with (color_fraction16_t&, int, int) and return a bool indicating whether to draw the pixel.</typeparam>
		/// <param name="start">The starting vertex of the line segment, containing x, y, and z coordinates.</param>
		/// <param name="end">The ending vertex of the line segment, containing x, y, and z coordinates.</param>
		/// <param name="pixelShader">A callable object or function that determines the color and visibility of each pixel along the line. It is called with the color, relative x offset, and relative y offset.</param>
		template<typename PixelShader>
		void RasterLine(const vertex16_t start, const vertex16_t end, PixelShader&& pixelShader)
		{
			// Z-plane clipping: only draw if at least part of the line is in front of z=0
			const bool in1 = start.z >= 0;
			const bool in2 = end.z >= 0;

			vertex16_t s = start;
			vertex16_t e = end;

			if (!in1 && !in2)
			{
				// Both endpoints are behind the z=0 plane, nothing to draw
				return;
			}
			else if (!in1 || !in2)
			{
				// One endpoint is behind z=0, clip to z=0 plane
				// Interpolate intersection with z=0
				const vertex16_t* front = in1 ? &s : &e;
				const vertex16_t* back = in1 ? &e : &s;

				int16_t dz = front->z - back->z;
				if (dz == 0) return; // Degenerate, should not happen

				// t = front->z / (front->z - back->z)
				int16_t t_num = front->z;
				int16_t t_den = dz;

				int16_t ix = front->x + (int16_t)(((int32_t)(back->x - front->x) * t_num) / t_den);
				int16_t iy = front->y + (int16_t)(((int32_t)(back->y - front->y) * t_num) / t_den);

				if (in1)
				{
					e.x = ix;
					e.y = iy;
					e.z = 0;
				}
				else
				{
					s.x = ix;
					s.y = iy;
					s.z = 0;
				}
			}

			// Now both s.z and e.z >= 0, so we can rasterize in 2D
			RasterLine(s.x, s.y, e.x, e.y, pixelShader);
		}


		template<typename PixelShader>
		void RasterTriangle(const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2, const int16_t x3, const int16_t y3, PixelShader&& pixelShader)
		{
			// Count how many vertices are inside the window
			const bool in1 = IsInsideWindow(x1, y1);
			const bool in2 = IsInsideWindow(x2, y2);
			const bool in3 = IsInsideWindow(x3, y3);

			const uint8_t insideCount = (uint8_t)in1 + (uint8_t)in2 + (uint8_t)in3;
			if (insideCount == 3)
			{
				// The whole triangle fits in the window.
				TriangleYRaster(x1, y1, x2, y2, x3, y3, pixelShader);
			}
			else if (insideCount == 2)
			{
				// Find the two inside vertices and the outside vertex
				int16_t ax, ay, bx, by, cx, cy;
				if (!in1)
				{
					// x1,y1 is outside
					ax = x2; ay = y2; bx = x3; by = y3; cx = x1; cy = y1;
				}
				else if (!in2)
				{
					// x2,y2 is outside
					ax = x1; ay = y1; bx = x3; by = y3; cx = x2; cy = y2;
				}
				else
				{
					// x3,y3 is outside
					ax = x1; ay = y1; bx = x2; by = y2; cx = x3; cy = y3;
				}

				// Clip edge AC
				int16_t pcx1 = cx, pcy1 = cy;
				ClipEndpointToWindow(pcx1, pcy1, ax, ay);

				// Clip edge BC
				int16_t pcx2 = cx, pcy2 = cy;
				ClipEndpointToWindow(pcx2, pcy2, bx, by);

				// The clipped polygon is a quad: A, B, pcx2/pcy2, pcx1/pcy1
				// Split into two triangles: (A, B, pcx2/pcy2) and (A, pcx2/pcy2, pcx1/pcy1)
				TriangleYRaster(ax, ay,
					bx, by,
					pcx2, pcy2, pixelShader);
				TriangleYRaster(ax, ay,
					pcx2, pcy2,
					pcx1, pcy1, pixelShader);
			}
			else if (insideCount == 1)
			{
				// Find the inside vertex and the two outside vertices
				int16_t ax, ay, bx, by, cx, cy;
				if (in1)
				{
					ax = x1; ay = y1; bx = x2; by = y2; cx = x3; cy = y3;
				}
				else if (in2)
				{
					ax = x2; ay = y2; bx = x3; by = y3; cx = x1; cy = y1;
				}
				else // in3
				{
					ax = x3; ay = y3; bx = x1; by = y1; cx = x2; cy = y2;
				}

				// Clip edge AB
				int16_t pbx = bx, pby = by;
				ClipEndpointToWindow(pbx, pby, ax, ay);

				// Clip edge AC
				int16_t pcx = cx, pcy = cy;
				ClipEndpointToWindow(pcx, pcy, ax, ay);

				// Draw the clipped triangle
				TriangleYRaster(ax, ay,
					pbx, pby,
					pcx, pcy, pixelShader);
			}
			else  // insideCount == 0, !TriangleCoversWindow
			{
				// All vertices are outside the window.
				//TODO: Check if triangle intersects the window and draw remaining are with polygon sub-division.
			}
		}

		template<typename PixelShader>
		void RasterTriangle(const vertex16_t a, const vertex16_t b, const vertex16_t c, PixelShader&& pixelShader)
		{
			if (a.z == b.z
				&& a.z == c.z)
			{
				// Screen plane 2D triangle, only need to check one point for bounds.
				if (a.z > 0)
				{
					RasterTriangle(a.x, a.y, b.x, b.y, c.x, c.y, pixelShader);
				}
				else
				{
					// Whole triangle out of z-bounds.
				}
			}
			else
			{
				const uint8_t inBounds = (a.z > 0) + (b.z > 0) + (c.z > 0);

				if (inBounds == 3)
				{
					// Whole triangle is in bounds.
					RasterTriangle(a.x, a.y, b.x, b.y, c.x, c.y, pixelShader);
				}
				else if (inBounds == 0)
				{
					// Whole triangle out of z-bounds.
				}
				else if (inBounds == 2)
				{
					//TODO: Triangle has 2 points in bounds.
				}
				else
				{
					//TODO: Triangle has 1 point in bounds.
				}
			}
		}

		/// <summary>
		/// Renders a rectangle on the framebuffer using a custom pixel shader, handling clipping to the surface boundaries and degenerate cases (lines or points).
		/// </summary>
		/// <typeparam name="PixelShader">The type of the pixel shader callable, which must be invocable with (color_fraction16_t&, int16_t, int16_t) and return a bool indicating whether to draw the pixel.</typeparam>
		/// <param name="x1">The x-coordinate of the first corner of the rectangle.</param>
		/// <param name="y1">The y-coordinate of the first corner of the rectangle.</param>
		/// <param name="x2">The x-coordinate of the opposite corner of the rectangle.</param>
		/// <param name="y2">The y-coordinate of the opposite corner of the rectangle.</param>
		/// <param name="pixelShader">A callable object or function that determines the color of each pixel. It is called with (color, dx, dy), where 'color' is an output parameter and 'dx', 'dy' are the pixel's coordinates relative to the rectangle's origin.</param>
		template<typename PixelShader>
		void RasterRectangle(const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2, PixelShader&& pixelShader)
		{
			uint8_t inCount = IsInsideWindow(x1, y1);
			inCount += IsInsideWindow(x1, y2);
			inCount += IsInsideWindow(x2, y1);
			inCount += IsInsideWindow(x2, y2);

			if (inCount > 0)
			{
				color_fraction16_t color{};
				const int16_t x1c = LimitValue(x1, int16_t(0), int16_t(SurfaceWidth - 1));
				const int16_t x2c = LimitValue(x2, int16_t(0), int16_t(SurfaceWidth - 1));
				const int16_t y1c = LimitValue(y1, int16_t(0), int16_t(SurfaceHeight - 1));
				const int16_t y2c = LimitValue(y2, int16_t(0), int16_t(SurfaceHeight - 1));

				if (x1c == x2c)
				{
					if (y1c == y2c)
					{
						// Degenerate rectangle, only draw a single pixel.
						if (pixelShader(color, x1c - x1, y1c - y1))
						{
							Surface->Pixel(color, x1c, y1c);
						}
					}
					else
					{
						// Degenerate rectangle, only draw a line.
						const int16_t height = (y2c - y1c);
						for (int_fast16_t y = 0; y <= height; y++)
						{
							if (pixelShader(color, x1c - x1, y + y1c - y1))
							{
								Surface->Pixel(color, x1c, y1c + y);
							}
						}
					}
				}
				else if (y1c == y2c)
				{
					// Degenerate rectangle, only draw a line.
					const int16_t width = (x2c - x1c);
					for (int_fast16_t x = 0; x <= width; x++)
					{
						if (pixelShader(color, x + x1c - x1, y1c - y1))
						{
							Surface->Pixel(color, x1c + x, y1c);
						}
					}
				}
				else
				{
					// Cropped rectangle.
					const int16_t height = (y2c - y1c);
					const int16_t width = (x2c - x1c);
					for (int_fast16_t y = 0; y <= height; y++)
					{
						for (int_fast16_t x = 0; x <= width; x++)
						{
							// Convert partial rect coordinates to rect space coordinates.
							if (pixelShader(color, x + x1c - x1, y + y1c - y1))
							{
								Surface->Pixel(color, x1c + x, y1c + y);
							}
						}
					}
				}
			}
			else
			{
				// Whole rectangle is out of bounds.
			}
		}


	public:
		bool IsInsideWindow(const int16_t x, const int16_t y) const
		{
			return x >= 0 && x < SurfaceWidth
				&& y >= 0 && y < SurfaceHeight;
		}

		bool IsInsideWindow(const vertex16_t point) const
		{
			return point.z >= 0 && IsInsideWindow(point.x, point.y);
		}

		void LimitToWindow(int16_t& x, int16_t& y) const
		{
			x = LimitValue(x, int16_t(0), int16_t(SurfaceWidth));
			y = LimitValue(y, int16_t(0), int16_t(SurfaceHeight));
		}
	private:
		/// <summary>
		/// Clips the endpoint (x1, y1) of a line segment to the boundaries of the window, using (x2, y2) as the other endpoint.
		/// </summary>
		/// <param name="x1">Reference to the x-coordinate of the endpoint to be clipped. Modified in place if clipping occurs.</param>
		/// <param name="y1">Reference to the y-coordinate of the endpoint to be clipped. Modified in place if clipping occurs.</param>
		/// <param name="x2">The x-coordinate of the fixed endpoint of the line segment.</param>
		/// <param name="y2">The y-coordinate of the fixed endpoint of the line segment.</param>
		void ClipEndpointToWindow(int16_t& x1, int16_t& y1, const int16_t x2, const int16_t y2) const
		{
			const int16_t dx = x2 - x1;
			const int16_t dy = y2 - y1;

			// Left
			if (x1 < 0 && dx != 0)
			{
				y1 += (int16_t)(((-(int32_t)x1) * (int32_t)dy) / (int32_t)dx);
				x1 = 0;
			}
			// Right
			else if (x1 >= SurfaceWidth && dx != 0)
			{
				y1 += (int16_t)(((int32_t)(SurfaceWidth - 1 - x1) * (int32_t)dy) / (int32_t)dx);
				x1 = SurfaceWidth - 1;
			}

			// Top
			if (y1 < 0 && dy != 0)
			{
				x1 += (int16_t)(((-(int32_t)y1) * (int32_t)dx) / (int32_t)dy);
				y1 = 0;
			}
			// Bottom
			else if (y1 >= SurfaceHeight && dy != 0)
			{
				x1 += (int16_t)(((int32_t)(SurfaceHeight - 1 - y1) * (int32_t)dx) / (int32_t)dy);
				y1 = SurfaceHeight - 1;
			}
		}

		/// <summary>
		/// Determines if a point (x, y) lies inside the triangle defined by (x1, y1), (x2, y2), and (x3, y3).
		/// </summary>
		/// <param name="x">X coordinate of the point to test.</param>
		/// <param name="y">Y coordinate of the point to test.</param>
		/// <param name="x1">X coordinate of the first triangle vertex.</param>
		/// <param name="y1">Y coordinate of the first triangle vertex.</param>
		/// <param name="x2">X coordinate of the second triangle vertex.</param>
		/// <param name="y2">Y coordinate of the second triangle vertex.</param>
		/// <param name="x3">X coordinate of the third triangle vertex.</param>
		/// <param name="y3">Y coordinate of the third triangle vertex.</param>
		/// <returns>True if the point (x, y) is inside the triangle; otherwise, false.</returns>
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

			return !has_neg && !has_pos;
		}

		/// <summary>
		/// Determines if the triangle defined by (x1, y1), (x2, y2), and (x3, y3) completely covers the current drawing window.
		/// </summary>
		/// <param name="x1">X coordinate of the first triangle vertex.</param>
		/// <param name="y1">Y coordinate of the first triangle vertex.</param>
		/// <param name="x2">X coordinate of the second triangle vertex.</param>
		/// <param name="y2">Y coordinate of the second triangle vertex.</param>
		/// <param name="x3">X coordinate of the third triangle vertex.</param>
		/// <param name="y3">Y coordinate of the third triangle vertex.</param>
		/// <returns>True if the triangle fully contains all four corners of the window; otherwise, false.</returns>
		bool TriangleCoversWindow(
			const int16_t x1, const int16_t y1,
			const int16_t x2, const int16_t y2,
			const int16_t x3, const int16_t y3) const
		{
			return PointInTriangle(0, 0, x1, y1, x2, y2, x3, y3) &&
				PointInTriangle(SurfaceWidth - 1, 0, x1, y1, x2, y2, x3, y3) &&
				PointInTriangle(0, SurfaceHeight - 1, x1, y1, x2, y2, x3, y3) &&
				PointInTriangle(SurfaceWidth - 1, SurfaceHeight - 1, x1, y1, x2, y2, x3, y3);
		}

	private:
		// Diagonal (Bresenham) Line implementation based on https://www.geeksforgeeks.org/bresenhams-line-generation-algorithm/
		template<typename PixelShader>
		void BresenhamLineShade(int16_t x1, int16_t y1, int16_t x2, int16_t y2,
			int16_t x0, int16_t y0, PixelShader&& pixelShader)
		{
			const int16_t dx = AbsValue(x2 - x1);
			const int8_t sx = x1 < x2 ? 1 : -1;
			const int16_t dy = -AbsValue(y2 - y1);
			const int8_t sy = y1 < y2 ? 1 : -1;
			int16_t err = dx + dy;
			color_fraction16_t color{};

			while (true)
			{
				if (pixelShader(color, x1 - x0, y1 - y0))
				{
					Surface->Pixel(color, x1, y1);
				}
				if (x1 == x2 && y1 == y2)
					break;
				int16_t e2 = 2 * err;
				if (e2 >= dy)
				{
					err += dy;
					x1 += sx;
				}
				if (e2 <= dx)
				{
					err += dx;
					y1 += sy;
				}
			}
		}

		template<typename PixelShader>
		void BresenhamFlatTopFill(const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2, const int16_t x3, const int16_t y3, const int16_t x0, const int16_t y0, PixelShader&& pixelShader)
		{
			// Calculate inverse slopes in fixed-point
			const int32_t invSlope1 = ((int32_t)(x3 - x1) << BRESENHAM_SCALE) / (y3 - y1);
			const int32_t invSlope2 = ((int32_t)(x3 - x2) << BRESENHAM_SCALE) / (y3 - y2);

			// Starting x positions in fixed-point
			int32_t ax = (int32_t)x3 << BRESENHAM_SCALE;
			int32_t bx = ax;

			// Loop from c.y down to a.y (inclusive)
			const int16_t startY = y3;
			const int16_t endY = MaxValue(int16_t(0), y1);

			color_fraction16_t color{};
			for (int16_t y = startY; y >= endY; y--)
			{
				if (y >= 0 && y < SurfaceHeight)
				{
					int16_t xStart{};
					int16_t xEnd{};

					if (ax > bx)
					{
						xStart = bx >> BRESENHAM_SCALE;
						xEnd = ax >> BRESENHAM_SCALE;
					}
					else
					{
						xStart = ax >> BRESENHAM_SCALE;
						xEnd = bx >> BRESENHAM_SCALE;
					}

					xStart = MaxValue(int16_t(0), xStart);
					xEnd = MinValue(int16_t(SurfaceWidth - 1), xEnd);

					if (xStart <= xEnd)
					{
						for (int_fast16_t x = xStart; x <= xEnd; x++)
						{
							if (pixelShader(color, x - x0, y - y0))
							{
								Surface->Pixel(color, x, y);
							}
						}
					}
				}

				ax -= invSlope1;
				bx -= invSlope2;
			}
		}

		template<typename PixelShader>
		void BresenhamFlatBottomFill(const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2, const int16_t x3, const int16_t y3, const int16_t x0, const int16_t y0, PixelShader&& pixelShader)
		{
			// Calculate inverse slopes in fixed-point
			int32_t invSlope1 = 1;
			if (y2 != y1)
			{
				invSlope1 = (int32_t(x2 - x1) << BRESENHAM_SCALE) / (y2 - y1);
			}
			int32_t invSlope2 = 1;
			if (y3 != y1)
			{
				invSlope2 = (int32_t(x3 - x1) << BRESENHAM_SCALE) / (y3 - y1);
			}

			// Starting x positions in fixed-point
			int32_t ax = int32_t(x1) << BRESENHAM_SCALE;
			int32_t bx = ax;

			// Loop from y1 to y2 (inclusive)
			const int16_t startY = y1;
			const int16_t endY = min(int16_t(SurfaceHeight - 1), y2);

			color_fraction16_t color{};
			for (int16_t y = startY; y < endY; y++)
			{
				if (y >= 0 && y < SurfaceHeight)
				{
					int16_t xStart{};
					int16_t xEnd{};

					if (ax > bx)
					{
						xStart = bx >> BRESENHAM_SCALE;
						xEnd = ax >> BRESENHAM_SCALE;
					}
					else
					{
						xStart = ax >> BRESENHAM_SCALE;
						xEnd = bx >> BRESENHAM_SCALE;
					}

					xStart = MaxValue(int16_t(0), xStart);
					xEnd = MinValue(int16_t(SurfaceWidth - 1), xEnd);

					if (xStart <= xEnd)
					{
						for (int_fast16_t x = xStart; x <= xEnd; x++)
						{
							if (pixelShader(color, x - x0, y - y0))
							{
								Surface->Pixel(color, x, y);
							}
						}
					}
				}

				ax += invSlope1;
				bx += invSlope2;
			}
		}

		template<typename PixelShader>
		void TriangleYRaster(const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2, const int16_t x3, const int16_t y3, PixelShader&& pixelShader)
		{
			if (y1 <= y2 && y1 <= y3)
			{
				// Vertex A is at the top.
				if (y2 <= y3)
				{
					TriangleYOrderedRaster(x1, y1, x2, y2, x3, y3, pixelShader);
				}
				else
				{
					TriangleYOrderedRaster(x1, y1, x3, y3, x2, y2, pixelShader);
				}
			}
			else if (y2 <= y1 && y2 <= y3)
			{
				// Vertex B is at the top.
				if (y1 <= y3)
				{
					TriangleYOrderedRaster(x2, y2, x1, y1, x3, y3, pixelShader);
				}
				else
				{
					TriangleYOrderedRaster(x2, y2, x3, y3, x1, y1, pixelShader);
				}
			}
			else
			{
				// Vertex C is at the top.
				if (y1 <= y2)
				{
					TriangleYOrderedRaster(x3, y3, x1, y1, x2, y2, pixelShader);
				}
				else
				{
					TriangleYOrderedRaster(x3, y3, x2, y2, x1, y1, pixelShader);
				}
			}
		}

		template<typename PixelShader>
		void TriangleYOrderedRaster(const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2, const int16_t x3, const int16_t y3, PixelShader&& pixelShader)
		{
			if (y2 == y3) // Flat bottom.
			{
				BresenhamFlatBottomFill(x1, y1, x2, y2, x3, y3, x1, y1, pixelShader);
			}
			else if (y1 == y2) // Flat top.
			{
				BresenhamFlatTopFill(x1, y1, x2, y2, x3, y3, x1, y1, pixelShader);
			}
			else // General triangle: split it.
			{
				// Calculate splitting vertex Vi.
				const int16_t dxTotal = x3 - x1;
				const int16_t dyTotal = y3 - y1;
				const int16_t dySegment = y2 - y1;

				if (dyTotal == 0)
					return; // Degenerate triangle

				// Calculate Vi_x in fixed-point.
				const int16_t Vi_x = SignedRightShift((((int32_t)x1 << BRESENHAM_SCALE) + ((((int32_t)dxTotal << BRESENHAM_SCALE) * dySegment) / dyTotal)), BRESENHAM_SCALE);
				const int16_t Vi_y = y2;

				// Draw the two sub-triangles
				BresenhamFlatBottomFill(x1, y1, x2, y2, Vi_x, Vi_y, x1, y1, pixelShader);
				BresenhamFlatTopFill(x2, y2, Vi_x, Vi_y, x3, y3, x1, y1, pixelShader);
			}
		}
	};
}
#endif