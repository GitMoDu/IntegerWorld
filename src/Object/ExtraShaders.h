#ifndef _INTEGER_WORLD_EXTRA_SHADERS_h
#define _INTEGER_WORLD_EXTRA_SHADERS_h

#include "Shader.h"

namespace IntegerWorld
{
	struct TriangleDitherFragmentShader : IFragmentShader<triangle_fragment_t>
	{
	private:
		struct MonochromeDitherShaderFunctor
		{
		private:
			static constexpr uint8_t bayer4x4[4][4] = {
										   { 0, 8, 2, 10 },
										   { 12, 4, 14, 6 },
										   { 3, 11, 1, 9 },
										   { 15, 7, 13, 5 } };

		private:
			uint8_t Intensity = 0;

		public:
			void SetColor(const Rgb8::color_t color)
			{

				const ufraction16_t gray = ((uint32_t(Rgb8::Red(color)) * 299) + (uint32_t(Rgb8::Green(color)) * 587) + (uint32_t(Rgb8::Blue(color)) * 114)) / 1000;
				Intensity = Fraction::Scale(gray, uint8_t(15));
			}

		public:
			bool operator()(Rgb8::color_t& color, const int16_t x, const int16_t y)
			{
				const uint8_t threshold = bayer4x4[y & 3][x & 3];

				if (Intensity >= threshold)
				{
					color = Rgb8::WHITE;
				}
				else
				{
					color = Rgb8::BLACK;
				}

				return true;
			}
		} MonochromeDitherShader{};

		Rgb8::color_t FragmentColor{};

	public:
		void FragmentShade(WindowRasterizer& rasterizer, const triangle_fragment_t& fragment, ISceneShader* sceneShader) final
		{
			FragmentColor = fragment.color;
			if (sceneShader != nullptr)
				sceneShader->Shade(FragmentColor, fragment.material);
			MonochromeDitherShader.SetColor(FragmentColor);
			rasterizer.RasterTriangle(fragment.triangleScreenA, fragment.triangleScreenB, fragment.triangleScreenC, MonochromeDitherShader);
		}
	};

	struct TriangleGradientFragmentShader : IFragmentShader<triangle_fragment_t>
	{
	private:
		struct GradientShaderFunctor : AbstractPixelShader::AbstractTriangleFunctor
		{
			scene_shade_t Shade{};
			const material_t* material = nullptr;
			//const triangle_fragment_t* fragment = nullptr;
			ISceneShader* sceneShader = nullptr;
			Rgb8::color_t ColorA = Rgb8::RED; // Vertex A color (red)
			Rgb8::color_t ColorB = Rgb8::GREEN; // Vertex B color (green)
			Rgb8::color_t ColorC = Rgb8::BLUE; // Vertex C color (blue)

			bool operator()(Rgb8::color_t& color, const int16_t x, const int16_t y)
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
				color = Rgb8::Color(
					((w0 * Rgb8::Red(ColorA)) + (w1 * Rgb8::Red(ColorB)) + (w2 * Rgb8::Red(ColorC))) / TriangleArea,
					((w0 * Rgb8::Green(ColorA)) + (w1 * Rgb8::Green(ColorB)) + (w2 * Rgb8::Green(ColorC))) / TriangleArea,
					((w0 * Rgb8::Blue(ColorA)) + (w1 * Rgb8::Blue(ColorB)) + (w2 * Rgb8::Blue(ColorC))) / TriangleArea);

				if (sceneShader != nullptr)
				{
					sceneShader->Shade(color, *material, Shade);
				}

				return true;
			}
		} GradientShader{};

	public:
		void SetColors(const Rgb8::color_t colorA, const Rgb8::color_t colorB, const Rgb8::color_t colorC)
		{
			GradientShader.ColorA = colorA;
			GradientShader.ColorB = colorB;
			GradientShader.ColorC = colorC;
		}

		void FragmentShade(WindowRasterizer& rasterizer, const triangle_fragment_t& fragment, ISceneShader* sceneShader) final
		{
			GradientShader.material = &fragment.material;
			GradientShader.Shade.normal = fragment.normalWorld;
			GradientShader.Shade.position = fragment.world;
			GradientShader.sceneShader = sceneShader;
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
			Rgb8::color_t ColorA = Rgb8::BLACK;
			Rgb8::color_t ColorB = Rgb8::BLACK;
			uint8_t StripeCount = 8;

			bool operator()(Rgb8::color_t& color, const int16_t x, const int16_t y)
			{
				const int32_t w0 = (int32_t(BmCy) * (x - Cx)) + (int32_t(CmBx) * (y - Cy));

				// Use w0 for stripes, quantized to StripeCount bands
				int32_t band = ((w0 * StripeCount) / TriangleArea) & 1;
				color = (band == 0) ? ColorA : ColorB;
				return true;
			}
		} StripeShader{};

	public:
		scene_shade_t Shade{};

	public:
		static constexpr material_t shine{ 0,0,UFRACTION8_1X, UFRACTION8_1X / 1 };

		void FragmentShade(WindowRasterizer& rasterizer, const triangle_fragment_t& fragment, ISceneShader* sceneShader) final
		{
			StripeShader.ColorA = Rgb8::WHITE;
			StripeShader.ColorB = fragment.color;
			Shade.normal = fragment.normalWorld;
			Shade.position = fragment.world;
			if (sceneShader != nullptr)
			{
				sceneShader->Shade(StripeShader.ColorA, fragment.material, Shade);
				sceneShader->Shade(StripeShader.ColorB, shine, Shade);
			}
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
			Rgb8::color_t ColorA = Rgb8::WHITE;
			Rgb8::color_t ColorB = Rgb8::BLACK;

			uint8_t CheckerSize = 4;

			bool operator()(Rgb8::color_t& color, const int16_t x, const int16_t y)
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
			Rgb8::color_t ColorA = Rgb8::WHITE;
			Rgb8::color_t ColorB = Rgb8::BLACK;

			uint8_t CheckerSize = 2;

			bool operator()(Rgb8::color_t& color, const int16_t x, const int16_t y)
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

		SquareShaderFunctor CheckerShader{};

	public:
		Rgb8::color_t ColorA = Rgb8::WHITE;

		Rgb8::color_t ColorB = Rgb8::BLACK;

		void SetColors(const Rgb8::color_t colorA, const Rgb8::color_t colorB, uint8_t size = 8)
		{
			CheckerShader.ColorA = colorA;
			CheckerShader.ColorB = colorB;
			CheckerShader.CheckerSize = size;
		}

		void FragmentShade(WindowRasterizer& rasterizer, const triangle_fragment_t& fragment, ISceneShader* sceneShader) final
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
			Rgb8::color_t ColorA = Rgb8::RED;
			Rgb8::color_t ColorB = Rgb8::GREEN;
			Rgb8::color_t ColorC = Rgb8::BLUE;

			bool operator()(Rgb8::color_t& color, const int16_t x, const int16_t y)
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
		void SetColors(const Rgb8::color_t colorA, const Rgb8::color_t colorB, const Rgb8::color_t colorC)
		{
			BandShader.ColorA = colorA;
			BandShader.ColorB = colorB;
			BandShader.ColorC = colorC;
		}

		void FragmentShade(WindowRasterizer& rasterizer, const triangle_fragment_t& fragment, ISceneShader* sceneShader) final
		{
			if (BandShader.SetFragmentData(fragment))
			{
				rasterizer.RasterTriangle(fragment.triangleScreenA, fragment.triangleScreenB, fragment.triangleScreenC, BandShader);
			}
		}
	};
}
#endif