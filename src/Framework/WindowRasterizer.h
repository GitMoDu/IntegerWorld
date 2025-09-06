#ifndef _INTEGER_WORLD_WINDOW_RASTERIZER_h
#define _INTEGER_WORLD_WINDOW_RASTERIZER_h

#include "../Rasterizer/Abstract3dRasterizer.h"
#include "IOutputSurface.h"

namespace IntegerWorld
{
	/// <summary>
	/// WindowRasterizer:
	/// - Direct Mode 2D/3D drawing API via Abstract3dRasterizer on an IOutputSurface.
	/// - Clipped draw window; window dimensions are queried from the surface.
	/// </summary>
	using WindowRasterizer = Abstract3dRasterizer<IOutputSurface>;

	/// <summary>
	/// Thin wrapper that binds an IOutputSurface and exposes convenience surface lifecycle and dimension update methods.
	/// </summary>
	struct SurfacedWindowRasterizer : WindowRasterizer
	{
		/// <summary>
		/// Constructs a rasterizer bound to the provided output surface.
		/// </summary>
		/// <param name="surface">Output surface to draw to.</param>
		SurfacedWindowRasterizer(IOutputSurface& surface)
			: WindowRasterizer(surface)
		{
		}

		/// <summary>
		/// Queries the surface for width/height/depth and updates the rasterizer window dimensions.
		/// </summary>
		/// <returns>True on success.</returns>
		bool UpdateDimensions()
		{
			int16_t width, height;
			uint8_t colorDepth;
			Surface.GetSurfaceDimensions(width, height, colorDepth);

			SurfaceWidth = width;
			SurfaceHeight = height;

			return true;
		}

		/// <summary>
		/// Starts the surface/framebuffer for drawing.
		/// </summary>
		/// <returns>True if the surface is available and ready.</returns>
		bool StartSurface()
		{
			return Surface.StartSurface();
		}

		/// <summary>
		/// Stops access to the surface/framebuffer.
		/// </summary>
		void StopSurface()
		{
			Surface.StopSurface();
		}

		/// <summary>
		/// Returns true if the surface is ready to accept drawing commands.
		/// </summary>
		bool IsSurfaceReady()
		{
			return Surface.IsSurfaceReady();
		}

		/// <summary>
		/// Presents or flips the underlying surface buffers.
		/// </summary>
		void FlipSurface()
		{
			Surface.FlipSurface();
		}
	};
}
#endif