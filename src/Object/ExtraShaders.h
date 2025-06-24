#ifndef _INTEGER_WORLD_EXTRA_SHADERS_h
#define _INTEGER_WORLD_EXTRA_SHADERS_h

#include "Shader.h"

namespace IntegerWorld
{
	struct TriangleGradientFragmentShader : IFragmentShader<triangle_fragment_t>
	{
	private:
		struct GradientShaderFunctor : AbstractPixelShader::AbstractTriangleFunctor
		{
			color_fraction16_t ColorA{ UFRACTION16_1X, 0, 0 }; // Vertex A color (red)
			color_fraction16_t ColorB{ 0, UFRACTION16_1X, 0 }; // Vertex B color (green)
			color_fraction16_t ColorC{ 0, 0, UFRACTION16_1X }; // Vertex C color (blue)

			bool operator()(color_fraction16_t& color, const int16_t x, const int16_t y)
			{
				// Barycentric coordinates
				int32_t w0 = (int32_t(BmCy) * (x - Cx)) + (int32_t(CmBx) * (y - Cy));
				int32_t w1 = (int32_t(CmAy) * (x - Cx)) + (int32_t(AmCx) * (y - Cy));
				int32_t w2 = TriangleArea - w0 - w1;

				// Clamp weights to [0, TriangleArea] to avoid edge artifacts
				if (w0 < 0) w0 = 0; else if (w0 > TriangleArea) w0 = TriangleArea;
				if (w1 < 0) w1 = 0; else if (w1 > TriangleArea) w1 = TriangleArea;
				if (w2 < 0) w2 = 0; else if (w2 > TriangleArea) w2 = TriangleArea;


				// Interpolate each color channel.
				color.r = ((w0 * ColorA.r) + (w1 * ColorB.r) + (w2 * ColorC.r)) / TriangleArea;
				color.g = ((w0 * ColorA.g) + (w1 * ColorB.g) + (w2 * ColorC.g)) / TriangleArea;
				color.b = ((w0 * ColorA.b) + (w1 * ColorB.b) + (w2 * ColorC.b)) / TriangleArea;

				return true;
			}
		} GradientShader{};

	public:
		void SetColors(const color_fraction16_t& colorA, const color_fraction16_t& colorB, const color_fraction16_t& colorC)
		{
			GradientShader.ColorA = colorA;
			GradientShader.ColorB = colorB;
			GradientShader.ColorC = colorC;
		}

		void FragmentShade(WindowRasterizer& rasterizer, const triangle_fragment_t& fragment, ISceneShader* sceneShader) final
		{
			FragmentShade(rasterizer, fragment);
		}

		void FragmentShade(WindowRasterizer& rasterizer, const triangle_fragment_t& fragment) final
		{
			if (GradientShader.SetFragmentData(fragment))
			{
				rasterizer.RasterTriangle(fragment.triangleScreenA, fragment.triangleScreenB, fragment.triangleScreenC, GradientShader);
			}
		}
	};

	struct TriangleBarycentricStripeFragmentShader : IFragmentShader<triangle_fragment_t>
	{
	private:
		struct StripeShaderFunctor : AbstractPixelShader::AbstractTriangleFunctor
		{
			color_fraction16_t ColorA = ColorFraction::COLOR_BLACK;
			color_fraction16_t ColorB = ColorFraction::COLOR_BLACK;
			uint8_t StripeCount = 8;

			bool operator()(color_fraction16_t& color, const int16_t x, const int16_t y)
			{
				const int32_t w0 = (int32_t(BmCy) * (x - Cx)) + (int32_t(CmBx) * (y - Cy));
				const int32_t w1 = (int32_t(CmAy) * (x - Cx)) + (int32_t(AmCx) * (y - Cy));
				const int32_t w2 = TriangleArea - w0 - w1;

				// Use w0 for stripes, quantized to StripeCount bands
				int32_t band = ((w0 * StripeCount) / TriangleArea) & 1;
				color = (band == 0) ? ColorA : ColorB;
				return true;
			}
		} StripeShader{};

	public:
		world_position_normal_shade_t Shade{};

	public:
		static constexpr material_t shine{ 0,0,UFRACTION8_1X, UFRACTION8_1X / 1 };

		void FragmentShade(WindowRasterizer& rasterizer, const triangle_fragment_t& fragment, ISceneShader* sceneShader) final
		{
			StripeShader.ColorA = fragment.color;
			StripeShader.ColorB = fragment.color;
			Shade.normalWorld = fragment.normal;
			Shade.positionWorld = fragment.world;
			sceneShader->Shade(StripeShader.ColorA, fragment.material, Shade);
			sceneShader->Shade(StripeShader.ColorB, shine, Shade);
			if (StripeShader.SetFragmentData(fragment))
			{
				rasterizer.RasterTriangle(fragment.triangleScreenA, fragment.triangleScreenB, fragment.triangleScreenC, StripeShader);
			}
		}

		void FragmentShade(WindowRasterizer& rasterizer, const triangle_fragment_t& fragment) final
		{
			StripeShader.ColorA = ColorFraction::COLOR_WHITE;
			StripeShader.ColorB = fragment.color;
			if (StripeShader.SetFragmentData(fragment))
			{
				rasterizer.RasterTriangle(fragment.triangleScreenA, fragment.triangleScreenB, fragment.triangleScreenC, StripeShader);
			}
		}
	};

	struct TriangleBarycentricCheckerFragmentShader : IFragmentShader<triangle_fragment_t>
	{
	private:
		struct SquareShaderFunctor : AbstractPixelShader::AbstractTriangleFunctor
		{
			color_fraction16_t ColorA = ColorFraction::COLOR_WHITE;
			color_fraction16_t ColorB = ColorFraction::COLOR_BLACK;

			uint8_t CheckerSize = 4;

			bool operator()(color_fraction16_t& color, const int16_t x, const int16_t y)
			{
				const int32_t w0 = (int32_t(BmCy) * (x - Cx)) + (int32_t(CmBx) * (y - Cy));
				const int32_t w1 = (int32_t(CmAy) * (x - Cx)) + (int32_t(AmCx) * (y - Cy));

				int32_t i = (w0 * CheckerSize) / TriangleArea;
				int32_t j = (w1 * CheckerSize) / TriangleArea;
				color = ((i ^ j) & 1) ? ColorA : ColorB;
				return true;
			}
		};

		struct DistortedShaderFunctor : AbstractPixelShader::AbstractTriangleFunctor
		{
			color_fraction16_t ColorA = ColorFraction::COLOR_WHITE;
			color_fraction16_t ColorB = ColorFraction::COLOR_BLACK;

			uint8_t CheckerSize = 6;

			bool operator()(color_fraction16_t& color, const int16_t x, const int16_t y)
			{
				const int32_t w0 = (int32_t(BmCy) * (x - Cx)) + (int32_t(CmBx) * (y - Cy));
				const int32_t w1 = (int32_t(CmAy) * (x - Cx)) + (int32_t(AmCx) * (y - Cy));
				const int32_t w2 = TriangleArea - w0 - w1;

				// Clamp to [0, TriangleArea]
				int32_t cw0 = w0 < 0 ? 0 : (w0 > TriangleArea ? TriangleArea : w0);
				int32_t cw1 = w1 < 0 ? 0 : (w1 > TriangleArea ? TriangleArea : w1);
				int32_t cw2 = w2 < 0 ? 0 : (w2 > TriangleArea ? TriangleArea : w2);

				// Map to (u,v) in [0,CheckerSize)
				int32_t sum01 = cw0 + cw1;
				int32_t sum02 = cw0 + cw2;
				int32_t iu = (sum01 == 0) ? 0 : (cw1 * CheckerSize) / sum01;
				int32_t iv = (sum02 == 0) ? 0 : (cw2 * CheckerSize) / sum02;

				color = ((iu ^ iv) & 1) ? ColorA : ColorB;
				return true;
			}
		};

		DistortedShaderFunctor CheckerShader{};

	public:
		color_fraction16_t ColorA{ UFRACTION16_1X, UFRACTION16_1X, UFRACTION16_1X };

		color_fraction16_t ColorB{ 0, 0, 0 };

		void SetColors(const color_fraction16_t& colorA, const color_fraction16_t& colorB, uint8_t size = 8)
		{
			CheckerShader.ColorA = colorA;
			CheckerShader.ColorB = colorB;
			CheckerShader.CheckerSize = size;
		}

		void FragmentShade(WindowRasterizer& rasterizer, const triangle_fragment_t& fragment, ISceneShader* sceneShader) final
		{
			FragmentShade(rasterizer, fragment);
		}
		void FragmentShade(WindowRasterizer& rasterizer, const triangle_fragment_t& fragment) final
		{
			if (CheckerShader.SetFragmentData(fragment))
			{
				rasterizer.RasterTriangle(fragment.triangleScreenA, fragment.triangleScreenB, fragment.triangleScreenC, CheckerShader);
			}
		}
	};

	struct TriangleBarycentricBandFragmentShader : IFragmentShader<triangle_fragment_t>
	{
	private:
		struct BandShaderFunctor : AbstractPixelShader::AbstractTriangleFunctor
		{
			color_fraction16_t ColorA{ UFRACTION16_1X, 0, 0 };
			color_fraction16_t ColorB{ 0, UFRACTION16_1X, 0 };
			color_fraction16_t ColorC{ 0, 0, UFRACTION16_1X };

			bool operator()(color_fraction16_t& color, const int16_t x, const int16_t y)
			{
				const int32_t w0 = (int32_t(BmCy) * (x - Cx)) + (int32_t(CmBx) * (y - Cy));
				const int32_t w1 = (int32_t(CmAy) * (x - Cx)) + (int32_t(AmCx) * (y - Cy));
				const int32_t w2 = TriangleArea - w0 - w1;

				// Assign color based on which barycentric coordinate is largest
				if (w0 >= w1 && w0 >= w2)
					color = ColorA;
				else if (w1 >= w0 && w1 >= w2)
					color = ColorB;
				else
					color = ColorC;
				return true;
			}
		} BandShader{};

	public:
		void SetColors(const color_fraction16_t& colorA, const color_fraction16_t& colorB, const color_fraction16_t& colorC)
		{
			BandShader.ColorA = colorA;
			BandShader.ColorB = colorB;
			BandShader.ColorC = colorC;
		}

		void FragmentShade(WindowRasterizer& rasterizer, const triangle_fragment_t& fragment, ISceneShader* sceneShader) final
		{
			FragmentShade(rasterizer, fragment);
		}
		void FragmentShade(WindowRasterizer& rasterizer, const triangle_fragment_t& fragment) final
		{
			if (BandShader.SetFragmentData(fragment))
			{
				rasterizer.RasterTriangle(fragment.triangleScreenA, fragment.triangleScreenB, fragment.triangleScreenC, BandShader);
			}
		}
	};
}
#endif