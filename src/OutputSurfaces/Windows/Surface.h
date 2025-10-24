#ifndef _INTEGER_WORLD_DIRECTX_SURFACE_h
#define _INTEGER_WORLD_DIRECTX_SURFACE_h

#include "SwapChainSurfaceWinRT.h"
#include "SwapChainSurfaceUniversal.h"

namespace IntegerWorld
{
	template<int16_t SurfaceWidth, int16_t SurfaceHeight>
	class DirectXSurface : public SwapChainDirectX::SwapChainSurface<SurfaceWidth, SurfaceHeight>
	{
	public:
		DirectXSurface() = default;

	public:// Buffer drawing interface.
		void Pixel(const Rgb8::color_t color, const int16_t x, const int16_t y) final
		{
			if (frameBuffer.size() != SurfaceWidth * SurfaceHeight)
				return;

			if (x < 0 || x >= SurfaceWidth || y < 0 || y >= SurfaceHeight)
				return;
			frameBuffer[y * SurfaceWidth + x] = color;
		}
		
		void Line(const Rgb8::color_t color, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2) final
		{
			if (frameBuffer.size() != SurfaceWidth * SurfaceHeight)
				return;

			int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
			int dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
			int err = dx + dy, e2;
			int x = x1, y = y1;
			while (true)
			{
				if (x >= 0 && x < SurfaceWidth && y >= 0 && y < SurfaceHeight)
					frameBuffer[y * SurfaceWidth + x] = color;
				if (x == x2 && y == y2)
					break;
				e2 = 2 * err;
				if (e2 >= dy) { err += dy; x += sx; }
				if (e2 <= dx) { err += dx; y += sy; }
			}
		}

		void TriangleFill(const Rgb8::color_t color, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2, const int16_t x3, const int16_t y3) final
		{
			if (frameBuffer.size() != SurfaceWidth * SurfaceHeight)
				return;

			const int16_t xStart = LimitValue<int16_t>(MinValue(x1, MinValue(x2, x3)), 0, SurfaceWidth - 1);
			const int16_t xEnd = LimitValue<int16_t>(MaxValue(x1, MaxValue(x2, x3)), 0, SurfaceWidth - 1);
			const int16_t yStart = LimitValue<int16_t>(MinValue(y1, MinValue(y2, y3)), 0, SurfaceHeight - 1);
			const int16_t yEnd = LimitValue<int16_t>(MaxValue(y1, MaxValue(y2, y3)), 0, SurfaceHeight - 1);

			if ((xEnd - xStart) < 1
				|| (yEnd - yStart) < 1)
				return;

			for (int16_t y = yStart; y <= yEnd; ++y)
			{
				for (int16_t x = xStart; x <= xEnd; ++x)
				{
					if (PointInTriangle(x, y, x1, y1, x2, y2, x3, y3))
					{
						frameBuffer[y * SurfaceWidth + x] = color;
					}
				}
			}
		}

		void RectangleFill(const Rgb8::color_t color, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2) final
		{
			if (frameBuffer.size() != SurfaceWidth * SurfaceHeight)
				return;

			const int16_t xStart = LimitValue<int16_t>(MinValue(x1, x2), 0, SurfaceWidth - 1);
			const int16_t xEnd = LimitValue<int16_t>(MaxValue(x1, x2), 0, SurfaceWidth - 1);
			const int16_t yStart = LimitValue<int16_t>(MinValue(y1, y2), 0, SurfaceHeight - 1);
			const int16_t yEnd = LimitValue<int16_t>(MaxValue(y1, y2), 0, SurfaceHeight - 1);

			if ((xEnd - xStart) < 1
				|| (yEnd - yStart) < 1)
				return;

			for (int16_t y = yStart; y <= yEnd; ++y)
			{
				for (int16_t x = xStart; x <= xEnd; ++x)
				{
					frameBuffer[y * SurfaceWidth + x] = color;
				}
			}
		}

		void PixelBlendAlpha(const Rgb8::color_t color, const int16_t x, const int16_t y) final
		{
			if (frameBuffer.size() != SurfaceWidth * SurfaceHeight)
				return;

			if (x < 0 || x >= SurfaceWidth || y < 0 || y >= SurfaceHeight)
				return;
			Rgb8::color_t src = color;
			Rgb8::color_t dst = frameBuffer[y * SurfaceWidth + x];

			uint8_t srcA = Rgb8::Alpha(src);
			float alpha = srcA / 255.0f;

			uint8_t outR = static_cast<uint8_t>(Rgb8::Red(src) * alpha + Rgb8::Red(dst) * (1.0f - alpha));
			uint8_t outG = static_cast<uint8_t>(Rgb8::Green(src) * alpha + Rgb8::Green(dst) * (1.0f - alpha));
			uint8_t outB = static_cast<uint8_t>(Rgb8::Blue(src) * alpha + Rgb8::Blue(dst) * (1.0f - alpha));
			uint8_t outA = static_cast<uint8_t>(srcA * alpha + Rgb8::Alpha(dst) * (1.0f - alpha));

			frameBuffer[y * SurfaceWidth + x] = Rgb8::Color(outA, outR, outG, outB);
		}

		void PixelBlendAdd(const Rgb8::color_t color, const int16_t x, const int16_t y) final
		{
			if (frameBuffer.size() != SurfaceWidth * SurfaceHeight)
				return;

			if (x < 0 || x >= SurfaceWidth || y < 0 || y >= SurfaceHeight)
				return;
			Rgb8::color_t src = color;
			Rgb8::color_t dst = frameBuffer[y * SurfaceWidth + x];

			uint8_t outR = static_cast<uint8_t>(std::min<uint16_t>(Rgb8::Red(src) + Rgb8::Red(dst), 255));
			uint8_t outG = static_cast<uint8_t>(std::min<uint16_t>(Rgb8::Green(src) + Rgb8::Green(dst), 255));
			uint8_t outB = static_cast<uint8_t>(std::min<uint16_t>(Rgb8::Blue(src) + Rgb8::Blue(dst), 255));

			frameBuffer[y * SurfaceWidth + x] = Rgb8::Color(outR, outG, outB);
		}

		void PixelBlendSubtract(const Rgb8::color_t color, const int16_t x, const int16_t y) final
		{
			if (frameBuffer.size() != SurfaceWidth * SurfaceHeight)
				return;

			if (x < 0 || x >= SurfaceWidth || y < 0 || y >= SurfaceHeight)
				return;
			Rgb8::color_t src = color;
			Rgb8::color_t dst = frameBuffer[y * SurfaceWidth + x];

			uint8_t outR = static_cast<uint8_t>(std::max<int16_t>(Rgb8::Red(dst) - Rgb8::Red(src), 0));
			uint8_t outG = static_cast<uint8_t>(std::max<int16_t>(Rgb8::Green(dst) - Rgb8::Green(src), 0));
			uint8_t outB = static_cast<uint8_t>(std::max<int16_t>(Rgb8::Blue(dst) - Rgb8::Blue(src), 0));

			frameBuffer[y * SurfaceWidth + x] = Rgb8::Color(outR, outG, outB);
		}

		void PixelBlendMultiply(const Rgb8::color_t color, const int16_t x, const int16_t y) final
		{
			if (frameBuffer.size() != SurfaceWidth * SurfaceHeight)
				return;

			if (x < 0 || x >= SurfaceWidth || y < 0 || y >= SurfaceHeight)
				return;
			Rgb8::color_t src = color;
			Rgb8::color_t dst = frameBuffer[y * SurfaceWidth + x];

			uint8_t outR = static_cast<uint8_t>((Rgb8::Red(src) * Rgb8::Red(dst)) / 255);
			uint8_t outG = static_cast<uint8_t>((Rgb8::Green(src) * Rgb8::Green(dst)) / 255);
			uint8_t outB = static_cast<uint8_t>((Rgb8::Blue(src) * Rgb8::Blue(dst)) / 255);

			frameBuffer[y * SurfaceWidth + x] = Rgb8::Color(outR, outG, outB);
		}

		void PixelBlendScreen(const Rgb8::color_t color, const int16_t x, const int16_t y) final
		{
			if (frameBuffer.size() != SurfaceWidth * SurfaceHeight)
				return;

			if (x < 0 || x >= SurfaceWidth || y < 0 || y >= SurfaceHeight)
				return;
			Rgb8::color_t src = color;
			Rgb8::color_t dst = frameBuffer[y * SurfaceWidth + x];

			uint8_t outR = static_cast<uint8_t>(255 - ((255 - Rgb8::Red(src)) * (255 - Rgb8::Red(dst)) / 255));
			uint8_t outG = static_cast<uint8_t>(255 - ((255 - Rgb8::Green(src)) * (255 - Rgb8::Green(dst)) / 255));
			uint8_t outB = static_cast<uint8_t>(255 - ((255 - Rgb8::Blue(src)) * (255 - Rgb8::Blue(dst)) / 255));

			frameBuffer[y * SurfaceWidth + x] = Rgb8::Color(outR, outG, outB);
		}

	private:
		static bool PointInTriangle(const int16_t x, const int16_t y, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2, const int16_t x3, const int16_t y3)
		{
			int32_t d1, d2, d3;
			bool has_neg, has_pos;
			d1 = int32_t(x - x2) * (y1 - y2) - int32_t(x1 - x2) * (y - y2);
			d2 = int32_t(x - x3) * (y2 - y3) - int32_t(x2 - x3) * (y - y3);
			d3 = int32_t(x - x1) * (y3 - y1) - int32_t(x3 - x1) * (y - y1);
			has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
			has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);
			return !(has_neg && has_pos);
		}
	};
}

#endif