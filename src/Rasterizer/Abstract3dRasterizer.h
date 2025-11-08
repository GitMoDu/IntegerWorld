#ifndef _INTEGER_WORLD_ABSTRACT_3D_RASTERIZER_h
#define _INTEGER_WORLD_ABSTRACT_3D_RASTERIZER_h

#include "Abstract3dDrawer.h"

namespace IntegerWorld
{
	/// <summary>
	/// 3D rasterization layer built on top of Abstract3dDrawer.
	/// - Adds shaderable Raster* helpers (per-pixel PixelShader) with z=0 plane clipping for vertex16_t primitives.
	/// - Projects visible portions into 2D and forwards to 2D Raster* routines in the base.
	/// - Re-exposes 2D Raster* overloads to avoid name hiding by the 3D Raster* overloads here.
	/// </summary>
	/// <typeparam name="SurfaceType">The output surface type (IOutputSurface-like API).</typeparam>
	template<typename SurfaceType>
	class Abstract3dRasterizer : public Abstract3dDrawer<SurfaceType>
	{
	public:
		using Abstract3dDrawer<SurfaceType>::RasterLine;     // 2D variant
		using Abstract3dDrawer<SurfaceType>::RasterTriangle; // 2D variant

	public:
		Abstract3dRasterizer(SurfaceType& surface)
			: Abstract3dDrawer<SurfaceType>(surface)
		{
		}

	public:
		/// <summary>
		/// Rasterizes a 3D line segment with Z-plane clipping and applying a custom pixel shader to each pixel.
		/// The line is clipped to the window boundaries as needed.
		/// </summary>
		/// <typeparam name="PixelShader">Callable signature: bool(Rgb8::color_t&amp; color, int16_t x, int16_t y).</typeparam>
		/// <param name="start">The starting vertex of the line segment, containing x, y, and z coordinates.</param>
		/// <param name="end">The ending vertex of the line segment, containing x, y, and z coordinates.</param>
		/// <param name="pixelShader">A callable that determines color/visibility of each pixel along the line.</param>
		template<typename PixelShader>
		void RasterLine(const vertex16_t& start, const vertex16_t& end, PixelShader&& pixelShader)
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
				// TODO: One endpoint is behind z=0, clip to z=0 plane
				return;
			}

			// Now both s.z and e.z >= 0, so we can rasterize in 2D
			Abstract2dRasterizer<SurfaceType>::RasterLine(s.x, s.y, e.x, e.y, pixelShader);
		}

		/// <summary>
		/// Rasterizes (fills) a triangle defined by three 3D vertices using a custom pixel shader.
		/// Handles z-plane clipping: only pixels with z > 0 are considered visible.
		/// The pixel shader is invoked for each pixel within the visible portion of the triangle, allowing for per-pixel color and visibility control.
		/// </summary>
		/// <typeparam name="PixelShader">Callable signature: bool(Rgb8::color_t&amp; color, int16_t x, int16_t y).</typeparam>
		/// <param name="a">The first vertex of the triangle, including x, y, and z coordinates.</param>
		/// <param name="b">The second vertex of the triangle, including x, y, and z coordinates.</param>
		/// <param name="c">The third vertex of the triangle, including x, y, and z coordinates.</param>
		/// <param name="pixelShader">A callable object or function that determines the color and visibility of each pixel along the triangle surface.</param>
		template<typename PixelShader>
		void RasterTriangle(const vertex16_t& a, const vertex16_t& b, const vertex16_t& c, PixelShader&& pixelShader)
		{
			if (a.z == b.z
				&& a.z == c.z)
			{
				// Screen plane 2D triangle, only need to check one point for bounds.
				if (a.z > 0)
				{
					Abstract2dRasterizer<SurfaceType>::RasterTriangle(a.x, a.y, b.x, b.y, c.x, c.y, pixelShader);
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
					Abstract2dRasterizer<SurfaceType>::RasterTriangle(a.x, a.y, b.x, b.y, c.x, c.y, pixelShader);
					break;
				default:
					break;
				}
			}
		}
	};
}
#endif