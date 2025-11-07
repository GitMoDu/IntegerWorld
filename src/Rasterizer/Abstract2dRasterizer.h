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
		using Abstract2dDrawer<SurfaceType>::FP_ROUND_HALF;

	protected:
		using Abstract2dDrawer<SurfaceType>::Surface;
		using Abstract2dDrawer<SurfaceType>::SurfaceWidth;
		using Abstract2dDrawer<SurfaceType>::SurfaceHeight;
		using Abstract2dDrawer<SurfaceType>::IsInsideWindow;

		using Abstract2dDrawer<SurfaceType>::ClipEndpointToWindow;
		using Abstract2dDrawer<SurfaceType>::ClipTriangleToWindow;
		using Abstract2dDrawer<SurfaceType>::FixedRoundToInt;
		using Abstract2dDrawer<SurfaceType>::IntToFixed;

	protected:
		using Abstract2dDrawer<SurfaceType>::clippedPolygon;
		using Abstract2dDrawer<SurfaceType>::clipScratchA;
		using Abstract2dDrawer<SurfaceType>::clipScratchB;
		using Abstract2dDrawer<SurfaceType>::p0;
		using Abstract2dDrawer<SurfaceType>::p1;
		using Abstract2dDrawer<SurfaceType>::p2;

	private:
		// 2 edge walkers for triangle rasterization.
		TriangleRasterHelper::EdgeWalker WalkerLeft{}, WalkerRight{};

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
			if (IsInsideWindow(x1, y1) && IsInsideWindow(x2, y2) && IsInsideWindow(x3, y3))
			{
				// Whole triangle is inside the window.
				TriangleYRaster(x1, y1, x2, y2, x3, y3, pixelShader);
			}
			else
			{
				// Pass the triangle through the clippedPolygon.
				clippedPolygon[0] = { x1, y1 };
				clippedPolygon[1] = { x2, y2 };
				clippedPolygon[2] = { x3, y3 };

				// Clip the triangle against the window (Sutherland–Hodgman with integer arithmetic).
				const uint8_t clippedVertexCount = ClipTriangleToWindow();
				if (clippedVertexCount == 0)
				{
					// Fully outside after clipping.
				}
				else if (clippedVertexCount == 1)
				{
					// Degenerate after clipping.
					const int16_t px = clippedPolygon[0].x;
					const int16_t py = clippedPolygon[0].y;
					if (IsInsideWindow(px, py))
					{
						Rgb8::color_t color{};
						if (pixelShader(color, px, py))
							Surface.Pixel(color, px, py);
					}
				}
				else if (clippedVertexCount == 2)
				{
					// Collapsed to a line after clipping.
					RasterLine(clippedPolygon[0].x, clippedPolygon[0].y,
					           clippedPolygon[1].x, clippedPolygon[1].y,
					           pixelShader);
				}
				else
				{
					// Triangulate convex clipped polygon (fan) and fill each triangle with edge functions.
					p0 = clippedPolygon[0];
					for (uint8_t i = 1; i + 1 < clippedVertexCount; i++)
					{
						p1 = clippedPolygon[i];
						p2 = clippedPolygon[i + 1];

						// Triangle fill with pixel shader. Requires mutable vertices.
						TriangleHitRaster(p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, pixelShader);
					}
				}
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
					for (int_fast16_t x = xMin; x <= xMax; x++)
					{
						if (pixelShader(color, x, y1))
						{
							Surface.Pixel(color, x, y1);
						}
					}
				}
			}
			// Original code for non-degenerate cases
			else if (y2 == y3) // Flat bottom.
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
			const int32_t dx1 = IntToFixed(x2 - x1) / (y2 - y1);
			const int32_t dx2 = IntToFixed(x3 - x1) / (y3 - y1);

			int32_t sx1 = IntToFixed(x1);
			int32_t sx2 = sx1;

			Rgb8::color_t color{};

			for (int_fast16_t y = y1; y <= y2; y++)
			{
				// round to nearest pixel
				int16_t startX = FixedRoundToInt(sx1);
				int16_t endX = FixedRoundToInt(sx2);

				if (startX > endX)
				{
					int16_t temp = startX;
					startX = endX;
					endX = temp;
				}

				for (int_fast16_t x = startX; x <= endX; x++)
				{
					if (pixelShader(color, x, y))
					{
						Surface.Pixel(color, x, y);
					}
				}

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
			const int32_t dx1 = IntToFixed(x3 - x1) / (y3 - y1);
			const int32_t dx2 = IntToFixed(x3 - x2) / (y3 - y2);

			int32_t sx1 = IntToFixed(x3);
			int32_t sx2 = sx1;

			Rgb8::color_t color{};

			for (int_fast16_t y = y3; y >= y1; y--)
			{
				// round to nearest pixel
				int16_t startX = FixedRoundToInt(sx1);
				int16_t endX = FixedRoundToInt(sx2);

				if (startX > endX)
				{
					int16_t temp = startX;
					startX = endX;
					endX = temp;
				}

				for (int_fast16_t x = startX; x <= endX; x++)
				{
					if (pixelShader(color, x, y))
					{
						Surface.Pixel(color, x, y);
					}
				}

				sx1 -= dx1;
				sx2 -= dx2;
			}
		}

	private:
		/// <summary>
		/// Rasterizes a filled triangle defined by three 2D integer vertices
		/// and invokes a pixel shader for each covered pixel.
		/// The function sorts vertices by Y, handles flat-top/flat-bottom/degenerate cases,
		/// clamps to the surface bounds, and writes pixels when the shader requests it.
		/// </summary>
		/// <typeparam name="PixelShader">The callable type for the pixel shader. Typical signature: bool(PixelColorType& color, int x, int y) (can be a forwarding reference).</typeparam>
		/// <param name="ax">X coordinate of the first vertex (A).</param>
		/// <param name="ay">Y coordinate of the first vertex (A).</param>
		/// <param name="bx">X coordinate of the second vertex (B).</param>
		/// <param name="by">Y coordinate of the second vertex (B).</param>
		/// <param name="cx">X coordinate of the third vertex (C).</param>
		/// <param name="cy">Y coordinate of the third vertex (C).</param>
		/// <param name="pixelShader">A callable invoked for each candidate pixel. It is called as pixelShader(color, x, y) where 'color' is an Rgb8::color_t provided to be modified by the shader and (x,y) are the pixel coordinates. The callable returns a bool: true to write the pixel to the Surface, false to skip writing.</param>
		template<typename PixelShader>
		void TriangleHitRaster(int16_t ax, int16_t ay,
			int16_t bx, int16_t by,
			int16_t cx, int16_t cy,
			PixelShader&& pixelShader)
		{
			// Sort by y (A top, C bottom)
			if (ay > by) { int16_t t = ax; ax = bx; bx = t; t = ay; ay = by; by = t; }
			if (ay > cy) { int16_t t = ax; ax = cx; cx = t; int16_t tt = ay; ay = cy; cy = tt; }
			if (by > cy) { int16_t t = bx; bx = cx; cx = t; int16_t tt = by; by = cy; cy = tt; }

			Rgb8::color_t color{};

			// Degenerate full-flat (all y equal)
			if (ay == cy)
			{
				int16_t xMin = ax, xMax = ax;
				if (bx < xMin) xMin = bx; if (bx > xMax) xMax = bx;
				if (cx < xMin) xMin = cx; if (cx > xMax) xMax = cx;
				if (ay >= 0 && ay < SurfaceHeight)
				{
					if (xMin < 0) xMin = 0;
					if (xMax >= SurfaceWidth) xMax = int16_t(SurfaceWidth - 1);
					for (int_fast16_t x = xMin; x <= xMax; ++x)
						if (pixelShader(color, x, ay)) Surface.Pixel(color, x, ay);
				}
				return;
			}

			const bool flatBottom = (by == cy); // two bottom verts share y
			const bool flatTop = (ay == by); // two top verts share y

			// Use two persistent walkers
			// Walker API: Init(x0,y0,x1,y1) with y0 <= y1, Step() advances one scanline (y+1) updating x by Bresenham-like error.
			if (flatBottom)
			{
				// A is top, B & C form flat bottom at y=by(=cy). Fill ay..by inclusive.
				WalkerLeft.Init(ax, ay, bx, by);
				WalkerRight.Init(ax, ay, cx, cy);

				int16_t yStart = ay < 0 ? 0 : ay;
				while (WalkerLeft.y < yStart && WalkerLeft.y < WalkerLeft.yEnd) WalkerLeft.Step();
				while (WalkerRight.y < yStart && WalkerRight.y < WalkerRight.yEnd) WalkerRight.Step();

				for (int_fast16_t y = yStart; y <= by && y < SurfaceHeight; ++y)
				{
					if (y >= 0)
					{
						int16_t xL = WalkerLeft.x;
						int16_t xR = WalkerRight.x;
						if (xL > xR) { int16_t t = xL; xL = xR; xR = t; }
						if (xL < 0) xL = 0;
						if (xR >= SurfaceWidth) xR = int16_t(SurfaceWidth - 1);
						for (int_fast16_t x = xL; x <= xR; ++x)
							if (pixelShader(color, x, y)) Surface.Pixel(color, x, y);
					}
					WalkerLeft.Step();
					WalkerRight.Step();
				}
			}
			else if (flatTop)
			{
				// A & B form flat top at y=ay(=by); C is bottom.
				WalkerLeft.Init(ax, ay, cx, cy);
				WalkerRight.Init(bx, by, cx, cy);

				// We fill from ay .. cy inclusive.
				int16_t yStart = ay < 0 ? 0 : ay;
				while (WalkerLeft.y < yStart && WalkerLeft.y < WalkerLeft.yEnd) WalkerLeft.Step();
				while (WalkerRight.y < yStart && WalkerRight.y < WalkerRight.yEnd) WalkerRight.Step();

				for (int_fast16_t y = yStart; y <= cy && y < SurfaceHeight; ++y)
				{
					if (y >= 0)
					{
						int16_t xL = WalkerLeft.x;
						int16_t xR = WalkerRight.x;
						if (xL > xR) { int16_t t = xL; xL = xR; xR = t; }
						if (xL < 0) xL = 0;
						if (xR >= SurfaceWidth) xR = int16_t(SurfaceWidth - 1);
						for (int_fast16_t x = xL; x <= xR; ++x)
							if (pixelShader(color, x, y)) Surface.Pixel(color, x, y);
					}
					WalkerLeft.Step();
					WalkerRight.Step();
				}
			}
			else
			{

				// General triangle: A (top), B (middle), C (bottom)
				WalkerLeft.Init(ax, ay, bx, by);
				WalkerRight.Init(ax, ay, cx, cy);

				// Top half: y in [ay, by) (exclusive of middle to avoid double)
				int16_t yStart = ay < 0 ? 0 : ay;
				while (WalkerLeft.y < yStart && WalkerLeft.y < WalkerLeft.yEnd) WalkerLeft.Step();
				while (WalkerRight.y < yStart && WalkerRight.y < WalkerRight.yEnd) WalkerRight.Step();

				for (int_fast16_t y = yStart; y < by && y < SurfaceHeight; ++y)
				{
					if (y >= 0)
					{
						int16_t xL = WalkerLeft.x;
						int16_t xR = WalkerRight.x;
						if (xL > xR) { int16_t t = xL; xL = xR; xR = t; }
						if (xL < 0) xL = 0;
						if (xR >= SurfaceWidth) xR = int16_t(SurfaceWidth - 1);
						for (int_fast16_t x = xL; x <= xR; ++x)
							if (pixelShader(color, x, y)) Surface.Pixel(color, x, y);
					}
					WalkerLeft.Step();
					WalkerRight.Step();
				}

				// Bottom half: rebuild left edge from B->C (right keeps A->C), now y in [by, cy] inclusive.
				WalkerLeft.Init(bx, by, cx, cy);
				while (WalkerLeft.y < by) WalkerLeft.Step();
				while (WalkerRight.y < by) WalkerRight.Step();

				for (int_fast16_t y = by; y <= cy && y < SurfaceHeight; ++y)
				{
					if (y >= 0)
					{
						int16_t xL = WalkerLeft.x;
						int16_t xR = WalkerRight.x;
						if (xL > xR) { int16_t t = xL; xL = xR; xR = t; }
						if (xL < 0) xL = 0;
						if (xR >= SurfaceWidth) xR = int16_t(SurfaceWidth - 1);
						for (int_fast16_t x = xL; x <= xR; ++x)
							if (pixelShader(color, x, y)) Surface.Pixel(color, x, y);
					}
					WalkerLeft.Step();
					WalkerRight.Step();
				}
			}
		}
	};
}
#endif