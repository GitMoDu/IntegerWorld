#ifndef _INTEGER_WORLD_ABSTRACT_2D_RASTERIZER_h
#define _INTEGER_WORLD_ABSTRACT_2D_RASTERIZER_h

#include "Abstract2dDrawer.h"

namespace IntegerWorld
{
	/// <summary>
	/// 2D rasterization utilities that operate in window/surface space.
	/// - Adds shaderable rasterization for lines, triangles and rectangles on top of the 2D drawing API.
	/// - Performs window clipping and handles degenerate cases.
	/// </summary>
	/// <typeparam name="SurfaceType">The output surface type (IOutputSurface-like API).</typeparam>
	template<typename SurfaceType>
	class Abstract2dRasterizer : public Abstract2dDrawer<SurfaceType>
	{
	protected:
		using Abstract2dDrawer<SurfaceType>::BRESENHAM_SCALE;

	protected:
		using Abstract2dDrawer<SurfaceType>::Surface;
		using Abstract2dDrawer<SurfaceType>::SurfaceWidth;
		using Abstract2dDrawer<SurfaceType>::SurfaceHeight;
		using Abstract2dDrawer<SurfaceType>::IsInsideWindow;

		using Abstract2dDrawer<SurfaceType>::ClipEndpointToWindow;
		using Abstract2dDrawer<SurfaceType>::ClipTriangleToWindow;

	protected:
		using Abstract2dDrawer<SurfaceType>::clipInputTriangle;
		using Abstract2dDrawer<SurfaceType>::clippedPolygon;
		using Abstract2dDrawer<SurfaceType>::clipScratchA;
		using Abstract2dDrawer<SurfaceType>::clipScratchB;
		using Abstract2dDrawer<SurfaceType>::p0;
		using Abstract2dDrawer<SurfaceType>::p1;
		using Abstract2dDrawer<SurfaceType>::p2;

	public:
		Abstract2dRasterizer(SurfaceType& surface)
			: Abstract2dDrawer<SurfaceType>(surface)
		{
		}

	public:
		/// <summary>
		/// Rasterizes a line between two points, applying a custom pixel shader to each pixel.
		/// The line is clipped to the window boundaries as needed.
		/// </summary>
		/// <typeparam name="PixelShader">Callable signature: bool(Rgb8::color_t&amp; color, int16_t x, int16_t y).</typeparam>
		/// <param name="x1">The x-coordinate of the starting point of the line.</param>
		/// <param name="y1">The y-coordinate of the starting point of the line.</param>
		/// <param name="x2">The x-coordinate of the ending point of the line.</param>
		/// <param name="y2">The y-coordinate of the ending point of the line.</param>
		/// <param name="pixelShader">Pixel shader that decides the color and whether to draw each pixel.</param>
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

			Rgb8::color_t color{};
			if (x2c == x1c && y2c == y1c)
			{
				// Degenerate line, only draw a single pixel.
				if (pixelShader(color, x1c, y1c))
				{
					Surface.Pixel(color, x1c, y1c);
				}
			}
			else if (y2c == y1c)
			{
				// Horizontal line.
				const int8_t xSign = x1c <= x2c ? 1 : -1;
				int_fast16_t x = x1c;
				do
				{
					if (pixelShader(color, x, y1c))
					{
						Surface.Pixel(color, x, y1c);
					}
					x += xSign;
				} while (x != x2c);
			}
			else if (x2c == x1c)
			{
				// Vertical line.
				const int8_t ySign = y1c <= y2c ? 1 : -1;
				int_fast16_t y = y1c;
				do
				{
					if (pixelShader(color, x1c, y))
					{
						Surface.Pixel(color, x1c, y);
					}
					y += ySign;
				} while (y != y2c);
			}
			else
			{
				// Diagonal line. Unroll to appropriate Bresenham line rasterizer.
				BresenhamLineShade(color, x1c, y1c, x2c, y2c, pixelShader);
			}
		}

		/// <summary>
		/// Rasterizes (fills) a triangle using a custom pixel shader, handling window clipping and degenerate cases.
		/// The pixel shader is invoked for each pixel within the triangle, allowing for per-pixel color and visibility control.
		/// </summary>
		/// <typeparam name="PixelShader">Callable signature: bool(Rgb8::color_t&amp; color, int16_t x, int16_t y).</typeparam>
		/// <param name="x1">The x-coordinate of the first vertex of the triangle.</param>
		/// <param name="y1">The y-coordinate of the first vertex of the triangle.</param>
		/// <param name="x2">The x-coordinate of the second vertex of the triangle.</param>
		/// <param name="y2">The y-coordinate of the second vertex of the triangle.</param>
		/// <param name="x3">The x-coordinate of the third vertex of the triangle.</param>
		/// <param name="y3">The y-coordinate of the third vertex of the triangle.</param>
		/// <param name="pixelShader">A callable object or function that determines the color and visibility of each pixel within the triangle.</param>
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

				// Use Sutherland-Hodgman clipping like in DrawTriangle
				clipInputTriangle[0] = { ax, ay };
				clipInputTriangle[1] = { bx, by };
				clipInputTriangle[2] = { cx, cy };
				const uint8_t outCount = ClipTriangleToWindow();

				if (outCount == 0)
				{
					return; // Fully outside after clipping
				}
				else if (outCount == 1)
				{
					// Collapsed to a point
					Rgb8::color_t color{};
					if (IsInsideWindow(clippedPolygon[0].x, clippedPolygon[0].y) &&
						pixelShader(color, clippedPolygon[0].x, clippedPolygon[0].y))
					{
						Surface.Pixel(color, clippedPolygon[0].x, clippedPolygon[0].y);
					}
					return;
				}
				else if (outCount == 2)
				{
					// Collapsed to a line
					if (clippedPolygon[0].x == clippedPolygon[1].x && clippedPolygon[0].y == clippedPolygon[1].y)
					{
						Rgb8::color_t color{};
						if (IsInsideWindow(clippedPolygon[0].x, clippedPolygon[0].y) &&
							pixelShader(color, clippedPolygon[0].x, clippedPolygon[0].y))
						{
							Surface.Pixel(color, clippedPolygon[0].x, clippedPolygon[0].y);
						}
					}
					else
					{
						RasterLine(clippedPolygon[0].x, clippedPolygon[0].y,
							clippedPolygon[1].x, clippedPolygon[1].y, pixelShader);
					}
					return;
				}

				// General convex polygon (up to 6 vertices) -> triangulate as a fan
				p0 = clippedPolygon[0];
				int32_t area2 = 0;
				for (uint8_t i = 1; i + 1 < outCount; ++i)
				{
					p1 = clippedPolygon[i];
					p2 = clippedPolygon[i + 1];

					// Skip degenerate triangles
					area2 = int32_t(p1.x - p0.x) * int32_t(p2.y - p0.y) - int32_t(p2.x - p0.x) * int32_t(p1.y - p0.y);
					if (area2 != 0)
					{
						TriangleYRaster(p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, pixelShader);
					}
				}
			}
			else  // insideCount == 0
			{
				// All vertices are outside the window.
				// Check if triangle intersects the window
				// Quick rejection test - check if all vertices are on the same side of the window
				uint8_t out1 = 0, out2 = 0, out3 = 0;
				if (x1 < 0) out1 |= 1; else if (x1 >= SurfaceWidth) out1 |= 2;
				if (y1 < 0) out1 |= 4; else if (y1 >= SurfaceHeight) out1 |= 8;
				if (x2 < 0) out2 |= 1; else if (x2 >= SurfaceWidth) out2 |= 2;
				if (y2 < 0) out2 |= 4; else if (y2 >= SurfaceHeight) out2 |= 8;
				if (x3 < 0) out3 |= 1; else if (x3 >= SurfaceWidth) out3 |= 2;
				if (y3 < 0) out3 |= 4; else if (y3 >= SurfaceHeight) out3 |= 8;

				// If all vertices are on the same side, the triangle doesn't intersect the window
				if ((out1 & out2 & out3) != 0)
					return;

				// Complex case: clip triangle against window
				// For this implementation, we'll handle edge cases by clipping lines
				// and rendering the resulting segments
				RasterLine(x1, y1, x2, y2, pixelShader);
				RasterLine(x2, y2, x3, y3, pixelShader);
				RasterLine(x3, y3, x1, y1, pixelShader);
			}
		}

		/// <summary>
		/// Renders a rectangle on the framebuffer using a custom pixel shader, handling clipping to the surface boundaries and degenerate cases (lines or points).
		/// </summary>
		/// <typeparam name="PixelShader">Callable signature: bool(Rgb8::color_t&amp; color, int16_t x, int16_t y).</typeparam>
		/// <param name="x1">The x-coordinate of the first corner of the rectangle.</param>
		/// <param name="y1">The y-coordinate of the first corner of the rectangle.</param>
		/// <param name="x2">The x-coordinate of the opposite corner of the rectangle.</param>
		/// <param name="y2">The y-coordinate of the opposite corner of the rectangle.</param>
		/// <param name="pixelShader">Pixel shader invoked for each pixel within the rasterized area.</param>
		template<typename PixelShader>
		void RasterRectangle(const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2, PixelShader&& pixelShader)
		{
			uint8_t inCount = IsInsideWindow(x1, y1);
			inCount += IsInsideWindow(x1, y2);
			inCount += IsInsideWindow(x2, y1);
			inCount += IsInsideWindow(x2, y2);

			if (inCount > 0)
			{
				Rgb8::color_t color{};
				const int16_t x1c = LimitValue(x1, int16_t(0), int16_t(SurfaceWidth - 1));
				const int16_t x2c = LimitValue(x2, int16_t(0), int16_t(SurfaceWidth - 1));
				const int16_t y1c = LimitValue(y1, int16_t(0), int16_t(SurfaceHeight - 1));
				const int16_t y2c = LimitValue(y2, int16_t(0), int16_t(SurfaceHeight - 1));

				if (x1c == x2c)
				{
					if (y1c == y2c)
					{
						// Degenerate rectangle, only draw a single pixel.
						if (pixelShader(color, x1c, y1c))
						{
							Surface.Pixel(color, x1c, y1c);
						}
					}
					else
					{
						// Degenerate rectangle, only draw a vertical line.
						const int8_t ySign = y1c <= y2c ? 1 : -1;
						int_fast16_t y = y1c;
						do
						{
							if (pixelShader(color, x1c, y))
							{
								Surface.Pixel(color, x1c, y);
							}
							y += ySign;
						} while (y != y2c);
					}
				}
				else if (y1c == y2c)
				{
					// Degenerate rectangle, only draw a horizontal line.
					const int8_t xSign = x1c <= x2c ? 1 : -1;
					int_fast16_t x = x1c;
					do
					{
						if (pixelShader(color, x, y1c))
						{
							Surface.Pixel(color, x, y1c);
						}
						x += xSign;
					} while (x != x2c);
				}
				else
				{
					// Cropped rectangle.
					const int8_t xSign = x1c <= x2c ? 1 : -1;
					const int8_t ySign = y1c <= y2c ? 1 : -1;
					int_fast16_t x = x1c;
					int_fast16_t y = y1c;
					do
					{
						x = x1c;
						do
						{
							if (pixelShader(color, x, y))
							{
								Surface.Pixel(color, x, y);
							}
							x += xSign;
						} while (x != x2c);
						y += ySign;
					} while (y != y2c);
				}
			}
			else
			{
				// Whole rectangle is out of bounds.
			}
		}

	protected:
		// Diagonal (Bresenham) Line implementation based on https://www.geeksforgeeks.org/bresenhams-line-generation-algorithm/
		template<typename PixelShader>
		void BresenhamLineShade(Rgb8::color_t& color, int16_t x1, int16_t y1,
			int16_t x2, int16_t y2,
			PixelShader&& pixelShader)
		{
			const int16_t dx = AbsValue(x2 - x1);
			const int8_t sx = x1 < x2 ? 1 : -1;
			const int16_t dy = -AbsValue(y2 - y1);
			const int8_t sy = y1 < y2 ? 1 : -1;
			int16_t err = dx + dy;

			while (true)
			{
				if (pixelShader(color, x1, y1))
				{
					Surface.Pixel(color, x1, y1);
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
			// First check for completely degenerate triangles (all y values equal)
			if (y1 == y2 && y2 == y3)
			{
				// Handle as a horizontal line or point
				Rgb8::color_t color{};

				// Check if it's a single point or a horizontal line
				if (x1 == x2 && x2 == x3)
				{
					// Single point
					if (pixelShader(color, x1, y1))
					{
						Surface.Pixel(color, x1, y1);
					}
				}
				else
				{
					// Find leftmost and rightmost x
					int16_t xMin = min(min(x1, x2), x3);
					int16_t xMax = max(max(x1, x2), x3);

					// Draw horizontal line
					for (int16_t x = xMin; x <= xMax; x++)
					{
						if (pixelShader(color, x, y1))
						{
							Surface.Pixel(color, x, y1);
						}
					}
				}
				return;
			}

			// Original code for non-degenerate cases
			if (y2 == y3) // Flat bottom.
			{
				BresenhamFlatBottomFill(x1, y1, x2, y2, x3, y3, pixelShader);
			}
			else if (y1 == y2) // Flat top.
			{
				BresenhamFlatTopFill(x1, y1, x2, y2, x3, y3, pixelShader);
			}
			else if (y3 == y1) // Degenerate triangle.
			{
				Rgb8::color_t color{};
				if (x1 == x3)
				{
					// Degenerate triangle, point only.
					if (pixelShader(color, x1, y1))
					{
						Surface.Pixel(color, x1, y1);
					}
				}
				else
				{
					// Degenerate triangle, raster horizontal line only.
					const int8_t xSign = x1 <= x3 ? 1 : -1;
					int_fast16_t x = x1;
					do
					{
						if (pixelShader(color, x, y1))
						{
							Surface.Pixel(color, x, y1);
						}
						x += xSign;
					} while (x != x3);
				}
			}
			else // General triangle: split it.
			{
				// Calculate splitting vertex Vi.
				const int16_t dxTotal = x3 - x1;
				const int16_t dyTotal = y3 - y1;
				const int16_t dySegment = y2 - y1;

				// Calculate Vi_x in fixed-point.
				const int16_t Vi_x = SignedRightShift(
					(SignedLeftShift<int32_t>(x1, BRESENHAM_SCALE))
					+ ((SignedLeftShift<int32_t>(dxTotal, BRESENHAM_SCALE) * dySegment) / dyTotal)
					, BRESENHAM_SCALE);

				// Draw the two sub-triangles
				BresenhamFlatBottomFill(x1, y1, x2, y2, Vi_x, y2, pixelShader);
				BresenhamFlatTopFill(x2, y2, Vi_x, y2, x3, y3, pixelShader);
			}
		}

	private:
		// Optimized flat-bottom triangle fill
		template<typename PixelShader>
		void BresenhamFlatBottomFill(const int16_t x1, const int16_t y1,
			const int16_t x2, const int16_t y2,
			const int16_t x3, const int16_t y3,
			PixelShader&& pixelShader)
		{
			// Pre-calculate slopes in 16.16 fixed point format
			const int32_t dx1 = SignedLeftShift<int32_t>(x2 - x1, BRESENHAM_SCALE) / (y2 - y1);
			const int32_t dx2 = SignedLeftShift<int32_t>(x3 - x1, BRESENHAM_SCALE) / (y3 - y1);

			// Initialize scanline starting x-coordinates in fixed point
			int32_t sx1 = SignedLeftShift<int32_t>(x1, BRESENHAM_SCALE);
			int32_t sx2 = sx1;

			Rgb8::color_t color{};

			// Scan all lines from top to bottom
			for (int16_t y = y1; y <= y2; y++)
			{
				int16_t startX = SignedRightShift(sx1 + 0x8000, BRESENHAM_SCALE); // round to nearest pixel
				int16_t endX = SignedRightShift(sx2 + 0x8000, BRESENHAM_SCALE);   // round to nearest pixel

				// Ensure left-to-right drawing order
				if (startX > endX)
				{
					int16_t temp = startX;
					startX = endX;
					endX = temp;
				}

				// Draw the scanline
				for (int16_t x = startX; x <= endX; x++)
				{
					if (pixelShader(color, x, y))
					{
						Surface.Pixel(color, x, y);
					}
				}

				// Update edge x-coordinates for the next scanline
				sx1 += dx1;
				sx2 += dx2;
			}
		}

		// Optimized flat-top triangle fill
		template<typename PixelShader>
		void BresenhamFlatTopFill(const int16_t x1, const int16_t y1,
			const int16_t x2, const int16_t y2,
			const int16_t x3, const int16_t y3,
			PixelShader&& pixelShader)
		{
			// Pre-calculate slopes in 16.16 fixed point format
			const int32_t dx1 = SignedLeftShift<int32_t>(x3 - x1, BRESENHAM_SCALE) / (y3 - y1);
			const int32_t dx2 = SignedLeftShift<int32_t>(x3 - x2, BRESENHAM_SCALE) / (y3 - y2);

			// Initialize scanline starting x-coordinates in fixed point
			int32_t sx1 = SignedLeftShift<int32_t>(x3, BRESENHAM_SCALE);
			int32_t sx2 = sx1;

			Rgb8::color_t color{};

			// Scan all lines from bottom to top
			for (int16_t y = y3; y >= y1; y--)
			{
				int16_t startX = SignedRightShift(sx1 + 0x8000, BRESENHAM_SCALE); // round to nearest pixel
				int16_t endX = SignedRightShift(sx2 + 0x8000, BRESENHAM_SCALE);   // round to nearest pixel

				// Ensure left-to-right drawing order
				if (startX > endX)
				{
					int16_t temp = startX;
					startX = endX;
					endX = temp;
				}

				// Draw the scanline
				for (int16_t x = startX; x <= endX; x++)
				{
					if (pixelShader(color, x, y))
					{
						Surface.Pixel(color, x, y);
					}
				}

				// Update edge x-coordinates for the next scanline
				sx1 -= dx1;
				sx2 -= dx2;
			}
		}
	};
}
#endif