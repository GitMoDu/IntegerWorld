#ifndef _INTEGER_WORLD_OBJECT_SHADER_h
#define _INTEGER_WORLD_OBJECT_SHADER_h

#include "AbstractObject.h"

namespace IntegerWorld
{
	struct AbstractPixelShader
	{
		static ufraction16_t GetZFraction(const int16_t& z, const int16_t rangeMin, const int16_t rangeMax)
		{
			if (z >= rangeMax)
			{
				return 0;
			}
			else if (z <= rangeMin)
			{
				return UFRACTION16_1X;
			}
			else
			{
				return UFRACTION16_1X - Fraction::GetUFraction16(uint32_t(z - rangeMin), uint32_t((rangeMax - rangeMin)));
			}
		}

		static ufraction16_t GetDistanceFraction(const int16_t z)
		{
			static constexpr uint16_t range = VERTEX16_RANGE;
			static constexpr uint8_t shifts = 15 - GetBitShifts(range);
			if (z > 0)
			{
				return MinValue(uint32_t(UFRACTION16_1X), (uint32_t(z) >> shifts));
			}
			else
			{
				return 0;
			}
		}

		static ufraction16_t GetNormalFraction(const int16_t normal)
		{
			static constexpr uint16_t range = VERTEX16_UNIT;

			if (normal >= 0)
			{
				return Fraction::GetUFraction16(uint16_t(normal), range);
			}
			else
			{
				return Fraction::GetUFraction16(uint32_t(-int32_t(normal)), uint32_t(range));
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


				// Standard CW area formula. TODO: refactor as CCW
				// Compute denominator (twice the area of the triangle)
				TriangleArea = (int32_t(b.y) - c.y) * (int32_t(a.x) - c.x) + (int32_t(c.x) - b.x) * (int32_t(a.y) - c.y);

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

	struct BackgroundFlatFillShader : public IFragmentShader<flat_background_fragment_t>
	{
	private:
		color_fraction16_t FragmentColor{};

	public:
		void FragmentShade(WindowRasterizer& rasterizer, const flat_background_fragment_t& fragment, ISceneShader* sceneShader) final
		{
			FragmentColor = fragment.color;
			sceneShader->Shade(FragmentColor, fragment.material);
			rasterizer.FillSurface(FragmentColor);
		}

		void FragmentShade(WindowRasterizer& rasterizer, const flat_background_fragment_t& fragment) final
		{
			rasterizer.FillSurface(fragment.color);
		}
	};

	struct PointPositionFragmentShader : IFragmentShader<point_fragment_t>
	{
	private:
		color_fraction16_t FragmentColor{};
		world_position_shade_t Shade{};

	public:
		void FragmentShade(WindowRasterizer& rasterizer, const point_fragment_t& fragment, ISceneShader* sceneShader) final
		{
			FragmentColor = fragment.color;
			Shade.positionWorld = fragment.world;
			sceneShader->Shade(FragmentColor, fragment.material, Shade);
			rasterizer.DrawPoint(FragmentColor, fragment.screen);
		}

		void FragmentShade(WindowRasterizer& rasterizer, const point_fragment_t& fragment) final
		{
			rasterizer.DrawPoint(fragment.color, fragment.screen);
		}
	};

	template<typename point_fragment_type_t = point_fragment_t>
	struct PointPixelFixedColorFragmentShader : IFragmentShader<point_fragment_type_t>
	{
	public:
		color_fraction16_t Color{ UFRACTION16_1X, UFRACTION16_1X, UFRACTION16_1X };

		void FragmentShade(WindowRasterizer& rasterizer, const point_fragment_type_t& fragment, ISceneShader* sceneShader) final
		{
			FragmentShade(rasterizer, fragment);
		}

		void FragmentShade(WindowRasterizer& rasterizer, const point_fragment_type_t& fragment) final
		{
			rasterizer.DrawPoint(Color, fragment.screen.x, fragment.screen.y);
		}
	};

	struct TriangleFillFragmentShader : IFragmentShader<triangle_fragment_t>
	{
	private:
		world_position_normal_shade_t Shade{};
		color_fraction16_t FragmentColor{};

	public:
		void FragmentShade(WindowRasterizer& rasterizer, const triangle_fragment_t& fragment, ISceneShader* sceneShader) final
		{
			FragmentColor = fragment.color;
			Shade.normalWorld = fragment.normalWorld;
			Shade.positionWorld = fragment.world;

			sceneShader->Shade(FragmentColor, fragment.material, Shade);
			rasterizer.DrawTriangle(FragmentColor, fragment.triangleScreenA, fragment.triangleScreenB, fragment.triangleScreenC);
		}

		void FragmentShade(WindowRasterizer& rasterizer, const triangle_fragment_t& fragment) final
		{
			rasterizer.DrawTriangle(fragment.color, fragment.triangleScreenA, fragment.triangleScreenB, fragment.triangleScreenC);
		}
	};

	struct TriangleWireframeFragmentShader : IFragmentShader<triangle_fragment_t>
	{
	private:
		world_position_normal_shade_t Shade{};
		color_fraction16_t FragmentColor{};

	public:
		void FragmentShade(WindowRasterizer& rasterizer, const triangle_fragment_t& fragment, ISceneShader* sceneShader) final
		{
			FragmentColor = fragment.color;
			Shade.positionWorld = fragment.world;
			Shade.normalWorld = fragment.normalWorld;
			sceneShader->Shade(FragmentColor, fragment.material, Shade);
			rasterizer.DrawLine(FragmentColor, fragment.triangleScreenA, fragment.triangleScreenB);
			rasterizer.DrawLine(FragmentColor, fragment.triangleScreenA, fragment.triangleScreenC);
			rasterizer.DrawLine(FragmentColor, fragment.triangleScreenB, fragment.triangleScreenC);
		}

		void FragmentShade(WindowRasterizer& rasterizer, const triangle_fragment_t& fragment) final
		{
			FragmentColor = fragment.color;
			Shade.positionWorld = fragment.world;
			Shade.normalWorld = fragment.normalWorld;
			rasterizer.DrawLine(FragmentColor, fragment.triangleScreenA, fragment.triangleScreenB);
			rasterizer.DrawLine(FragmentColor, fragment.triangleScreenA, fragment.triangleScreenC);
			rasterizer.DrawLine(FragmentColor, fragment.triangleScreenB, fragment.triangleScreenC);
		}
	};

	struct PointNormalFragmentShader : IFragmentShader<point_normal_fragment_t>
	{
	private:
		color_fraction16_t FragmentColor{};
		world_position_normal_shade_t Shade{};

	public:
		void FragmentShade(WindowRasterizer& rasterizer, const point_normal_fragment_t& fragment, ISceneShader* sceneShader) final
		{
			FragmentColor = fragment.color;
			Shade.normalWorld = fragment.normal;
			Shade.positionWorld = fragment.world;

			sceneShader->Shade(FragmentColor, fragment.material, Shade);
			rasterizer.DrawPoint(FragmentColor, fragment.screen);
		}

		void FragmentShade(WindowRasterizer& rasterizer, const point_normal_fragment_t& fragment) final
		{
			rasterizer.DrawPoint(fragment.color, fragment.screen);
		}
	};

	struct PointZFragmentShader : IFragmentShader<point_fragment_t>
	{
	private:
		color_fraction16_t FragmentColor{};

	public:
		color_fraction16_t FarColor{ UFRACTION16_1X, 0, 0 };
		color_fraction16_t NearColor{ 0, 0, UFRACTION16_1X };

		void FragmentShade(WindowRasterizer& rasterizer, const point_fragment_t& fragment, ISceneShader* sceneShader) final
		{
			FragmentShade(rasterizer, fragment);
		}

		void FragmentShade(WindowRasterizer& rasterizer, const point_fragment_t& fragment) final
		{
			const ufraction16_t proximityFraction = AbstractPixelShader::GetZFraction(fragment.screen.z, 1, ((VERTEX16_RANGE / 3) * 2));
			ColorFraction::ColorInterpolateLinear(FragmentColor, FarColor, NearColor, proximityFraction);
			rasterizer.DrawPoint(FragmentColor, fragment.screen.x, fragment.screen.y);
		}
	};

	struct TriangleFillZFragmentShader : IFragmentShader<triangle_fragment_t>
	{
	private:
		color_fraction16_t FragmentColor{};
		vertex16_t LineCenter{};

	public:
		color_fraction16_t FarColor{ UFRACTION16_1X, 0, 0 };
		color_fraction16_t NearColor{ 0, 0, UFRACTION16_1X };

		void FragmentShade(WindowRasterizer& rasterizer, const triangle_fragment_t& fragment, ISceneShader* sceneShader) final
		{
			FragmentShade(rasterizer, fragment);
		}

		void FragmentShade(WindowRasterizer& rasterizer, const triangle_fragment_t& fragment) final
		{
			const uint16_t z = int16_t(((int32_t)fragment.triangleScreenA.z + fragment.triangleScreenB.z + fragment.triangleScreenC.z) / 3);
			const ufraction16_t proximityFraction = AbstractPixelShader::GetZFraction(z, 1, ((VERTEX16_RANGE / 3) * 2));
			ColorFraction::ColorInterpolateLinear(FragmentColor, FarColor, NearColor, proximityFraction);
			rasterizer.DrawTriangle(FragmentColor, fragment.triangleScreenA, fragment.triangleScreenB, fragment.triangleScreenC);
		}
	};

	struct TriangleInterpolateZFragmentShader : IFragmentShader<triangle_fragment_t>
	{
	private:
		struct ZInterpolatorShaderFunctor : AbstractPixelShader::AbstractTriangleZFunctor
		{
			color_fraction16_t FarColor{ UFRACTION16_1X,0, 0 };
			color_fraction16_t NearColor{ 0, 0, UFRACTION16_1X };

			bool operator()(color_fraction16_t& color, const int16_t x, const int16_t y)
			{
				const int32_t w0 = (int32_t(BmCy) * (x - Cx)) + (int32_t(CmBx) * (y - Cy));
				const int32_t w1 = (int32_t(CmAy) * (x - Cx)) + (int32_t(AmCx) * (y - Cy));
				const int32_t w2 = TriangleArea - w0 - w1;
				const int16_t z = ((w0 * Az) + (w1 * Bz) + (w2 * Cz)) / TriangleArea;
				const ufraction16_t proximityFraction = AbstractPixelShader::GetZFraction(z, 1, ((VERTEX16_RANGE / 3) * 2));
				ColorFraction::ColorInterpolateLinear(color, FarColor, NearColor, proximityFraction);

				return true;
			}
		} ZShader{};

	public:
		void SetColors(const color_fraction16_t nearColor, const color_fraction16_t farColor)
		{
			ZShader.NearColor = nearColor;
			ZShader.FarColor = farColor;
		}

		color_fraction16_t GetNearColor() const
		{
			return ZShader.NearColor;
		}

		color_fraction16_t GetFarColor() const
		{
			return ZShader.FarColor;
		}

		void FragmentShade(WindowRasterizer& rasterizer, const triangle_fragment_t& fragment, ISceneShader* sceneShader) final
		{
			FragmentShade(rasterizer, fragment);
		}

		void FragmentShade(WindowRasterizer& rasterizer, const triangle_fragment_t& fragment) final
		{
			if (ZShader.SetFragmentData(fragment))
			{
				rasterizer.RasterTriangle(fragment.triangleScreenA, fragment.triangleScreenB, fragment.triangleScreenC, ZShader);
			}
		}
	};

	struct TriangleFillNormalFragmentShader : IFragmentShader<triangle_fragment_t>
	{
	private:
		color_fraction16_t FragmentColor{};

	public:
		void FragmentShade(WindowRasterizer& rasterizer, const triangle_fragment_t& fragment, ISceneShader* sceneShader) final
		{
			FragmentShade(rasterizer, fragment);
		}

		void FragmentShade(WindowRasterizer& rasterizer, const triangle_fragment_t& fragment) final
		{
			FragmentColor = { AbstractPixelShader::GetNormalFraction((int16_t)fragment.normalWorld.x),
								AbstractPixelShader::GetNormalFraction((int16_t)fragment.normalWorld.y),
								AbstractPixelShader::GetNormalFraction((int16_t)fragment.normalWorld.z) };
			rasterizer.DrawTriangle(FragmentColor, fragment.triangleScreenA, fragment.triangleScreenB, fragment.triangleScreenC);
		}
	};

	struct PointFixedNormalFragmentShader : IFragmentShader<point_normal_fragment_t>
	{
	private:
		color_fraction16_t Color{ UFRACTION16_1X, UFRACTION16_1X, UFRACTION16_1X };

	public:
		void SetNormal(const vertex16_t normal)
		{
			vertex16_t normalized(normal);
			NormalizeVertex16(normalized);
			Color = { AbstractPixelShader::GetNormalFraction((int16_t)normalized.x),
								AbstractPixelShader::GetNormalFraction((int16_t)normalized.y),
								AbstractPixelShader::GetNormalFraction((int16_t)normalized.z) };
		}

		void FragmentShade(WindowRasterizer& rasterizer, const point_normal_fragment_t& fragment, ISceneShader* sceneShader) final
		{
			FragmentShade(rasterizer, fragment);
		}

		void FragmentShade(WindowRasterizer& rasterizer, const point_normal_fragment_t& fragment) final
		{
			rasterizer.DrawPoint(Color, fragment.screen.x, fragment.screen.y);
		}
	};

	struct EdgeFragmentShader : IFragmentShader<edge_fragment_t>
	{
	private:
		color_fraction16_t FragmentColor{};

	public:
		void FragmentShade(WindowRasterizer& rasterizer, const edge_fragment_t& fragment, ISceneShader* sceneShader) final
		{
			FragmentShade(rasterizer, fragment);
		}

		void FragmentShade(WindowRasterizer& rasterizer, const edge_fragment_t& fragment) final
		{
			FragmentColor = fragment.color;
			rasterizer.DrawLine(FragmentColor, fragment.start, fragment.end);
		}
	};

	struct EdgeLitFragmentShader : IFragmentShader<edge_fragment_t>
	{
	private:
		color_fraction16_t FragmentColor{};
		world_position_shade_t Shade{};

	public:
		void FragmentShade(WindowRasterizer& rasterizer, const edge_fragment_t& fragment, ISceneShader* sceneShader) final
		{
			FragmentColor = fragment.color;
			Shade.positionWorld = fragment.world;
			sceneShader->Shade(FragmentColor, fragment.material, Shade);
			rasterizer.DrawLine(FragmentColor, fragment.start, fragment.end);
		}

		void FragmentShade(WindowRasterizer& rasterizer, const edge_fragment_t& fragment) final
		{
			rasterizer.DrawLine(fragment.color, fragment.start, fragment.end);
		}
	};
}
#endif