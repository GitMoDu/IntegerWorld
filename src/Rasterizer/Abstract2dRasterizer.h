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
	private:
		using Base = Abstract2dDrawer<SurfaceType>;

	public:
		using Abstract2dDrawer<SurfaceType>::BlendPixel;

	protected:
		using Abstract2dDrawer<SurfaceType>::Surface;
		using Abstract2dDrawer<SurfaceType>::SurfaceWidth;
		using Abstract2dDrawer<SurfaceType>::SurfaceHeight;
		using Abstract2dDrawer<SurfaceType>::IsInsideWindow;
		using Abstract2dDrawer<SurfaceType>::ClipEndpointToWindow;
		using Abstract2dDrawer<SurfaceType>::ClipTriangleToWindow;
		using Abstract2dDrawer<SurfaceType>::FixedCeilToInt;
		using Abstract2dDrawer<SurfaceType>::IntToFixed;
		using Abstract2dDrawer<SurfaceType>::clippedPolygon;

	public:
		Abstract2dRasterizer(SurfaceType& surface)
			: Abstract2dDrawer<SurfaceType>(surface)
		{
		}

	public:
		/// <summary>Rasterize a clipped line with pixel shader.</summary>
		template<pixel_blend_mode_t blendMode, typename pixel_shader_t>
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

			if (x1c == x2c && y1c == y2c) // Degenerate line (point).
			{
				Base::template BlendPixel<blendMode>(pixelShader(x1c, y1c), x1c, y1c);
			}
			else if (y1c == y2c) // Horizontal line.
			{
				const int8_t step = x1c <= x2c ? 1 : -1;
				for (int_fast16_t x = x1c; ; x += step)
				{
					Base::template BlendPixel<blendMode>(pixelShader(x, y1c), x, y1c);
					if (x == x2c)
						break;
				}
			}
			else if (x1c == x2c) // Vertical line.
			{
				const int8_t step = y1c <= y2c ? 1 : -1;
				for (int_fast16_t y = y1c; ; y += step)
				{
					Base::template BlendPixel<blendMode>(pixelShader(x1c, y), x1c, y);
					if (y == y2c)
						break;
				}
			}
			else // General case: Bresenham's line algorithm.
			{
				const int16_t dxAbs = AbsValue<int16_t>(x2c - x1c);
				const int16_t dyAbs = AbsValue<int16_t>(y2c - y1c);
				const bool xMajor = (dxAbs >= dyAbs);

				if (xMajor)
				{
					if (x2c < x1c)
						BresenhamLineRight<blendMode>(x2c, y2c, x1c, y1c, pixelShader);
					else
						BresenhamLineRight<blendMode>(x1c, y1c, x2c, y2c, pixelShader);
				}
				else
				{
					if (y2c < y1c)
						BresenhamLineUp<blendMode>(x2c, y2c, x1c, y1c, pixelShader);
					else
						BresenhamLineUp<blendMode>(x1c, y1c, x2c, y2c, pixelShader);
				}
			}
		}

		template<typename pixel_shader_t>
		void RasterLine(const int16_t x1, const  int16_t y1, const int16_t x2, const int16_t y2, pixel_shader_t&& pixelShader)
		{
			RasterLine<pixel_blend_mode_t::Replace>(x1, y1, x2, y2, pixelShader);
		}

		/// <summary>Triangle raster with clipping + fan triangulation.</summary>
		template<pixel_blend_mode_t blendMode, typename pixel_shader_t>
		void RasterTriangle(const int16_t x1, const  int16_t y1, const int16_t x2, const int16_t y2, const int16_t x3, const int16_t y3, pixel_shader_t&& pixelShader)
		{
			clippedPolygon[0] = { x1, y1 };
			clippedPolygon[1] = { x2, y2 };
			clippedPolygon[2] = { x3, y3 };

			const uint8_t clippedVertexCount = ClipTriangleToWindow();
			switch (clippedVertexCount)
			{
			case 0:
				return;
			case 1:
				Base::template BlendPixel<blendMode>(pixelShader(clippedPolygon[0].x, clippedPolygon[0].y),
					clippedPolygon[0].x, clippedPolygon[0].y);
				return;
			case 2:
				RasterLine<blendMode>(clippedPolygon[0].x, clippedPolygon[0].y,
					clippedPolygon[1].x, clippedPolygon[1].y, pixelShader);
				return;
			case 3:
				RasterTriangleDispatch<blendMode>(
					clippedPolygon[0].x, clippedPolygon[0].y,
					clippedPolygon[1].x, clippedPolygon[1].y,
					clippedPolygon[2].x, clippedPolygon[2].y,
					pixelShader);
				return;
			default:
				for (uint_fast8_t i = 1; i + 1 < clippedVertexCount; ++i)
				{
					RasterTriangleDispatch<blendMode>(
						clippedPolygon[0].x, clippedPolygon[0].y,
						clippedPolygon[i].x, clippedPolygon[i].y,
						clippedPolygon[i + 1].x, clippedPolygon[i + 1].y,
						pixelShader);
				}
				return;
			}
		}

		template<typename pixel_shader_t>
		void RasterTriangle(const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2, const int16_t x3, const int16_t y3, pixel_shader_t&& pixelShader)
		{
			RasterTriangle<pixel_blend_mode_t::Replace>(x1, y1, x2, y2, x3, y3, pixelShader);
		}

		/// <summary>Rectangle raster with pixel shader (clipped).</summary>
		template<pixel_blend_mode_t blendMode, typename pixel_shader_t>
		void RasterRectangle(const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2, pixel_shader_t&& pixelShader)
		{
			const uint8_t inCount = IsInsideWindow(x1, y1) + IsInsideWindow(x1, y2) +
				IsInsideWindow(x2, y1) + IsInsideWindow(x2, y2);

			if (!inCount)
				return; // Whole rectangle is out of bounds.

			const int16_t x1c = LimitValue<int16_t>(x1, 0, SurfaceWidth - 1);
			const int16_t x2c = LimitValue<int16_t>(x2, 0, SurfaceWidth - 1);
			const int16_t y1c = LimitValue<int16_t>(y1, 0, SurfaceHeight - 1);
			const int16_t y2c = LimitValue<int16_t>(y2, 0, SurfaceHeight - 1);

			if (x1c == x2c && y1c == y2c)
			{
				// Degenerate rectangle, only draw a single pixel.
				Base::template BlendPixel<blendMode>(pixelShader(x1c, y1c), x1c, y1c);
			}
			else if (x1c == x2c)
			{
				// Degenerate rectangle, only draw a vertical line.
				const int8_t step = y1c <= y2c ? 1 : -1;
				for (int_fast16_t y = y1c; ; y += step)
				{
					Base::template BlendPixel<blendMode>(pixelShader(x1c, y), x1c, y);
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
					Base::template BlendPixel<blendMode>(pixelShader(x, y1c), x, y1c);
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
						Base::template BlendPixel<blendMode>(pixelShader(x, y), x, y);
						if (x == x2c)
							break;
					}
					if (y == y2c)
						break;
				}
			}
		}

		template<typename pixel_shader_t>
		void RasterRectangle(const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2, pixel_shader_t&& pixelShader)
		{
			RasterRectangle<pixel_blend_mode_t::Replace>(x1, y1, x2, y2, pixelShader);
		}

	private:
		// Dispatches triangle vertices to edge-fill routine in sorted Y order.
		template<pixel_blend_mode_t blendMode, typename pixel_shader_t>
		void RasterTriangleDispatch(const int16_t x1, const  int16_t y1,
			const int16_t x2, const  int16_t y2,
			const int16_t x3, const  int16_t y3,
			pixel_shader_t&& pixelShader)
		{
			if (y1 <= y2 && y1 <= y3)
			{
				if (y2 <= y3)
					RasterTriangleEdgeFill<blendMode>(x1, y1, x2, y2, x3, y3, pixelShader);
				else
					RasterTriangleEdgeFill<blendMode>(x1, y1, x3, y3, x2, y2, pixelShader);
			}
			else if (y2 <= y1 && y2 <= y3)
			{
				if (y1 <= y3)
					RasterTriangleEdgeFill<blendMode>(x2, y2, x1, y1, x3, y3, pixelShader);
				else
					RasterTriangleEdgeFill<blendMode>(x2, y2, x3, y3, x1, y1, pixelShader);
			}
			else
			{
				if (y1 <= y2)
					RasterTriangleEdgeFill<blendMode>(x3, y3, x1, y1, x2, y2, pixelShader);
				else
					RasterTriangleEdgeFill<blendMode>(x3, y3, x2, y2, x1, y1, pixelShader);
			}
		}

		// Edge-function based triangle fill. Eliminates seams & overdraw (top-left rule).
		template<pixel_blend_mode_t blendMode, typename pixel_shader_t>
		void RasterTriangleEdgeFill(const int16_t x0, const int16_t y0,
			const int16_t x1, const int16_t y1,
			const int16_t x2, const int16_t y2,
			pixel_shader_t&& pixelShader)
		{
			// Degenerate (all vertices share Y): single scanline fill.
			if (y0 == y2)
			{
				const int16_t xStart = MinValue<int16_t>(x0, MinValue<int16_t>(x1, x2));
				const int16_t xEnd = MaxValue<int16_t>(x0, MaxValue<int16_t>(x1, x2));

				for (int_fast16_t x = xStart; x <= xEnd; x++)
				{
					Base::template BlendPixel<blendMode>(pixelShader(x, y0), x, y0);
				}
				return;
			}

			// Fixed-point X positions for original vertices.
			const int32_t fx0 = IntToFixed(x0);
			const int32_t fx1 = IntToFixed(x1);

			// Sorted Y (already ensured by caller dispatch order).
			const int16_t hTop = y1 - y0;      // Height of top segment.
			const int16_t hBottom = y2 - y1;   // Height of bottom segment.
			const int16_t hTotal = y2 - y0;    // Total height.

			// Fixed-point per-scanline X deltas along each relevant edge.
			const int32_t dxLong = (hTotal != 0) ? (IntToFixed(x2 - x0) / hTotal) : 0;   // Edge x0->x2
			const int32_t dxTop = (hTop != 0) ? (IntToFixed(x1 - x0) / hTop) : 0;        // Edge x0->x1
			const int32_t dxBottom = (hBottom != 0) ? (IntToFixed(x2 - x1) / hBottom) : 0; // Edge x1->x2

			// Determine orientation: compare long edge X at y1 vs actual x1.
			const bool longEdgeIsLeft = ((fx0 + dxLong * hTop) <= fx1);

			// Raster top segment.
			if (hTop > 0)
			{
				// Working scanline edge positions and steps.
				const int32_t stepLeft = longEdgeIsLeft ? dxLong : dxTop;
				const int32_t stepRight = longEdgeIsLeft ? dxTop : dxLong;
				int32_t fxLeft = fx0;
				int32_t fxRight = fx0;

				for (int_fast16_t y = y0; y < y1; y++)
				{
					// Top-left rule: left edge ceil, right edge ceil - 1 (half-open on the right).
					const int16_t startX = FixedCeilToInt(fxLeft);
					const int16_t endX = FixedCeilToInt(fxRight) - 1;

					if (startX <= endX)
					{
						for (int_fast16_t x = startX; x <= endX; x++)
							Base::template BlendPixel<blendMode>(pixelShader(x, y), x, y);
					}

					fxLeft += stepLeft;
					fxRight += stepRight;
				}
			}

			// Raster bottom segment.
			if (hBottom > 0)
			{
				// Working scanline edge positions and steps.
				const int32_t stepLeft = longEdgeIsLeft ? dxLong : dxBottom;
				const int32_t stepRight = longEdgeIsLeft ? dxBottom : dxLong;
				int32_t fxLeft = longEdgeIsLeft ? (fx0 + dxLong * hTop) : fx1;
				int32_t fxRight = longEdgeIsLeft ? fx1 : (fx0 + dxLong * hTop);

				for (int_fast16_t y = y1; y < y2; y++)
				{
					// Top-left rule: left edge ceil, right edge ceil - 1 (half-open on the right).
					const int16_t startX = FixedCeilToInt(fxLeft);
					const int16_t endX = FixedCeilToInt(fxRight) - 1;

					if (startX <= endX)
					{
						for (int_fast16_t x = startX; x <= endX; x++)
							Base::template BlendPixel<blendMode>(pixelShader(x, y), x, y);
					}

					fxLeft += stepLeft;
					fxRight += stepRight;
				}
			}
			// Bottom scanline excluded (half-open vertical interval).
		}

		template<pixel_blend_mode_t blendMode, typename pixel_shader_t>
		void BresenhamLineRight(const int16_t x1, const int16_t y1,
			const int16_t x2, const int16_t y2, pixel_shader_t&& pixelShader)
		{
			const uint16_t scaledWidth = static_cast<uint16_t>(x2 - x1) << 1;
			const uint16_t slopeMagnitude = static_cast<uint16_t>(AbsValue<int16_t>(y2 - y1)) << 1;
			const int8_t slopeUnit = (y2 >= y1) ? 1 : -1;
			const int8_t slopeSign = (x2 >= x1) ? 1 : -1;

			int32_t slopeError = slopeMagnitude - (x2 - x1);
			int_fast16_t y = y1;
			for (int_fast16_t x = x1; x != x2; x += slopeSign)
			{
				Base::template BlendPixel<blendMode>(pixelShader(x, y), x, y);

				slopeError += slopeMagnitude;
				if (slopeError >= 0)
				{
					y += slopeUnit;
					slopeError -= scaledWidth;
				}
			}
			Base::template BlendPixel<blendMode>(pixelShader(x2, y), x2, y); // last pixel
		}

		template<pixel_blend_mode_t blendMode, typename pixel_shader_t>
		void BresenhamLineUp(const int16_t x1, const int16_t y1,
			const int16_t x2, const int16_t y2, pixel_shader_t&& pixelShader)
		{
			const uint16_t scaledHeight = static_cast<uint16_t>(y2 - y1) << 1;
			const uint16_t slopeMagnitude = static_cast<uint16_t>(AbsValue<int16_t>(x2 - x1)) << 1;
			const int8_t slopeUnit = (x2 >= x1) ? 1 : -1;
			const int8_t slopeSign = (y2 >= y1) ? 1 : -1;

			int32_t slopeError = slopeMagnitude - (y2 - y1);
			int_fast16_t x = x1;
			for (int_fast16_t y = y1; y != y2; y += slopeSign)
			{
				Base::template BlendPixel<blendMode>(pixelShader(x, y), x, y);

				slopeError += slopeMagnitude;
				if (slopeError >= 0)
				{
					x += slopeUnit;
					slopeError -= scaledHeight;
				}
			}
			Base::template BlendPixel<blendMode>(pixelShader(x, y2), x, y2); // last pixel
		}
	};
}
#endif