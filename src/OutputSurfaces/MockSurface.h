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
			virtual bool StartSurface() { return true; }

			virtual bool IsSurfaceReady() { return true; }

			virtual void GetSurfaceDimensions(int16_t& width, int16_t& height, uint8_t& colorDepth)
			{
				width = surfaceWidth; height = surfaceHeight; colorDepth = 16;
			}

#if defined(ARDUINO)
			void PrintName(Print& serial)
			{
				serial.print(F("Mock Output"));
			}
#endif
		};
	}
}

#endif