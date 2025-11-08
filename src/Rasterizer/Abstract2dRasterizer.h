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
	template<typename SurfaceType>
	class Abstract2dRasterizer : public Abstract2dDrawer<SurfaceType>
	{
	protected:
		using Abstract2dDrawer<SurfaceType>::Surface;
		using Abstract2dDrawer<SurfaceType>::SurfaceWidth;
		using Abstract2dDrawer<SurfaceType>::SurfaceHeight;
		using Abstract2dDrawer<SurfaceType>::IsInsideWindow;
		using Abstract2dDrawer<SurfaceType>::ClipEndpointToWindow;
		using Abstract2dDrawer<SurfaceType>::ClipTriangleToWindow;
		using Abstract2dDrawer<SurfaceType>::FixedRoundToInt;
		using Abstract2dDrawer<SurfaceType>::IntToFixed;
		using Abstract2dDrawer<SurfaceType>::clippedPolygon;

	public:
		Abstract2dRasterizer(SurfaceType& surface)
			: Abstract2dDrawer<SurfaceType>(surface)
		{
		}

	public:
		/// <summary>Rasterize a clipped line with pixel shader.</summary>
		template<typename pixel_shader_t>
		void RasterLine(const int16_t x1, const  int16_t y1, const int16_t x2, const int16_t y2, pixel_shader_t&& pixelShader)
		{
			int16_t x1c(x1), y1c(y1), x2c(x2), y2c(y2);

			const bool in1 = IsInsideWindow(x1, y1);
			const bool in2 = IsInsideWindow(x2, y2);

			if (!in1 || !in2)
			{
				if (in1) // Only endpoint 1 is inside the window.
				{
					ClipEndpointToWindow(x2c, y2c, x1c, y1c);
				}
				else if (in2) // Only endpoint 2 is inside the window.
				{
					ClipEndpointToWindow(x1c, y1c, x2c, y2c);
				}
				else // Both endpoints are outside the window.
				{
					// Trivial reject: both endpoints outside on the same side of the window.
					const uint8_t out1 = (x1 < 0 ? 1 : 0) | (x1 >= SurfaceWidth ? 2 : 0) |
						(y1 < 0 ? 4 : 0) | (y1 >= SurfaceHeight ? 8 : 0);
					const uint8_t out2 = (x2 < 0 ? 1 : 0) | (x2 >= SurfaceWidth ? 2 : 0) |
						(y2 < 0 ? 4 : 0) | (y2 >= SurfaceHeight ? 8 : 0);
					if (out1 & out2)
						return; // Both endpoints are outside on the same side

					// Try to clip remaining endpoints to the window.
					ClipEndpointToWindow(x1c, y1c, x2, y2);
					ClipEndpointToWindow(x2c, y2c, x1, y1);

					// If either endpoint is not inside, or the segment is degenerate, reject.
					if (!IsInsideWindow(x1c, y1c) || !IsInsideWindow(x2c, y2c))
						return; // No visible segment after clipping.
				}
			}

			Rgb8::color_t color{};
			if (x1c == x2c && y1c == y2c) // Degenerate line (point).
			{
				if (pixelShader(color, x1c, y1c))
					Surface.Pixel(color, x1c, y1c);
			}
			else if (y1c == y2c) // Horizontal line.
			{
				const int8_t step = x1c <= x2c ? 1 : -1;
				for (int_fast16_t x = x1c; ; x += step)
				{
					if (pixelShader(color, x, y1c))
						Surface.Pixel(color, x, y1c);
					if (x == x2c)
						break;
				}
			}
			else if (x1c == x2c) // Vertical line.
			{
				const int8_t step = y1c <= y2c ? 1 : -1;
				for (int_fast16_t y = y1c; ; y += step)
				{
					if (pixelShader(color, x1c, y))
						Surface.Pixel(color, x1c, y);
					if (y == y2c)
						break;
				}
			}
			else // General case: Bresenham's line algorithm.
			{
				bool xMajor;
				{
					const int16_t dxAbs = AbsValue<int16_t>(x2c - x1c);
					const int16_t dyAbs = AbsValue<int16_t>(y2c - y1c);
					xMajor = (dxAbs >= dyAbs);
				}

				if (xMajor) // X-major: ensure x1 < x2 for BresenhamRight.
				{
					if (x2c < x1c)
					{
						BresenhamLineRight(x2c, y2c, x1c, y1c, pixelShader);
					}
					else
					{
						BresenhamLineRight(x1c, y1c, x2c, y2c, pixelShader);
					}
				}
				else // Y-major: ensure y1 < y2 for BresenhamUp.
				{
					if (y2c < y1c)
					{
						BresenhamLineUp(x2c, y2c, x1c, y1c, pixelShader);
					}
					else
					{
						BresenhamLineUp(x1c, y1c, x2c, y2c, pixelShader);
					}
				}
			}
		}

		/// <summary>Triangle raster with clipping + fan triangulation.</summary>
		template<typename pixel_shader_t>
		void RasterTriangle(const int16_t x1, const  int16_t y1, const int16_t x2, const int16_t y2, const int16_t x3, const int16_t y3, pixel_shader_t&& pixelShader)
		{
			// Cache triangle vertices into the clipping polygon buffer.
			clippedPolygon[0] = { x1, y1 };
			clippedPolygon[1] = { x2, y2 };
			clippedPolygon[2] = { x3, y3 };

			// Clip triangle to window using Sutherland-Hodgman algorithm.
			const uint8_t clippedVertexCount = ClipTriangleToWindow();
			switch (clippedVertexCount)
			{
			case 0: // Fully clipped, nothing to draw.
				break;
			case 1: // Degenerate triangle collapsed to a point.
			{
				Rgb8::color_t color{};
				if (pixelShader(color, clippedPolygon[0].x, clippedPolygon[0].y))
					Surface.Pixel(color, clippedPolygon[0].x, clippedPolygon[0].y);
			}
			break;
			case 2: // Degenerate triangle collapsed to a line.
				RasterLine(clippedPolygon[0].x, clippedPolygon[0].y, clippedPolygon[1].x, clippedPolygon[1].y, pixelShader);
				break;
			case 3: // Fast path, whole triangle inside window.
				RasterTriangleDispatch(clippedPolygon[0].x, clippedPolygon[0].y,
					clippedPolygon[1].x, clippedPolygon[1].y,
					clippedPolygon[2].x, clippedPolygon[2].y, pixelShader);
				break;
			default: // General convex polygon (up to 6 vertices) -> triangulate as a fan.
				for (uint8_t i = 1; i + 1 < clippedVertexCount; i++)
				{
					if (clippedPolygon[0].x == clippedPolygon[i].x && clippedPolygon[0].x == clippedPolygon[i + 1].x &&
						clippedPolygon[0].y == clippedPolygon[i].y && clippedPolygon[0].y == clippedPolygon[i + 1].y)
					{
						// Degenerate triangle collapsed to a point.
						Rgb8::color_t color{};
						if (pixelShader(color, clippedPolygon[0].x, clippedPolygon[0].y))
							Surface.Pixel(color, clippedPolygon[0].x, clippedPolygon[0].y);
					}
					else if (clippedPolygon[0].x == clippedPolygon[i].x && clippedPolygon[0].x == clippedPolygon[i + 1].x ||
						clippedPolygon[0].y == clippedPolygon[i].y && clippedPolygon[0].y == clippedPolygon[i + 1].y)
					{
						// Degenerate triangle collapsed to a line.
						RasterLine(clippedPolygon[0].x, clippedPolygon[0].y,
							clippedPolygon[i].x, clippedPolygon[i].y,
							pixelShader);
						RasterLine(clippedPolygon[i].x, clippedPolygon[i].y,
							clippedPolygon[i + 1].x, clippedPolygon[i + 1].y,
							pixelShader);
					}
					else
					{
						RasterTriangleDispatch(clippedPolygon[0].x, clippedPolygon[0].y,
							clippedPolygon[i].x, clippedPolygon[i].y,
							clippedPolygon[i + 1].x, clippedPolygon[i + 1].y, pixelShader);
					}
				}
				break;
			}
		}

		/// <summary>Rectangle raster with pixel shader (clipped).</summary>
		template<typename pixel_shader_t>
		void RasterRectangle(const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2, pixel_shader_t&& pixelShader)
		{
			const uint8_t inCount = IsInsideWindow(x1, y1) + IsInsideWindow(x1, y2) +
				IsInsideWindow(x2, y1) + IsInsideWindow(x2, y2);

			if (!inCount)
				return; // Whole rectangle is out of bounds.

			Rgb8::color_t color{};
			const int16_t x1c = LimitValue<int16_t>(x1, 0, SurfaceWidth - 1);
			const int16_t x2c = LimitValue<int16_t>(x2, 0, SurfaceWidth - 1);
			const int16_t y1c = LimitValue<int16_t>(y1, 0, SurfaceHeight - 1);
			const int16_t y2c = LimitValue<int16_t>(y2, 0, SurfaceHeight - 1);

			if (x1c == x2c && y1c == y2c)
			{
				// Degenerate rectangle, only draw a single pixel.
				if (pixelShader(color, x1c, y1c))
					Surface.Pixel(color, x1c, y1c);
			}
			else if (x1c == x2c)
			{
				// Degenerate rectangle, only draw a vertical line.
				const int8_t step = y1c <= y2c ? 1 : -1;
				for (int_fast16_t y = y1c; ; y += step)
				{
					if (pixelShader(color, x1c, y))
						Surface.Pixel(color, x1c, y);
					if (y == y2c)
						break;
				}
			}
			else if (y1c == y2c)
			{
				// Degenerate rectangle, only draw a horizontal line.
				const int8_t step = x1c <= x2c ? 1 : -1;
				for (int_fast16_t x = x1c; ; x += step)
				{
					if (pixelShader(color, x, y1c))
						Surface.Pixel(color, x, y1c);
					if (x == x2c)
						break;
				}
			}
			else
			{
				// Cropped rectangle.
				const int8_t xStep = x1c <= x2c ? 1 : -1;
				const int8_t yStep = y1c <= y2c ? 1 : -1;
				for (int_fast16_t y = y1c; ; y += yStep)
				{
					for (int_fast16_t x = x1c; ; x += xStep)
					{
						if (pixelShader(color, x, y))
							Surface.Pixel(color, x, y);
						if (x == x2c)
							break;
					}
					if (y == y2c)
						break;
				}
			}
		}

	private:
		/// <summary>Triangle dispatcher (sorts by Y).</summary>
		template<typename pixel_shader_t>
		void RasterTriangleDispatch(const int16_t x1, const  int16_t y1,
			const int16_t x2, const  int16_t y2,
			const int16_t x3, const  int16_t y3,
			pixel_shader_t&& pixelShader)
		{
			if (y1 <= y2 && y1 <= y3)
			{
				if (y2 <= y3)
					RasterTriangleYOrdered(x1, y1, x2, y2, x3, y3, pixelShader);
				else
					RasterTriangleYOrdered(x1, y1, x3, y3, x2, y2, pixelShader);
			}
			else if (y2 <= y1 && y2 <= y3)
			{
				if (y1 <= y3)
					RasterTriangleYOrdered(x2, y2, x1, y1, x3, y3, pixelShader);
				else
					RasterTriangleYOrdered(x2, y2, x3, y3, x1, y1, pixelShader);
			}
			else
			{
				if (y1 <= y2)
					RasterTriangleYOrdered(x3, y3, x1, y1, x2, y2, pixelShader);
				else
					RasterTriangleYOrdered(x3, y3, x2, y2, x1, y1, pixelShader);
			}
		}

		/// <summary>Raster ordered triangle; splits if needed.</summary>
		template<typename pixel_shader_t>
		void RasterTriangleYOrdered(const int16_t x1, const int16_t y1,
			const int16_t x2, const int16_t y2,
			const int16_t x3, const int16_t y3,
			pixel_shader_t&& pixelShader)
		{
			if (y1 == y2 && y2 == y3)
			{
				Rgb8::color_t color{};
				if (x1 == x2 && x2 == x3)
				{
					if (pixelShader(color, x1, y1))
						Surface.Pixel(color, x1, y1);
					return;
				}
				int16_t xMin = (x2 < x1 ? x2 : x1);
				xMin = (x3 < xMin ? x3 : xMin);
				int16_t xMax = (x2 > x1 ? x2 : x1);
				xMax = (x3 > xMax ? x3 : xMax);

				for (int_fast16_t x = xMin; x <= xMax; ++x)
				{
					if (pixelShader(color, x, y1))
						Surface.Pixel(color, x, y1);
				}
			}
			else if (y2 == y3)
			{
				BresenhamTriangleFlatBottom(x1, y1, x2, y2, x3, y3, pixelShader);
			}
			else if (y1 == y2)
			{
				BresenhamTriangleFlatTop(x1, y1, x2, y2, x3, y3, pixelShader);
			}
			else if (y3 == y1)
			{
				Rgb8::color_t color{};
				if (x1 == x3)
				{
					if (pixelShader(color, x1, y1))
						Surface.Pixel(color, x1, y1);
				}
				else
				{
					const int8_t step = x1 <= x3 ? 1 : -1;
					for (int_fast16_t x = x1; ; x += step)
					{
						if (pixelShader(color, x, y1))
							Surface.Pixel(color, x, y1);
						if (x == x3)
							break;
					}
				}
			}
			else
			{
				// Split at scanline of middle vertex (fixed-point interpolation).
				const int16_t dxTotal = x3 - x1;
				const int16_t dyTotal = y3 - y1;
				const int16_t dySegment = y2 - y1;

				const int16_t splitX = FixedRoundToInt(IntToFixed(x1)
					+ (IntToFixed(dxTotal) * dySegment) / dyTotal);

				BresenhamTriangleFlatBottom(x1, y1, x2, y2, splitX, y2, pixelShader);
				BresenhamTriangleFlatTop(x2, y2, splitX, y2, x3, y3, pixelShader);
			}
		}

		/// <summary>Fill flat-bottom triangle.</summary>
		template<typename pixel_shader_t>
		void BresenhamTriangleFlatBottom(int16_t x1, int16_t y1,
			int16_t x2, int16_t y2,
			int16_t x3, int16_t y3,
			pixel_shader_t&& pixelShader)
		{
			const int32_t dx1 = IntToFixed(x2 - x1) / (y2 - y1);
			const int32_t dx2 = IntToFixed(x3 - x1) / (y3 - y1);
			int32_t sx1 = IntToFixed(x1);
			int32_t sx2 = sx1;

			Rgb8::color_t color{};
			for (int16_t y = y1; y <= y2; y++)
			{
				int16_t startX = FixedRoundToInt(sx1);
				int16_t endX = FixedRoundToInt(sx2);
				if (startX > endX) { int16_t t = startX; startX = endX; endX = t; }

				for (int16_t x = startX; x <= endX; x++)
				{
					if (pixelShader(color, x, y))
						Surface.Pixel(color, x, y);
				}

				sx1 += dx1;
				sx2 += dx2;
			}
		}

		/// <summary>Fill flat-top triangle.</summary>
		template<typename pixel_shader_t>
		void BresenhamTriangleFlatTop(int16_t x1, int16_t y1,
			int16_t x2, int16_t y2,
			int16_t x3, int16_t y3,
			pixel_shader_t&& pixelShader)
		{
			const int32_t dx1 = IntToFixed(x3 - x1) / (y3 - y1);
			const int32_t dx2 = IntToFixed(x3 - x2) / (y3 - y2);
			int32_t sx1 = IntToFixed(x3);
			int32_t sx2 = sx1;

			Rgb8::color_t color{};
			for (int16_t y = y3; y >= y1; y--)
			{
				int16_t startX = FixedRoundToInt(sx1);
				int16_t endX = FixedRoundToInt(sx2);
				if (startX > endX) { int16_t t = startX; startX = endX; endX = t; }

				for (int16_t x = startX; x <= endX; x++)
				{
					if (pixelShader(color, x, y))
						Surface.Pixel(color, x, y);
				}

				sx1 -= dx1;
				sx2 -= dx2;
			}
		}

		template<typename pixel_shader_t>
		void BresenhamLineRight(const int16_t x1, const int16_t y1,
			const int16_t x2, const int16_t y2, pixel_shader_t&& pixelShader)
		{
			const uint16_t scaledWidth = static_cast<uint16_t>(x2 - x1) << 1;
			const uint16_t slopeMagnitude = static_cast<uint16_t>(AbsValue<int16_t>(y2 - y1)) << 1;
			const int8_t slopeUnit = (y2 >= y1) ? 1 : -1;
			const int8_t slopeSign = (x2 >= x1) ? 1 : -1;

			Rgb8::color_t color{};
			int32_t slopeError = slopeMagnitude - (x2 - x1);
			int_fast16_t y = y1;
			for (int_fast16_t x = x1; x != x2; x += slopeSign)
			{
				if (pixelShader(color, x, y))
					Surface.Pixel(color, x, y);

				slopeError += slopeMagnitude;
				if (slopeError >= 0)
				{
					y += slopeUnit;
					slopeError -= scaledWidth;
				}
			}
		}

		template<typename pixel_shader_t>
		void BresenhamLineUp(const int16_t x1, const int16_t y1,
			const int16_t x2, const int16_t y2, pixel_shader_t&& pixelShader)
		{
			const uint16_t scaledHeight = static_cast<uint16_t>(y2 - y1) << 1;
			const uint16_t slopeMagnitude = static_cast<uint16_t>(AbsValue<int16_t>(x2 - x1)) << 1;
			const int8_t slopeUnit = (x2 >= x1) ? 1 : -1;
			const int8_t slopeSign = (y2 >= y1) ? 1 : -1;

			Rgb8::color_t color{};
			int32_t slopeError = slopeMagnitude - (y2 - y1);
			int_fast16_t x = x1;
			for (int_fast16_t y = y1; y != y2; y += slopeSign)
			{
				if (pixelShader(color, x, y))
					Surface.Pixel(color, x, y);

				slopeError += slopeMagnitude;
				if (slopeError >= 0)
				{
					x += slopeUnit;
					slopeError -= scaledHeight;
				}
			}
		}
	};
}
#endif