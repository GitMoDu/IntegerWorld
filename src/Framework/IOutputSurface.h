#ifndef _INTEGER_WORLD_IOUTPUT_SURFACE_h
#define _INTEGER_WORLD_IOUTPUT_SURFACE_h

#include <IntegerSignal.h>
#include <IntegerTrigonometry16.h>

namespace IntegerWorld
{
	using namespace ColorFraction;

	/// <summary>
	/// Interface for output framebuffer/screen.
	/// </summary>
	struct IOutputSurface
	{
	public:// Buffer managment interface.
		virtual void StartSurface() {}
		virtual void StopSurface() {}

		virtual void FlipSurface() {}
		virtual bool IsSurfaceReady() { return false; }


	public:// Buffer window interface.
		virtual void GetSurfaceDimensions(int16_t& width, int16_t& height, uint8_t& colorDepth) { width = 0; height = 0; colorDepth = 0; }

	public:// Buffer drawing interface.
		virtual void Pixel(const color_fraction16_t color, const int16_t x, const int16_t y) {}
		virtual void Line(const color_fraction16_t color, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2) {}
		virtual void TriangleFill(const color_fraction16_t color, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2, const int16_t x3, const int16_t y3) {}
		virtual void RectangleFill(const color_fraction16_t color, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2) {}
	};
}
#endif