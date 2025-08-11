#ifndef _INTEGER_WORLD_ADAFRUIT_SURFACE_h
#define _INTEGER_WORLD_ADAFRUIT_SURFACE_h

#include <IntegerWorld.h>
#include <SPI.h>
#include <Wire.h>

namespace IntegerWorld
{
	namespace Adafruit
	{
#if defined(ARDUINO_ARCH_RP2040)
		using SpiType = SPIClassRP2040;
#else
		using SpiType = SPIClass;
#endif

		enum class SurfaceStateEnum : uint8_t
		{
			Disabled,
			WaitingForDraw,
			DrawLocked
		};

		struct AdafruitMonochromeColorConverter
		{
			static constexpr uint8_t ColorThreshold = UINT8_MAX / 2;

			static constexpr uint8_t GetColorDepth()
			{
				return 1;
			}

			static constexpr uint16_t GetNativeColor(const Rgb8::color_t shaderColor)
			{
				return (Rgb8::Red(shaderColor) >= ColorThreshold
					|| Rgb8::Green(shaderColor) >= ColorThreshold
					|| Rgb8::Blue(shaderColor) >= ColorThreshold) * 1;
			}
		};

		struct Adafruit16BitColorConverter
		{
			static constexpr uint8_t GetColorDepth()
			{
				return 16;
			}

			static constexpr uint16_t GetNativeColor(const Rgb8::color_t shaderColor)
			{
				return (uint16_t(Rgb8::Red(shaderColor) >> 3) << 11)
					| (uint16_t(Rgb8::Green(shaderColor) >> 2) << 5)
					| (uint8_t(Rgb8::Blue(shaderColor) >> 3));
			}
		};

		template<typename ScreenType, typename ColorConverter>
		class AbstractAdafruitSurface : public IOutputSurface
		{
		protected:
			ScreenType& Display;

		public:
			AbstractAdafruitSurface(ScreenType& display)
				: IOutputSurface()
				, Display(display)
			{
			}

		public:
			void GetSurfaceDimensions(int16_t& width, int16_t& height, uint8_t& colorDepth)
			{
				switch (3)
				{
				case 1:
				case 3:
					width = Display.width();
					height = Display.height();
					break;
				default:
					height = Display.width();
					width = Display.height();
					break;
				}
				colorDepth = ColorConverter::GetColorDepth();
			}

			void Pixel(const Rgb8::color_t color, const int16_t x, const int16_t y) final
			{
				Display.drawPixel(x, y, ColorConverter::GetNativeColor(color));
			}

			void Line(const Rgb8::color_t color, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2) final
			{
				if (x1 == x2)
				{
					Display.drawFastVLine(x1, y1, y2 - y1 + 1, ColorConverter::GetNativeColor(color));
				}
				else if (y1 == y2)
				{
					Display.drawFastHLine(x1, y1, x2 - x1 + 1, ColorConverter::GetNativeColor(color));
				}
				else
				{
					Display.drawLine(x1, y1, x2, y2, ColorConverter::GetNativeColor(color));
				}
			}

			void TriangleFill(const Rgb8::color_t color, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2, const int16_t x3, const int16_t y3) final
			{
				Display.fillTriangle(x1, y1, x2, y2, x3, y3, ColorConverter::GetNativeColor(color));
			}

			void RectangleFill(const Rgb8::color_t color, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2) final
			{
				Display.fillRect(x1, y1, x2 - x1, y2 - y1, ColorConverter::GetNativeColor(color));
			}
		};

		template<typename ScreenType, typename ColorConverter>
		class TemplateAdafruitFramebufferSurface : public AbstractAdafruitSurface<ScreenType, ColorConverter>
		{
		private:
			using Base = AbstractAdafruitSurface<ScreenType, ColorConverter>;

		protected:
			using Base::Display;

		public:
			uint32_t TargetPeriod;

		private:
			uint32_t LastPush = 0;
			SurfaceStateEnum State = SurfaceStateEnum::Disabled;

		public:
			TemplateAdafruitFramebufferSurface(ScreenType& display, const uint32_t targetPeriod)
				: Base(display)
				, TargetPeriod(targetPeriod)
			{
			}

		protected:
			virtual bool StartScreen() { return false; }
			virtual void OnDrawStart() {}
			virtual void OnDrawEnd() {}

		public:
			bool StartSurface() final
			{
				if (!StartScreen())
				{
					return false;
				}

				Display.clearDisplay();
				Display.display();
				LastPush = micros() - TargetPeriod;
				State = SurfaceStateEnum::WaitingForDraw;

				return true;
			}

			void StopSurface() final
			{
			}

		public:
			bool IsSurfaceReady() final
			{
				if (State == SurfaceStateEnum::WaitingForDraw
					&& ((micros() - LastPush) > TargetPeriod))
				{
					Display.clearDisplay();
					OnDrawStart();
					State = SurfaceStateEnum::DrawLocked;
				}

				return State == SurfaceStateEnum::DrawLocked;
			}

			void FlipSurface() final
			{
				switch (State)
				{
				case SurfaceStateEnum::DrawLocked:
					LastPush = micros();
					Display.display();
					OnDrawEnd();
					State = SurfaceStateEnum::WaitingForDraw;
					break;
				default:
					break;
				}
			}

		};

		template<typename ScreenType>
		class TemplateAdafruitFramebufferColor16Surface : public TemplateAdafruitFramebufferSurface<ScreenType, Adafruit16BitColorConverter>
		{
		private:
			using Base = TemplateAdafruitFramebufferSurface<ScreenType, Adafruit16BitColorConverter>;
			using ColorConverter = Adafruit16BitColorConverter;

		protected:
			using Base::Display;

		public:
			TemplateAdafruitFramebufferColor16Surface(ScreenType& display, const uint32_t targetPeriod)
				: Base(display, targetPeriod)
			{
			}

			void PixelBlendAlpha(const Rgb8::color_t color, const int16_t x, const int16_t y) final
			{
				uint16_t existingPixel = Display.getPixel(x, y);
				uint16_t newPixel = ColorConverter::GetNativeColor(color);
				uint8_t alpha = Rgb8::Alpha(color);

				// Extract RGB components from RGB565
				uint8_t r0 = (existingPixel >> 11) & 0x1F;
				uint8_t g0 = (existingPixel >> 5) & 0x3F;
				uint8_t b0 = existingPixel & 0x1F;

				uint8_t r1 = (newPixel >> 11) & 0x1F;
				uint8_t g1 = (newPixel >> 5) & 0x3F;
				uint8_t b1 = newPixel & 0x1F;

				// Convert to 8-bit for blending
				uint8_t r = (((r0 << 3) * (255 - alpha)) + ((r1 << 3) * alpha)) >> 8;
				uint8_t g = (((g0 << 2) * (255 - alpha)) + ((g1 << 2) * alpha)) >> 8;
				uint8_t b = (((b0 << 3) * (255 - alpha)) + ((b1 << 3) * alpha)) >> 8;

				// Convert back to RGB565
				uint16_t blended = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
				Display.drawPixel(x, y, blended);
			}

			void PixelBlendAdd(const Rgb8::color_t color, const int16_t x, const int16_t y) final
			{
				uint16_t existingPixel = Display.getPixel(x, y);
				uint16_t newPixel = ColorConverter::GetNativeColor(color);

				uint8_t r = MinValue<uint16_t>(((existingPixel >> 11) & 0x1F) + ((newPixel >> 11) & 0x1F), 31);
				uint8_t g = MinValue<uint16_t>(((existingPixel >> 5) & 0x3F) + ((newPixel >> 5) & 0x3F), 63);
				uint8_t b = MinValue<uint16_t>((existingPixel & 0x1F) + (newPixel & 0x1F), 31);

				uint16_t blended = (r << 11) | (g << 5) | b;
				Display.drawPixel(x, y, blended);
			}

			void PixelBlendSubtract(const Rgb8::color_t color, const int16_t x, const int16_t y) final
			{
				uint16_t existingPixel = Display.getPixel(x, y);
				uint16_t newPixel = ColorConverter::GetNativeColor(color);

				int16_t r = MaxValue<int16_t>(((existingPixel >> 11) & 0x1F) - ((newPixel >> 11) & 0x1F), 0);
				int16_t g = MaxValue<int16_t>(((existingPixel >> 5) & 0x3F) - ((newPixel >> 5) & 0x3F), 0);
				int16_t b = MaxValue<int16_t>((existingPixel & 0x1F) - (newPixel & 0x1F), 0);

				uint16_t blended = (r << 11) | (g << 5) | b;
				Display.drawPixel(x, y, blended);
			}

			void PixelBlendMultiply(const Rgb8::color_t color, const int16_t x, const int16_t y) final
			{
				uint16_t existingPixel = Display.getPixel(x, y);
				uint16_t newPixel = ColorConverter::GetNativeColor(color);

				uint8_t r = (((existingPixel >> 11) & 0x1F) * ((newPixel >> 11) & 0x1F)) >> 5;
				uint8_t g = (((existingPixel >> 5) & 0x3F) * ((newPixel >> 5) & 0x3F)) >> 6;
				uint8_t b = ((existingPixel & 0x1F) * (newPixel & 0x1F)) >> 5;

				uint16_t blended = (r << 11) | (g << 5) | b;
				Display.drawPixel(x, y, blended);
			}

			void PixelBlendScreen(const Rgb8::color_t color, const int16_t x, const int16_t y) final
			{
				uint16_t existingPixel = Display.getPixel(x, y);
				uint16_t newPixel = ColorConverter::GetNativeColor(color);

				uint8_t r0 = (existingPixel >> 11) & 0x1F;
				uint8_t g0 = (existingPixel >> 5) & 0x3F;
				uint8_t b0 = existingPixel & 0x1F;

				uint8_t r1 = (newPixel >> 11) & 0x1F;
				uint8_t g1 = (newPixel >> 5) & 0x3F;
				uint8_t b1 = newPixel & 0x1F;

				uint8_t r = 31 - (((31 - r0) * (31 - r1)) >> 5);
				uint8_t g = 63 - (((63 - g0) * (63 - g1)) >> 6);
				uint8_t b = 31 - (((31 - b0) * (31 - b1)) >> 5);

				uint16_t blended = (r << 11) | (g << 5) | b;
				Display.drawPixel(x, y, blended);
			}
		};

		template<typename ScreenType>
		class TemplateAdafruitFramebufferMonochromeSurface : public TemplateAdafruitFramebufferSurface<ScreenType, AdafruitMonochromeColorConverter>
		{
		private:
			using Base = TemplateAdafruitFramebufferSurface<ScreenType, AdafruitMonochromeColorConverter>;
			using ColorConverter = AdafruitMonochromeColorConverter;

		protected:
			using Base::Display;

		private:
			AlphaRandomDitherer AlphaDitherer{};

		public:
			TemplateAdafruitFramebufferMonochromeSurface(ScreenType& display, const uint32_t targetPeriod)
				: Base(display, targetPeriod)
			{
			}

		public:
			void PixelBlendAlpha(const Rgb8::color_t color, const int16_t x, const int16_t y) final
			{
				const bool existingPixel = Display.getPixel(x, y);
				const bool newPixel = ColorConverter::GetNativeColor(color);

				if (existingPixel != newPixel
					&& AlphaDitherer.Dither(Rgb8::Alpha(color)))
				{
					// Use dithering to decide whether to draw the pixel based on its alpha value.
					Display.drawPixel(x, y, newPixel);
				}
			}

			void PixelBlendAdd(const Rgb8::color_t color, const int16_t x, const int16_t y) final
			{
				const bool existingPixel = Display.getPixel(x, y);
				const bool newPixel = ColorConverter::GetNativeColor(color);

				if (existingPixel != newPixel
					&& AlphaDitherer.Dither(Rgb8::Alpha(color)))
				{
					// Use dithering to decide whether to draw the pixel based on its alpha value.
					Display.drawPixel(x, y, newPixel);
				}
			}

			void PixelBlendSubtract(const Rgb8::color_t color, const int16_t x, const int16_t y) final
			{
				const bool existingPixel = Display.getPixel(x, y);
				const bool newPixel = ColorConverter::GetNativeColor(color);
				if (existingPixel != newPixel
					&& !AlphaDitherer.Dither(Rgb8::Alpha(color)))
				{
					// Approximate subtract blend mode by inverting the color and using reversed dithering.
					Display.drawPixel(x, y, !newPixel);
				}
			}

			void PixelBlendMultiply(const Rgb8::color_t color, const int16_t x, const int16_t y) final
			{
				const bool existingPixel = Display.getPixel(x, y);
				const bool newPixel = ColorConverter::GetNativeColor(color);
				if (existingPixel != newPixel
					&& AlphaDitherer.Dither(Rgb8::Alpha(color)))
				{
					// Approximate multiply blend mode by using dithering to decide whether to draw the pixel based on its alpha value.
					Display.drawPixel(x, y, newPixel);
				}
			}

			void PixelBlendScreen(const Rgb8::color_t color, const int16_t x, const int16_t y) final
			{
				PixelBlendMultiply(color, x, y);
			}
		};

		template<typename ScreenType, typename ColorConverter>
		class TemplateAdafruitDirectDrawSurface : public AbstractAdafruitSurface<ScreenType, ColorConverter>
		{
		private:
			using Base = AbstractAdafruitSurface<ScreenType, ColorConverter>;

		protected:
			using Base::Display;

		public:
			uint32_t TargetPeriod;

		private:
			AlphaRandomDitherer AlphaDitherer{};
			uint32_t LastPush = 0;
			SurfaceStateEnum State = SurfaceStateEnum::Disabled;

		public:
			TemplateAdafruitDirectDrawSurface(ScreenType& display, const uint32_t targetPeriod)
				: Base(display)
				, TargetPeriod(targetPeriod)
			{
			}

		protected:
			virtual bool StartScreen() { return false; }
			virtual void OnDrawStart() {}
			virtual void OnDrawEnd() {}

		public:
			bool StartSurface() final
			{
				if (!StartScreen())
				{
					return false;
				}

				Display.startWrite();
				Display.fillScreen(0);
				Display.endWrite();
				LastPush = micros() - TargetPeriod;
				State = SurfaceStateEnum::WaitingForDraw;

				return true;
			}

			void StopSurface() final
			{
				Display.endWrite();
			}

		public:
			bool IsSurfaceReady() final
			{
				if (State == SurfaceStateEnum::WaitingForDraw
					&& ((micros() - LastPush) > TargetPeriod))
				{
					OnDrawStart();
					Display.startWrite();
					Display.fillScreen(0);
					State = SurfaceStateEnum::DrawLocked;
				}

				return State == SurfaceStateEnum::DrawLocked;
			}

			void FlipSurface() final
			{
				switch (State)
				{
				case SurfaceStateEnum::DrawLocked:
					LastPush = micros();
					Display.endWrite();
					OnDrawEnd();
					State = SurfaceStateEnum::WaitingForDraw;
					break;
				default:
					break;
				}
			}

		public:
			void PixelBlendAlpha(const Rgb8::color_t color, const int16_t x, const int16_t y) final
			{
				// Use dithering to decide whether to draw the pixel based on its alpha value.
				if (AlphaDitherer.Dither(Rgb8::Alpha(color)))
				{
					Display.drawPixel(x, y, ColorConverter::GetNativeColor(color));
				}
			}

			void PixelBlendAdd(const Rgb8::color_t color, const int16_t x, const int16_t y) final
			{
				// Use Multiply blend mode for add in direct draw mode.
				PixelBlendMultiply(color, x, y);
			}

			void PixelBlendSubtract(const Rgb8::color_t color, const int16_t x, const int16_t y) final
			{
				// Approximate subtract blend mode by inverting the color and using reversed dithering.
				if (!AlphaDitherer.Dither(INT8_MAX))
				{
					Display.drawPixel(x, y, ~ColorConverter::GetNativeColor(color));
				}
			}

			void PixelBlendMultiply(const Rgb8::color_t color, const int16_t x, const int16_t y) final
			{
				// Use 50/50 dithering to decide whether to draw the pixel based on its alpha value.
				if (AlphaDitherer.Dither(INT8_MAX))
				{
					Display.drawPixel(x, y, ColorConverter::GetNativeColor(color));
				}
			}

			void PixelBlendScreen(const Rgb8::color_t color, const int16_t x, const int16_t y) final
			{
				// Use Multiply blend mode for screen in direct draw mode.
				PixelBlendMultiply(color, x, y);
			}
		};
	}
}
#endif

