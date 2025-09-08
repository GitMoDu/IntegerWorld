#ifndef _INTEGER_WORLD_ABSTRACT_2D_DRAWER_h
#define _INTEGER_WORLD_ABSTRACT_2D_DRAWER_h

#include "AbstractSurfaceRasterizer.h"
#include "TriangleRasterHelper.h"

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
	protected:
		using AbstractSurfaceRasterizer<SurfaceType>::Surface;
		using AbstractSurfaceRasterizer<SurfaceType>::SurfaceWidth;
		using AbstractSurfaceRasterizer<SurfaceType>::SurfaceHeight;
		using AbstractSurfaceRasterizer<SurfaceType>::IsInsideWindow;

	protected:
		using point2d_t = TriangleRasterHelper::point2d_t;
		using ClipEdgeEnum = TriangleRasterHelper::ClipEdgeEnum;

	protected:
		// Temporary buffers for Sutherland–Hodgman clipping (triangle in, convex polygon out).
		point2d_t clipInputTriangle[3];
		point2d_t clippedPolygon[8];

		// Resusable small buffers for edge ping-pong. Max 6 vertices after clipping a triangle to a rect.
		point2d_t clipScratchA[8];
		point2d_t clipScratchB[8];

		// Reusable points for clipping calculations.
		point2d_t p0, p1, p2;

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
		/// Draws a filled triangle with robust clipping to the window.
		/// Handles all cases, including when all vertices are outside but the triangle intersects the window.
		/// </summary>
		/// <param name="color">Fill color.</param>
		/// <param name="x1">X of first vertex.</param>
		/// <param name="y1">Y of first vertex.</param>
		/// <param name="x2">X of second vertex.</param>
		/// <param name="y2">Y of second vertex.</param>
		/// <param name="x3">X of third vertex.</param>
		/// <param name="y3">Y of third vertex.</param>
		void DrawTriangle(const Rgb8::color_t color, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2, const int16_t x3, const int16_t y3)
		{
			// Fast path: fully inside
			if (IsInsideWindow(x1, y1) && IsInsideWindow(x2, y2) && IsInsideWindow(x3, y3))
			{
				Surface.TriangleFill(color, x1, y1, x2, y2, x3, y3);
				return;
			}

			// Clip the triangle against the window (Sutherland–Hodgman with integer arithmetic).
			clipInputTriangle[0] = { x1, y1 };
			clipInputTriangle[1] = { x2, y2 };
			clipInputTriangle[2] = { x3, y3 };
			const uint8_t outCount = ClipTriangleToWindow();

			if (outCount == 0)
			{
				return; // Fully outside
			}
			else if (outCount == 1)
			{
				// Collapsed to a point
				if (IsInsideWindow(clippedPolygon[0].x, clippedPolygon[0].y))
					Surface.Pixel(color, clippedPolygon[0].x, clippedPolygon[0].y);
				return;
			}
			else if (outCount == 2)
			{
				// Collapsed to a line
				if (clippedPolygon[0].x == clippedPolygon[1].x && clippedPolygon[0].y == clippedPolygon[1].y)
				{
					if (IsInsideWindow(clippedPolygon[0].x, clippedPolygon[0].y))
						Surface.Pixel(color, clippedPolygon[0].x, clippedPolygon[0].y);
				}
				else
				{
					Surface.Line(color, clippedPolygon[0].x, clippedPolygon[0].y, clippedPolygon[1].x, clippedPolygon[1].y);
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
					Surface.TriangleFill(color, p0.x, p0.y, p1.x, p1.y, p2.x, p2.y);
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
		/// Determines if the triangle defined by (x1, y1), (x2, y2), and (x3, y3) completely covers the current drawing window.
		/// </summary>
		/// <returns>True if the triangle fully contains all four corners of the window; otherwise, false.</returns>
		bool TriangleCoversWindow(
			const int16_t x1, const int16_t y1,
			const int16_t x2, const int16_t y2,
			const int16_t x3, const int16_t y3) const
		{
			return TriangleRasterHelper::PointInTriangle(0, 0, x1, y1, x2, y2, x3, y3) &&
				TriangleRasterHelper::PointInTriangle(SurfaceWidth - 1, 0, x1, y1, x2, y2, x3, y3) &&
				TriangleRasterHelper::PointInTriangle(0, SurfaceHeight - 1, x1, y1, x2, y2, x3, y3) &&
				TriangleRasterHelper::PointInTriangle(SurfaceWidth - 1, SurfaceHeight - 1, x1, y1, x2, y2, x3, y3);
		}

		/// <summary>
		/// Clips a triangle to the boundaries of the window and outputs the resulting polygon vertices.
		/// </summary>
		/// <returns>The number of vertices in the resulting clipped polygon. Returns 0 if the triangle is completely outside the window.</returns>
		uint8_t ClipTriangleToWindow()
		{
			// Initialize with triangle vertices
			uint8_t countA = 3;
			for (uint8_t i = 0; i < 3; ++i)
			{
				clipScratchA[i] = clipInputTriangle[i];
			}

			// Clip sequentially: Left, Right, Top, Bottom
			uint8_t countB = ClipAgainstEdge(clipScratchA, countA, clipScratchB, ClipEdgeEnum::Left, SurfaceWidth, SurfaceHeight);
			if (countB == 0) return 0;

			countA = ClipAgainstEdge(clipScratchB, countB, clipScratchA, ClipEdgeEnum::Right, SurfaceWidth, SurfaceHeight);
			if (countA == 0) return 0;

			countB = ClipAgainstEdge(clipScratchA, countA, clipScratchB, ClipEdgeEnum::Top, SurfaceWidth, SurfaceHeight);
			if (countB == 0) return 0;

			countA = ClipAgainstEdge(clipScratchB, countB, clipScratchA, ClipEdgeEnum::Bottom, SurfaceWidth, SurfaceHeight);
			if (countA == 0) return 0;

			// Copy to out
			for (uint8_t i = 0; i < countA; ++i)
			{
				clippedPolygon[i] = clipScratchA[i];
			}

			return countA;
		}
	};
}
#endif