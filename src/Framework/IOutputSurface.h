#ifndef _INTEGER_WORLD_IOUTPUT_SURFACE_h
#define _INTEGER_WORLD_IOUTPUT_SURFACE_h

#include <IntegerSignal.h>
#include <IntegerTrigonometry16.h>

namespace IntegerWorld
{
	enum class pixel_blend_mode_t : uint8_t
	{
		Alpha,		// Blends the new color with the existing pixel color using alpha blending.
		Add,		// Adds the new color to the existing pixel color.
		Subtract,	// Subtracts the new color from the existing pixel color.
		Multiply,	// Multiplies the new color with the existing pixel color.
		Screen		// Blends the new color with the existing pixel color using the screen blend mode.
	};

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
		virtual void Pixel(const Rgb8::color_t color, const int16_t x, const int16_t y) = 0;
		virtual void Line(const Rgb8::color_t color, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2) = 0;
		virtual void TriangleFill(const Rgb8::color_t color, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2, const int16_t x3, const int16_t y3) = 0;
		virtual void RectangleFill(const Rgb8::color_t color, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2) = 0;

		virtual void PixelBlendAlpha(const Rgb8::color_t color, const int16_t x, const int16_t y) = 0;
		virtual void PixelBlendAdd(const Rgb8::color_t color, const int16_t x, const int16_t y) = 0;
		virtual void PixelBlendSubtract(const Rgb8::color_t color, const int16_t x, const int16_t y) = 0;
		virtual void PixelBlendMultiply(const Rgb8::color_t color, const int16_t x, const int16_t y) = 0;
		virtual void PixelBlendScreen(const Rgb8::color_t color, const int16_t x, const int16_t y) = 0;
	};
}
#endif