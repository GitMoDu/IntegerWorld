#ifndef _INTEGER_WORLD_OBJECT_SHADER_h
#define _INTEGER_WORLD_OBJECT_SHADER_h

#include "AbstractObject.h"

namespace IntegerWorld
{
	struct AbstractPixelShader
	{
		static uint8_t GetZ8(const int16_t& z, const int16_t rangeMin = 1024, const int16_t rangeMax = 24576)
		{
			if (z >= rangeMax)
			{
				return 0;
			}
			else if (z <= rangeMin)
			{
				return UINT8_MAX;
			}
			else
			{
				return UINT8_MAX - ((uint32_t(z - rangeMin) * UINT8_MAX) / uint32_t((rangeMax - rangeMin)));
			}
		}

		static ufraction16_t GetZFraction(const int16_t& z, const int16_t rangeMin = 1024, const int16_t rangeMax = 24576)
		{
			if (z >= rangeMax)
			{
				return 0;
			}
			else if (z <= rangeMin)
			{
				return UFraction16::FRACTION_1X;
			}
			else
			{
				return UFraction16::FRACTION_1X - ((UFraction16::FRACTION_1X * (uint32_t)(z - rangeMin)) / (uint32_t)(rangeMax - rangeMin));
			}
		}

		static ufraction16_t GetDistanceFraction(const int16_t z)
		{
			static constexpr uint16_t range = VERTEX16_RANGE;
			static constexpr uint8_t shifts = 15 - GetBitShifts(range);
			if (z > 0)
			{
				return MinValue(uint32_t(UFraction16::FRACTION_1X), (uint32_t(z) >> shifts));
			}
			else
			{
				return 0;
			}
		}

		static ufraction16_t GetNormalFraction(const int16_t normal)
		{
			static constexpr uint16_t range = VERTEX16_UNIT;

			const uint16_t normalAbs = uint16_t(normal < 0 ? -normal : normal);

			if (normalAbs >= range)
			{
				return UFraction16::FRACTION_1X;
			}
			else
			{
				return (UFraction16::FRACTION_1X * normalAbs) / range;
			}
		}

		static uint8_t GetNormal8(const int16_t normal)
		{
			static constexpr uint16_t range = VERTEX16_UNIT;

			const uint16_t normalAbs = uint16_t(normal < 0 ? -normal : normal);

			if (normalAbs >= range)
			{
				return UINT8_MAX;
			}
			else
			{
				return (uint16_t(normalAbs) * UINT8_MAX) >> GetBitShifts(range);
			}
		}


		class AbstractTriangleFunctor
		{
		protected:
			int32_t TriangleArea = 0;
			int16_t Cx = 0;
			int16_t Cy = 0;
			int16_t BmCy = 0;
			int16_t CmBx = 0;
			int16_t CmAy = 0;
			int16_t AmCx = 0;

		public:
			AbstractTriangleFunctor() {}

			bool SetFragmentData(const triangle_fragment_t& fragment)
			{
				const vertex16_t& a = fragment.triangleScreenA;
				const vertex16_t& b = fragment.triangleScreenB;
				const vertex16_t& c = fragment.triangleScreenC;

				// Compute denominator (twice the area of the triangle)
				TriangleArea = (int32_t(b.y) - c.y) * (int32_t(a.x) - c.x) + (int32_t(c.x) - b.x) * (int32_t(a.y) - c.y);

				if (TriangleArea == 0)
				{
					return false;
				}
				else // Pre-calculate intermediates and cache triangle properties.
				{
					if (TriangleArea < 0)
					{
						TriangleArea = -TriangleArea;

						// Swap B and C
						BmCy = c.y - b.y;
						CmBx = b.x - c.x;
						CmAy = b.y - a.y;
						AmCx = a.x - b.x;

						Cx = b.x;
						Cy = b.y;
					}
					else
					{
						BmCy = b.y - c.y;
						CmBx = c.x - b.x;
						CmAy = c.y - a.y;
						AmCx = a.x - c.x;

						Cx = c.x;
						Cy = c.y;
					}

					return true;
				}
			}
		};

		class AbstractTriangleZFunctor
		{
		protected:
			int32_t TriangleArea = 0;
			int16_t Az = 0;
			int16_t Bz = 0;
			int16_t Cz = 0;
			int16_t Cx = 0;
			int16_t Cy = 0;
			int16_t BmCy = 0;
			int16_t CmBx = 0;
			int16_t CmAy = 0;
			int16_t AmCx = 0;

		public:
			AbstractTriangleZFunctor() {}

			bool SetFragmentData(const triangle_fragment_t& fragment)
			{
				const vertex16_t& a = fragment.triangleScreenA;
				const vertex16_t& b = fragment.triangleScreenB;
				const vertex16_t& c = fragment.triangleScreenC;

				// Standard CW area formula.
				// Compute denominator (twice the area of the triangle)
				TriangleArea = (int32_t(b.y - c.y) * (a.x - c.x)) + (int32_t(c.x - b.x) * (a.y - c.y));

				if (TriangleArea == 0)
				{
					return false;
				}
				else
				{
					if (TriangleArea < 0)
					{
						TriangleArea = -TriangleArea;

						// Swap B and C
						BmCy = c.y - b.y;
						CmBx = b.x - c.x;
						CmAy = b.y - a.y;
						AmCx = a.x - b.x;

						Cx = b.x;
						Cy = b.y;

						Az = a.z;
						Bz = c.z;
						Cz = b.z;
					}
					else
					{
						BmCy = b.y - c.y;
						CmBx = c.x - b.x;
						CmAy = c.y - a.y;
						AmCx = a.x - c.x;

						Cx = c.x;
						Cy = c.y;

						Az = a.z;
						Bz = b.z;
						Cz = c.z;
					}

					return true;
				}
			}
		};
	};

	struct BackgroundFlatFillShader final : public IFragmentShader<flat_background_fragment_t>
	{
		void FragmentShade(WindowRasterizer& rasterizer, const flat_background_fragment_t& fragment)
		{
			rasterizer.Fill(fragment.color);
		}
	};

	struct PointPositionFragmentShader final : IFragmentShader<point_normal_fragment_t>
	{
		void FragmentShade(WindowRasterizer& rasterizer, const point_normal_fragment_t& fragment)
		{
			rasterizer.DrawPoint(fragment.color, fragment.screen);
		}
	};

	struct PointPositionShadedFragmentShader final : IFragmentShader<point_normal_fragment_t>
	{
	public:
		ISceneShader* SceneShader = nullptr;

	private:
		Rgb8::color_t FragmentColor{};
		scene_shade_t Shade{};

	public:
		void FragmentShade(WindowRasterizer& rasterizer, const point_normal_fragment_t& fragment)
		{
			FragmentColor = fragment.color;
			if (SceneShader != nullptr)
			{
				Shade.position = fragment.world;
				Shade.z = fragment.screen.z;
				SceneShader->Shade(FragmentColor, fragment.material, Shade);
			}
			rasterizer.DrawPoint(FragmentColor, fragment.screen);
		}
	};

	template<typename point_fragment_type_t = point_normal_fragment_t>
	struct PointPixelFixedColorFragmentShader final : IFragmentShader<point_fragment_type_t>
	{
	public:
		Rgb8::color_t Color = Rgb8::WHITE;

		void FragmentShade(WindowRasterizer& rasterizer, const point_fragment_type_t& fragment)
		{
			rasterizer.DrawPixel(Color, fragment.screen.x, fragment.screen.y);
		}
	};

	struct TriangleFillFragmentShader final : IFragmentShader<triangle_fragment_t>
	{
		void FragmentShade(WindowRasterizer& rasterizer, const triangle_fragment_t& fragment)
		{
			rasterizer.DrawTriangle(fragment.color, fragment.triangleScreenA, fragment.triangleScreenB, fragment.triangleScreenC);
		}
	};

	struct TriangleFillShadedFragmentShader final : IFragmentShader<triangle_fragment_t>
	{
	public:
		ISceneShader* SceneShader = nullptr;

	private:
		scene_shade_t Shade{};
		Rgb8::color_t FragmentColor{};

	public:
		void FragmentShade(WindowRasterizer& rasterizer, const triangle_fragment_t& fragment)
		{
			FragmentColor = fragment.color;
			if (SceneShader != nullptr)
			{
				Shade.normal = fragment.normalWorld;
				Shade.position = fragment.world;
				Shade.z = fragment.z;

				SceneShader->Shade(FragmentColor, fragment.material, Shade);
			}
			rasterizer.DrawTriangle(FragmentColor, fragment.triangleScreenA, fragment.triangleScreenB, fragment.triangleScreenC);
		}
	};

	struct TriangleWireframeFragmentShader final : IFragmentShader<triangle_fragment_t>
	{
		void FragmentShade(WindowRasterizer& rasterizer, const triangle_fragment_t& fragment)
		{
			rasterizer.DrawLine(fragment.color, fragment.triangleScreenA, fragment.triangleScreenB);
			rasterizer.DrawLine(fragment.color, fragment.triangleScreenA, fragment.triangleScreenC);
			rasterizer.DrawLine(fragment.color, fragment.triangleScreenB, fragment.triangleScreenC);
		}
	};

	struct TriangleWireframeShadedFragmentShader final : IFragmentShader<triangle_fragment_t>
	{
	public:
		ISceneShader* SceneShader = nullptr;

	private:
		scene_shade_t Shade{};
		Rgb8::color_t FragmentColor{};

	public:
		void FragmentShade(WindowRasterizer& rasterizer, const triangle_fragment_t& fragment)
		{
			FragmentColor = fragment.color;
			Shade.position = fragment.world;
			Shade.normal = fragment.normalWorld;
			Shade.z = fragment.z;

			if (SceneShader != nullptr)
			{
				SceneShader->Shade(FragmentColor, fragment.material, Shade);
			}
			rasterizer.DrawLine(FragmentColor, fragment.triangleScreenA, fragment.triangleScreenB);
			rasterizer.DrawLine(FragmentColor, fragment.triangleScreenA, fragment.triangleScreenC);
			rasterizer.DrawLine(FragmentColor, fragment.triangleScreenB, fragment.triangleScreenC);
		}
	};


	struct BillboardPlaceholderFragmentShader final : IFragmentShader<billboard_fragment_t>
	{
	public:
		Rgb8::color_t Color = Rgb8::WHITE;

	public:
		void FragmentShade(WindowRasterizer& rasterizer, const billboard_fragment_t& fragment)
		{
			// Rectangle edges
			rasterizer.DrawLine(Color, fragment.topLeftX, fragment.topLeftY, fragment.bottomRightX, fragment.topLeftY);
			rasterizer.DrawLine(Color, fragment.bottomRightX, fragment.topLeftY, fragment.bottomRightX, fragment.bottomRightY);
			rasterizer.DrawLine(Color, fragment.bottomRightX, fragment.bottomRightY, fragment.topLeftX, fragment.bottomRightY);
			rasterizer.DrawLine(Color, fragment.topLeftX, fragment.bottomRightY, fragment.topLeftX, fragment.topLeftY);

			// Diagonals
			rasterizer.DrawLine(Color, fragment.topLeftX, fragment.topLeftY, fragment.bottomRightX, fragment.bottomRightY);
			rasterizer.DrawLine(Color, fragment.bottomRightX, fragment.topLeftY, fragment.topLeftX, fragment.bottomRightY);
		}
	};

	struct PointNormalFragmentShader final : IFragmentShader<point_normal_fragment_t>
	{
		void FragmentShade(WindowRasterizer& rasterizer, const point_normal_fragment_t& fragment)
		{
			rasterizer.DrawPoint(fragment.color, fragment.screen);
		}
	};

	struct PointNormalShadedFragmentShader final : IFragmentShader<point_normal_fragment_t>
	{
	public:
		ISceneShader* SceneShader = nullptr;

	private:
		Rgb8::color_t FragmentColor{};
		scene_shade_t Shade{};

	public:
		void FragmentShade(WindowRasterizer& rasterizer, const point_normal_fragment_t& fragment)
		{
			FragmentColor = fragment.color;

			if (SceneShader != nullptr)
			{
				Shade.normal = fragment.normal;
				Shade.position = fragment.world;
				Shade.z = fragment.screen.z;
				SceneShader->Shade(FragmentColor, fragment.material, Shade);
			}
			rasterizer.DrawPoint(FragmentColor, fragment.screen);
		}
	};

	struct PointZFragmentShader final : IFragmentShader<point_normal_fragment_t>
	{
		void FragmentShade(WindowRasterizer& rasterizer, const point_normal_fragment_t& fragment)
		{
			const ufraction16_t proximityFraction = AbstractPixelShader::GetZFraction(fragment.screen.z);

			const uint8_t gray = Curves::Power2U8<>::Get(UFraction16::Fraction(proximityFraction, uint8_t(UINT8_MAX)));

			rasterizer.DrawPixel(Rgb8::Color(gray, gray, gray),
				fragment.screen.x, fragment.screen.y);
		}
	};

	struct TriangleFillZFragmentShader final : IFragmentShader<triangle_fragment_t>
	{
		void FragmentShade(WindowRasterizer& rasterizer, const triangle_fragment_t& fragment)
		{
			const uint16_t z = int16_t(((int32_t)fragment.triangleScreenA.z + fragment.triangleScreenB.z + fragment.triangleScreenC.z) / 3);
			const ufraction16_t proximityFraction = AbstractPixelShader::GetZFraction(z);
			const uint8_t gray = Curves::Power2U8<>::Get(UFraction16::Fraction(proximityFraction, uint8_t(UINT8_MAX)));
			rasterizer.DrawTriangle(Rgb8::Color(gray, gray, gray),
				fragment.triangleScreenA, fragment.triangleScreenB, fragment.triangleScreenC);
		}
	};

	struct TriangleInterpolateZFragmentShader final : IFragmentShader<triangle_fragment_t>
	{
	private:
		struct ZInterpolatorShaderFunctor : AbstractPixelShader::AbstractTriangleZFunctor
		{
			bool operator()(Rgb8::color_t& color, const int16_t x, const int16_t y)
			{
				if (TriangleArea == 0)
				{
					return false;
				}
				const int32_t w0 = (int32_t(BmCy) * (x - Cx)) + (int32_t(CmBx) * (y - Cy));
				const int32_t w1 = (int32_t(CmAy) * (x - Cx)) + (int32_t(AmCx) * (y - Cy));
				const int32_t w2 = TriangleArea - w0 - w1;
				const int16_t z = ((w0 * Az) + (w1 * Bz) + (w2 * Cz)) / TriangleArea;

				if (z <= 0)
				{
					return false;
				}

				const uint8_t gray = Curves::Power2U8<>::Get(AbstractPixelShader::GetZ8(z));

				color = Rgb8::Color(gray, gray, gray);

				return true;
			}
		} ZShader{};

	public:
		void FragmentShade(WindowRasterizer& rasterizer, const triangle_fragment_t& fragment)
		{
			if (ZShader.SetFragmentData(fragment))
			{
				rasterizer.RasterTriangle(fragment.triangleScreenA, fragment.triangleScreenB, fragment.triangleScreenC, ZShader);
			}
		}
	};

	struct TriangleFillNormalFragmentShader final : IFragmentShader<triangle_fragment_t>
	{
	private:
		Rgb8::color_t FragmentColor{};

	public:
		void FragmentShade(WindowRasterizer& rasterizer, const triangle_fragment_t& fragment)
		{
			FragmentColor = Rgb8::Color(
				AbstractPixelShader::GetNormal8((int16_t)fragment.normalWorld.x),
				AbstractPixelShader::GetNormal8((int16_t)fragment.normalWorld.y),
				AbstractPixelShader::GetNormal8((int16_t)fragment.normalWorld.z));
			rasterizer.DrawTriangle(FragmentColor, fragment.triangleScreenA, fragment.triangleScreenB, fragment.triangleScreenC);
		}
	};

	struct PointFixedNormalFragmentShader final : IFragmentShader<point_normal_fragment_t>
	{
	private:
		Rgb8::color_t Color = Rgb8::WHITE;

	public:
		void SetNormal(const vertex16_t normal)
		{
			vertex16_t normalized(normal);
			NormalizeVertex16(normalized);
			Color = Rgb8::Color(AbstractPixelShader::GetNormal8((int16_t)normalized.x),
				AbstractPixelShader::GetNormal8((int16_t)normalized.y),
				AbstractPixelShader::GetNormal8((int16_t)normalized.z));
		}

		void FragmentShade(WindowRasterizer& rasterizer, const point_normal_fragment_t& fragment)
		{
			rasterizer.DrawPixel(Color, fragment.screen.x, fragment.screen.y);
		}
	};

	struct EdgeFragmentShader final : IFragmentShader<edge_fragment_t>
	{
	private:
		Rgb8::color_t FragmentColor{};

	public:
		void FragmentShade(WindowRasterizer& rasterizer, const edge_fragment_t& fragment)
		{
			FragmentColor = fragment.color;
			rasterizer.DrawLine(FragmentColor, fragment.start, fragment.end);
		}
	};

	struct EdgeLitFragmentShader final : IFragmentShader<edge_fragment_t>
	{
	public:
		ISceneShader* SceneShader = nullptr;

	private:
		Rgb8::color_t FragmentColor{};
		scene_shade_t Shade{};

	public:
		void FragmentShade(WindowRasterizer& rasterizer, const edge_fragment_t& fragment)
		{
			FragmentColor = fragment.color;

			if (SceneShader != nullptr)
			{
				Shade.position = fragment.world;
				Shade.z = fragment.z;
				SceneShader->Shade(FragmentColor, fragment.material, Shade);
			}
			rasterizer.DrawLine(FragmentColor, fragment.start, fragment.end);
		}
	};
}
#endif