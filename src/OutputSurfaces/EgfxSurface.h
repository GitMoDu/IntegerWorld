#ifndef _INTEGER_WORLD_EGFX_OUTPUT_SURFACE_h
#define _INTEGER_WORLD_EGFX_OUTPUT_SURFACE_h

#include "../Framework/Interface.h"
#include <EgfxCore.h>

namespace IntegerWorld
{
	namespace EgfxSurface
	{
		namespace Abstract
		{
			enum class StateEnum : uint8_t
			{
				Disabled,
				WaitingForDraw,
				DrawLocked,
				DrawDone
			};

			class AbstractSurface : public IOutputSurface
			{
			protected:
				StateEnum State = StateEnum::Disabled;

			public:
				int16_t OffsetX = 0;
				int16_t OffsetY = 0;
				int16_t SizeX = -1;
				int16_t SizeY = -1;

			protected:
				virtual void OnDrawLocked() {}

			public:
				AbstractSurface() : IOutputSurface()
				{
				}

			protected:
				static constexpr Egfx::rgb_color_t GetNativeColor(const Rgb8::color_t shaderColor)
				{
					return Egfx::Rgb::Color(shaderColor);
				}

			public:
				bool StartSurface()
				{
					if (SizeX == 0 || SizeY == 0)
					{
						// Invalid size.
						return false;
					}

					SetEnabled(true);

					return true;
				}

				void StopSurface()
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
				bool DrawCall(Egfx::IFrameBuffer* frame, const uint32_t frameTime, const uint16_t frameCounter)
				{
					switch (State)
					{
					case StateEnum::WaitingForDraw:
						if (frame != nullptr)
						{
							State = StateEnum::DrawLocked; // Surface can now be drawn to.
						}
						else
						{
							State = StateEnum::WaitingForDraw;// Engine is done and renderer can move on to the next layer/frame.
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
				void SetEnabled(const bool enabled)
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
				bool IsEnabled() const
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
			};
		}

		template<typename FramebufferType>
		class FramebufferTemplate final : public Abstract::AbstractSurface
		{
		private:
			FramebufferType& Framebuffer;

		public:
			FramebufferTemplate(FramebufferType& framebuffer)
				: Abstract::AbstractSurface()
				, Framebuffer(framebuffer)
			{
			}

			void GetSurfaceDimensions(int16_t& width, int16_t& height, uint8_t& colorDepth)
			{
				width = FramebufferType::FrameWidth;
				height = FramebufferType::FrameHeight;
				colorDepth = FramebufferType::ColorDepth;

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

			void PixelBlendAlpha(const Rgb8::color_t color, const int16_t x, const int16_t y)
			{
				Framebuffer.PixelBlendAlpha(GetNativeColor(color), x + OffsetX, y + OffsetY, Rgb8::Alpha(color));
			}

			void PixelBlendAdd(const Rgb8::color_t color, const int16_t x, const int16_t y)
			{
				Framebuffer.PixelBlendAdd(GetNativeColor(color), x + OffsetX, y + OffsetY);
			}

			void PixelBlendSubtract(const Rgb8::color_t color, const int16_t x, const int16_t y)
			{
				Framebuffer.PixelBlendSubtract(GetNativeColor(color), x + OffsetX, y + OffsetY);
			}

			void PixelBlendMultiply(const Rgb8::color_t color, const int16_t x, const int16_t y)
			{
				Framebuffer.PixelBlendMultiply(GetNativeColor(color), x + OffsetX, y + OffsetY);
			}

			void PixelBlendScreen(const Rgb8::color_t color, const int16_t x, const int16_t y)
			{
				Framebuffer.PixelBlendScreen(GetNativeColor(color), x + OffsetX, y + OffsetY);
			}

			void Pixel(const Rgb8::color_t color, const int16_t x, const int16_t y)
			{
				Framebuffer.Pixel(GetNativeColor(color), x + OffsetX, y + OffsetY);
			}

			void Line(const Rgb8::color_t color, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2)
			{
				Framebuffer.Line(GetNativeColor(color), x1 + OffsetX, y1 + OffsetY, x2 + OffsetX, y2 + OffsetY);
			}

			void TriangleFill(const Rgb8::color_t color, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2, const int16_t x3, const int16_t y3)
			{
				Framebuffer.TriangleFill(GetNativeColor(color), x1 + OffsetX, y1 + OffsetY, x2 + OffsetX, y2 + OffsetY, x3 + OffsetX, y3 + OffsetY);
			}

			void RectangleFill(const Rgb8::color_t color, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2)
			{
				Framebuffer.RectangleFill(GetNativeColor(color), x1 + OffsetX, y1 + OffsetY, x2 + OffsetX, y2 + OffsetY);
			}
		};

		template<typename FramebufferType>
		class FramebufferPointerTemplate final : public Abstract::AbstractSurface
		{
		private:
			FramebufferType* Framebuffer = nullptr;

		public:
			FramebufferPointerTemplate(FramebufferType* framebuffer = nullptr)
				: Abstract::AbstractSurface()
				, Framebuffer(framebuffer)
			{
			}

			virtual bool StartSurface() override
			{
				if (Framebuffer == nullptr)
				{
					Serial.println("EgfxSurface: No framebuffer set!");
					return false;
				}
				else
				{
					return Abstract::AbstractSurface::StartSurface();
				}
			}

			void SetBuffer(FramebufferType& framebuffer)
			{
				Framebuffer = &framebuffer;
			}

			void GetSurfaceDimensions(int16_t& width, int16_t& height, uint8_t& colorDepth)
			{
				width = FramebufferType::FrameWidth;
				height = FramebufferType::FrameHeight;
				colorDepth = FramebufferType::ColorDepth;

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

			void PixelBlendAlpha(const Rgb8::color_t color, const int16_t x, const int16_t y)
			{
				Framebuffer->PixelBlendAlpha(GetNativeColor(color), x + OffsetX, y + OffsetY, Rgb8::Alpha(color));
			}

			void PixelBlendAdd(const Rgb8::color_t color, const int16_t x, const int16_t y)
			{
				Framebuffer->PixelBlendAdd(GetNativeColor(color), x + OffsetX, y + OffsetY);
			}

			void PixelBlendSubtract(const Rgb8::color_t color, const int16_t x, const int16_t y)
			{
				Framebuffer->PixelBlendSubtract(GetNativeColor(color), x + OffsetX, y + OffsetY);
			}

			void PixelBlendMultiply(const Rgb8::color_t color, const int16_t x, const int16_t y) {
				Framebuffer->PixelBlendMultiply(GetNativeColor(color), x + OffsetX, y + OffsetY);
			}

			void PixelBlendScreen(const Rgb8::color_t color, const int16_t x, const int16_t y)
			{
				Framebuffer->PixelBlendScreen(GetNativeColor(color), x + OffsetX, y + OffsetY);
			}

			void Pixel(const Rgb8::color_t color, const int16_t x, const int16_t y)
			{
				Framebuffer->Pixel(GetNativeColor(color), x + OffsetX, y + OffsetY);
			}

			void Line(const Rgb8::color_t color, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2)
			{
				Framebuffer->Line(GetNativeColor(color), x1 + OffsetX, y1 + OffsetY, x2 + OffsetX, y2 + OffsetY);
			}

			void TriangleFill(const Rgb8::color_t color, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2, const int16_t x3, const int16_t y3)
			{
				Framebuffer->TriangleFill(GetNativeColor(color), x1 + OffsetX, y1 + OffsetY, x2 + OffsetX, y2 + OffsetY, x3 + OffsetX, y3 + OffsetY);
			}

			void RectangleFill(const Rgb8::color_t color, const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2)
			{
				Framebuffer->RectangleFill(GetNativeColor(color), x1 + OffsetX, y1 + OffsetY, x2 + OffsetX, y2 + OffsetY);
			}
		};
	}
}
#endif