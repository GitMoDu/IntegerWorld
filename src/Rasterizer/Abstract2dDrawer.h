#ifndef _INTEGER_WORLD_ABSTRACT_2D_DRAWER_h
#define _INTEGER_WORLD_ABSTRACT_2D_DRAWER_h

#include "AbstractSurfaceRasterizer.h"

namespace IntegerWorld
{
	/// <summary>
	/// High-level 2D drawing API built on top of AbstractSurfaceRasterizer.
	/// - Adds convenience Draw/Fill routines with window clipping.
	/// - Provides optional per-pixel blending helpers.
	/// </summary>
	/// <typeparam name="SurfaceType">The output surface type (IOutputSurface-like API).</typeparam>
	template<typename SurfaceType>
	class Abstract2dDrawer : public AbstractSurfaceRasterizer<SurfaceType>
	{
	public:
		Abstract2dDrawer(SurfaceType& surface)
			: AbstractSurfaceRasterizer<SurfaceType>(surface)
		{
		}

	public:
		/// <summary>
		/// Fills the entire drawing surface with the specified color.
		/// </summary>
		/// <param name="color">The color to use for filling surface</param>
		void Fill(const Rgb8::color_t color)
		{
			Surface.RectangleFill(color, 0, 0, SurfaceWidth - 1, SurfaceHeight - 1);
		}

		/// <summary>
		/// Blends a pixel at (x, y) using the specified blending mode, if inside the window.
		/// </summary>
		/// <param name="color">Source color.</param>
		/// <param name="x">X coordinate.</param>
		/// <param name="y">Y coordinate.</param>
		/// <param name="blendMode">Blending mode (Alpha, Add, Subtract, Multiply, Screen).</param>
		void BlendPixel(const Rgb8::color_t color, const int16_t x, const int16_t y, pixel_blend_mode_t blendMode)
		{
			if (IsInsideWindow(x, y))
			{
				switch (blendMode)
				{
				case pixel_blend_mode_t::Alpha:
					Surface.PixelBlendAlpha(color, x, y);
					break;
				case pixel_blend_mode_t::Add:
					Surface.PixelBlendAdd(color, x, y);
					break;
				case pixel_blend_mode_t::Subtract:
					Surface.PixelBlendSubtract(color, x, y);
					break;
				case pixel_blend_mode_t::Multiply:
					Surface.PixelBlendMultiply(color, x, y);
					break;
				case pixel_blend_mode_t::Screen:
					Surface.PixelBlendScreen(color, x, y);
					break;
				default:
					break;
				}
			}
		}

		/// <summary>
		/// Templated variant that inlines the blend mode at compile time.
		/// </summary>
		/// <typeparam name="blendMode">Blending mode (Alpha, Add, Subtract, Multiply, Screen).</typeparam>
		/// <param name="color">Source color.</param>
		/// <param name="x">X coordinate.</param>
		/// <param name="y">Y coordinate.</param>
		template<pixel_blend_mode_t blendMode = pixel_blend_mode_t::Alpha>
		void BlendPixel(const Rgb8::color_t color, const int16_t x, const int16_t y)
		{
			if (IsInsideWindow(x, y))
			{
				switch (blendMode)
				{
				case pixel_blend_mode_t::Alpha:
					Surface.PixelBlendAlpha(color, x, y);
					break;
				case pixel_blend_mode_t::Add:
					Surface.PixelBlendAdd(color, x, y);
					break;
				case pixel_blend_mode_t::Subtract:
					Surface.PixelBlendSubtract(color, x, y);
					break;
				case pixel_blend_mode_t::Multiply:
					Surface.PixelBlendMultiply(color, x, y);
					break;
				case pixel_blend_mode_t::Screen:
					Surface.PixelBlendScreen(color, x, y);
					break;
				default:
					break;
				}
			}
		}

		/// <summary>
		/// Draws a point at the specified coordinates with the given color, if the point is inside the window.
		/// </summary>
		/// <param name="color">The color to use for the point, represented as a Rgb8::color_t value.</param>
		/// <param name="x">The x-coordinate of the point.</param>
		/// <param name="y">The y-coordinate of the point.</param>
		void DrawPixel(const Rgb8::color_t color, const int16_t x, const int16_t y)
		{
			if (IsInsideWindow(x, y))
			{
				Surface.Pixel(color, x, y);
			}
		}

		/// <summary>
		/// Draws a line between two points, clipping the line to the window boundaries if necessary.
		/// </summary>
		/// <param name="color">The color to use for the line, specified as a Rgb8::color_t value.</param>
		/// <param name="x1">The x-coordinate of the starting point of the line.</param>
		/// <param name="y1">The y-coordinate of the starting point of the line.</param>
		/// <param name="x2">The x-coordinate of the ending point of the line.</param>
		/// <param name="y2">The y-coordinate of the ending point of the line.</param>
		void DrawLine(const Rgb8::color_t color, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2)
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
				Surface.Pixel(color, x1c, y1c);
			}
			else
			{
				Surface.Line(color, x1c, y1c, x2c, y2c);
			}
		}

		/// <summary>
		/// Draws a filled triangle with the specified color and vertex coordinates, clipping it to the window boundaries if necessary.
		/// </summary>
		/// <param name="color">The color to fill the triangle, specified as a Rgb8::color_t value.</param>
		/// <param name="x1">The x-coordinate of the first vertex of the triangle.</param>
		/// <param name="y1">The y-coordinate of the first vertex of the triangle.</param>
		/// <param name="x2">The x-coordinate of the second vertex of the triangle.</param>
		/// <param name="y2">The y-coordinate of the second vertex of the triangle.</param>
		/// <param name="x3">The x-coordinate of the third vertex of the triangle.</param>
		/// <param name="y3">The y-coordinate of the third vertex of the triangle.</param>
		void DrawTriangle(const Rgb8::color_t color, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2, const int16_t x3, const int16_t y3)
		{
			// Count how many vertices are inside the window
			const bool in1 = IsInsideWindow(x1, y1);
			const bool in2 = IsInsideWindow(x2, y2);
			const bool in3 = IsInsideWindow(x3, y3);

			const uint8_t insideCount = (uint8_t)in1 + (uint8_t)in2 + (uint8_t)in3;
			if (insideCount == 3)
			{
				// The whole triangle fits in the window.
				Surface.TriangleFill(color,
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
				Surface.TriangleFill(
					color,
					ax, ay,
					bx, by,
					pcx2, pcy2
				);
				Surface.TriangleFill(
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
				Surface.TriangleFill(color,
					ax, ay,
					pbx, pby,
					pcx, pcy);
			}
			else if (TriangleCoversWindow(x1, y1, x2, y2, x3, y3)) //  insideCount == 0.
			{
				// The triangle covers the whole window.
				Fill(color);
			}
			else  // insideCount == 0, !TriangleCoversWindow
			{
				// All vertices are outside the window.
				//TODO: Check if triangle intersects the window and draw remaining are with polygon sub-division.
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
		void DrawRectangle(const Rgb8::color_t color, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2)
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
						Surface.Pixel(color, x1c, y1c);
					}
					else
					{
						// Degenerate rectangle, only draw a line.
						Surface.Line(color, x1c, y1c, x1c, y2c);
					}
				}
				else if (y1c == y2c)
				{
					// Degenerate rectangle, only draw a line.
					Surface.Line(color, x1c, y1c, x2c, y1c);
				}
				else
				{
					// Cropped rectangle.
					Surface.RectangleFill(color, x1c, y1c, x2c, y2c);
				}
			}
			else
			{
				// Whole rectangle is out of bounds.
			}
		}

	protected:
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
			else if (x1 >= (SurfaceWidth - 1) && dx != 0)
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
			else if (y1 >= (SurfaceHeight - 1) && dy != 0)
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

		template<typename PixelShader>
		void BresenhamFlatTopFill(const int16_t x1, const int16_t y1,
			const int16_t x2, const int16_t y2,
			const int16_t x3, const int16_t y3,
			PixelShader&& pixelShader)
		{
			Rgb8::color_t color{};

			if (y1 == y3)
			{
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
			else
			{
				// Calculate inverse slopes in fixed-point
				const int32_t invSlope2 = (int32_t(x3 - x1) << BRESENHAM_SCALE) / (y3 - y1);
				const int32_t invSlope1 = (int32_t(x3 - x2) << BRESENHAM_SCALE) / (y3 - y2);

				// Starting x positions in fixed-point
				int32_t ax = int32_t(x3) << BRESENHAM_SCALE;
				int32_t bx = ax;

				// Loop variables.
				int16_t xEnd{};
				const int16_t yEnd = y1 - 1;
				int_fast16_t x{};
				int_fast16_t y = y3;
				do
				{
					x = ax >> BRESENHAM_SCALE;
					xEnd = bx >> BRESENHAM_SCALE;
					const int8_t xSign = x <= xEnd ? 1 : -1;
					xEnd += xSign;
					do
					{
						if (pixelShader(color, x, y))
						{
							Surface.Pixel(color, x, y);
						}
						x += xSign;
					} while (x != xEnd);

					ax -= invSlope1;
					bx -= invSlope2;
					y--;
				} while (y != yEnd);
			}
		}

		template<typename PixelShader>
		void BresenhamFlatBottomFill(const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2, const int16_t x3, const int16_t y3, PixelShader&& pixelShader)
		{
			Rgb8::color_t color{};

			if (y1 == y2)
			{
				if (x1 == x2)
				{
					// Degenerate triangle, point only.
					if (pixelShader(color, x2, y2))
					{
						Surface.Pixel(color, x2, y2);
					}
				}
				else
				{
					// Degenerate triangle, raster horizontal line only.
					const int8_t xSign = x1 <= x2 ? 1 : -1;
					int_fast16_t x = x1;
					do
					{
						if (pixelShader(color, x, y2))
						{
							Surface.Pixel(color, x, y2);
						}
						x += xSign;
					} while (x != x2);
				}
			}
			else
			{
				// Calculate inverse slopes in fixed-point
				const int32_t invSlope2 = (int32_t(x3 - x1) << BRESENHAM_SCALE) / (y3 - y1);
				const int32_t invSlope1 = (int32_t(x2 - x1) << BRESENHAM_SCALE) / (y2 - y1);

				// Starting x positions in fixed-point
				int32_t ax = int32_t(x1) << BRESENHAM_SCALE;
				int32_t bx = ax;

				// Loop variables.
				int16_t xEnd{};
				int_fast16_t x{};
				int_fast16_t y = y1;
				do
				{
					x = ax >> BRESENHAM_SCALE;
					xEnd = bx >> BRESENHAM_SCALE;
					const int8_t xSign = x <= xEnd ? 1 : -1;
					xEnd += xSign;
					do
					{
						if (pixelShader(color, x, y))
						{
							Surface.Pixel(color, x, y);
						}
						x += xSign;
					} while (x != xEnd);

					ax += invSlope1;
					bx += invSlope2;
					y++;
				} while (y != y2);
			}
		}
	};
}
#endif