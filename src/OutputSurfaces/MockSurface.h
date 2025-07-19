#ifndef _INTEGER_WORLD_MOCK_SURFACE_h
#define _INTEGER_WORLD_MOCK_SURFACE_h

#include "../Framework/Interface.h"

namespace IntegerWorld
{
	namespace MockOutput
	{
		template<uint16_t surfaceWidth, uint16_t surfaceHeight>
		struct OutputSurface : IOutputSurface
		{
			bool StartSurface() final { return true; }
			void StopSurface() final {}
			bool IsSurfaceReady() final { return true; }
			void FlipSurface() final {}

			void GetSurfaceDimensions(int16_t& width, int16_t& height, uint8_t& colorDepth) final
			{
				width = surfaceWidth; height = surfaceHeight; colorDepth = 16;
			}

#if defined(ARDUINO)
			void PrintName(Print& serial)
			{
				serial.print(F("Mock Output"));
			}
#endif

			void Pixel(const color_fraction16_t color, const int16_t x, const int16_t y) final {}
			void Line(const color_fraction16_t color, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2) final {}
			void TriangleFill(const color_fraction16_t color, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2, const int16_t x3, const int16_t y3) final {}
			void RectangleFill(const color_fraction16_t color, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2) final {}
		};
	}
}

#endif