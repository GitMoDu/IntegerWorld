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
		virtual bool StartSurface() = 0;
		virtual void StopSurface() = 0;

		virtual void FlipSurface() = 0;
		virtual bool IsSurfaceReady() = 0;


	public:// Buffer window interface.
		virtual void GetSurfaceDimensions(int16_t& width, int16_t& height, uint8_t& colorDepth) = 0;

	public:// Buffer drawing interface.
		virtual void Pixel(const color_fraction16_t color, const int16_t x, const int16_t y) = 0;
		virtual void Line(const color_fraction16_t color, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2) = 0;
		virtual void TriangleFill(const color_fraction16_t color, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2, const int16_t x3, const int16_t y3) = 0;
		virtual void RectangleFill(const color_fraction16_t color, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2) = 0;
	};
}
#endif