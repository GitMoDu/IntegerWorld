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
				return (Rgb8::R(shaderColor) >= ColorThreshold
					|| Rgb8::G(shaderColor) >= ColorThreshold
					|| Rgb8::B(shaderColor) >= ColorThreshold) * 1;
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
				return (uint16_t(Rgb8::R(shaderColor) >> 3) << 11)
					| (uint16_t(Rgb8::G(shaderColor) >> 2) << 5)
					| (uint8_t(Rgb8::B(shaderColor) >> 3));
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
			TemplateAdafruitFramebufferSurface(ScreenType& display, const uint32_t targetPeriod = 10000)
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
		};
	}
}
#endif

