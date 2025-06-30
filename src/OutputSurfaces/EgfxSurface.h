#ifndef _INTEGER_EGFX_OUTPUT_SURFACE_h
#define _INTEGER_EGFX_OUTPUT_SURFACE_h

#include "../Framework/Interface.h"
#include <ArduinoGraphicsCore.h>

namespace IntegerWorld
{
	namespace EgfxSurface
	{
		/// <summary>
		/// EGFX full frame layer as Output Surface.
		/// Translates render engine draw calls into direct Framebuffer draws.
		/// Synchronizes Renderer with Egfx indirect layer drawing.
		///// Template framebuffer interface defaults to generic IFrameBuffer but can be specialized for optimization.
		/// </summary>
		class FramebufferSurface : public IOutputSurface, public Egfx::IFrameDraw
		{
		private:
			enum class StateEnum : uint8_t
			{
				Disabled,
				WaitingForDraw,
				DrawLocked,
				DrawDone
			};

			StateEnum State = StateEnum::Disabled;

		private:
			Egfx::IFrameBuffer* Framebuffer = nullptr;

		public:
			int16_t OffsetX = 0;
			int16_t OffsetY = 0;
			int16_t SizeX = -1;
			int16_t SizeY = -1;

		public:
			FramebufferSurface()
				: IOutputSurface()
				, Egfx::IFrameDraw()
			{
			}

#if defined(ARDUINO)
			void PrintName(Print& serial)
			{
				serial.print(F("EGFX Framebuffer"));
			}
#endif

		public:
			bool StartSurface() final
			{
				SetEnabled(true);

				return true;
			}

			void StopSurface() final
			{
				SetEnabled(false);
			}

		public://IFrameDraw callbacks
			/// <summary>
			/// Recurrent call for drawing. At least one draw call per frame.
			/// </summary>
			/// <param name="frameTime">Current frame time in microseconds.</param>
			/// <param name="frameCounter">Current frame being rendered.</param>
			/// <returns>True when all drawing is done for this frame.</returns>
			bool DrawCall(Egfx::IFrameBuffer* frame, const uint32_t frameTime, const uint16_t frameCounter) final
			{
				switch (State)
				{
				case StateEnum::WaitingForDraw:
					if (frame != nullptr)
					{
						Framebuffer = frame;
						State = StateEnum::DrawLocked; // Surface can now be drawn to.
					}
					return false;
				case StateEnum::DrawLocked:
					return false; // Still doing 3d rendering/rasterizing.
				case StateEnum::DrawDone:
					State = StateEnum::WaitingForDraw;// Engine is done and renderer can move on to the next layer/frame.
				case StateEnum::Disabled:
				default:
					return true;
				}
			}

			/// <summary>
			/// Flag layer as enabled/disabled to skip rendering.
			/// </summary>
			/// <param name="enabled"></param>
			void SetEnabled(const bool enabled) final
			{
				if (enabled)
				{
					switch (State)
					{
					case StateEnum::DrawDone:
					case StateEnum::DrawLocked:
					case StateEnum::WaitingForDraw:
						// Already enabled.
						break;
					case StateEnum::Disabled:
					default:
						// Loops around at least 1 frame to ensure a clean layer drawing.
						State = StateEnum::DrawDone;
						break;
					}
				}
				else
				{
					State = StateEnum::Disabled;
				}
			}

			/// <summary>
			/// </summary>
			/// <param name="enabled">Is layer enabled/disabled.</param>
			bool IsEnabled() const final
			{
				return State != StateEnum::Disabled;
			}

		public:
			/// <summary>
			/// 
			/// </summary>
			/// <returns>True when Renderer is locked and ready to draw.</returns>
			bool IsSurfaceReady() final
			{
				return State == StateEnum::DrawLocked;
			}

			void FlipSurface() final
			{
				switch (State)
				{
				case StateEnum::DrawLocked:
					State = StateEnum::DrawDone; // Engine is done drawing.
					break;
				default:
					break;
				}
			}

			void GetSurfaceDimensions(Egfx::IFrameBuffer* framebuffer, int16_t& width, int16_t& height, uint8_t& colorDepth)
			{
				if (framebuffer != nullptr)
				{
					width = framebuffer->GetFrameWidth();
					height = framebuffer->GetFrameHeight();
					colorDepth = framebuffer->GetColorDepth();

					// Bound limit dimensions for current offset.
					if (OffsetX > 0)
					{
						if (OffsetX >= width)
						{
							width = 0;
						}
						else
						{
							width -= OffsetX;
						}
					}
					if (OffsetY > 0)
					{
						if (OffsetY >= height)
						{
							height = 0;
						}
						else
						{
							height -= OffsetY;
						}
					}

					// Crop dimensions if size is set.
					if (SizeX >= 0)
					{
						width = MinValue(SizeX, width);
					}
					if (SizeY >= 0)
					{
						height = MinValue(SizeY, height);
					}
				}
				else
				{
					width = 1;
					height = 1;
					colorDepth = 1;
				}
			}

			virtual void GetSurfaceDimensions(int16_t& width, int16_t& height, uint8_t& colorDepth) final
			{
				GetSurfaceDimensions(Framebuffer, width, height, colorDepth);
			}

			virtual void Pixel(const color_fraction16_t color, const int16_t x, const int16_t y) final
			{
				Framebuffer->Pixel(GetNativeColor(color), x + OffsetX, y + OffsetY);
			}

			virtual void Line(const color_fraction16_t color, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2) final
			{
				Framebuffer->Line(GetNativeColor(color), x1 + OffsetX, y1 + OffsetY, x2 + OffsetX, y2 + OffsetY);

			}
			virtual void TriangleFill(const color_fraction16_t color, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2, const int16_t x3, const int16_t y3) final
			{
				Framebuffer->TriangleFill(GetNativeColor(color), x1 + OffsetX, y1 + OffsetY, x2 + OffsetX, y2 + OffsetY, x3 + OffsetX, y3 + OffsetY);
			}

			virtual void RectangleFill(const color_fraction16_t color, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2) final
			{
				Framebuffer->RectangleFill(GetNativeColor(color), x1 + OffsetX, y1 + OffsetY, x2 + OffsetX, y2 + OffsetY);
			}

		private:
			/// <summary>
			/// Optmized convertion of fractions (15.5 bit) directly into 8:8:8 color space for native conversion.
			/// </summary>
			/// <param name="shaderColor">Engine native color in RGB ufraction16_t (15 bits).</param>
			/// <returns></returns>
			static constexpr Egfx::rgb_color_t GetNativeColor(const color_fraction16_t shaderColor)
			{
				return Egfx::Rgb::Color(ColorFraction::FractionToRgbValue(shaderColor.r),
					ColorFraction::FractionToRgbValue(shaderColor.g),
					ColorFraction::FractionToRgbValue(shaderColor.b));
			}
		};
	}
}
#endif