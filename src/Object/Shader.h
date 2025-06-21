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
			Shade.normalWorld = fragment.normal;
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
			Shade.normalWorld = fragment.normal;
			sceneShader->Shade(FragmentColor, fragment.material, Shade);
			rasterizer.DrawLine(FragmentColor, fragment.triangleScreenA, fragment.triangleScreenB);
			rasterizer.DrawLine(FragmentColor, fragment.triangleScreenA, fragment.triangleScreenC);
			rasterizer.DrawLine(FragmentColor, fragment.triangleScreenB, fragment.triangleScreenC);
		}

		void FragmentShade(WindowRasterizer& rasterizer, const triangle_fragment_t& fragment) final
		{
			FragmentColor = fragment.color;
			Shade.positionWorld = fragment.world;
			Shade.normalWorld = fragment.normal;
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
		color_fraction16_t FarColor{ UFRACTION16_1X,0, 0 };
		color_fraction16_t NearColor{ 0, 0, UFRACTION16_1X };

		void FragmentShade(WindowRasterizer& rasterizer, const point_fragment_t& fragment, ISceneShader* sceneShader) final
		{
			FragmentShade(rasterizer, fragment);
		}

		void FragmentShade(WindowRasterizer& rasterizer, const point_fragment_t& fragment) final
		{
			const ufraction16_t proximityFraction = AbstractPixelShader::GetZFraction(fragment.screen.z, 1, VERTEX16_RANGE);
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
		color_fraction16_t FarColor{ UFRACTION16_1X,0, 0 };
		color_fraction16_t NearColor{ 0, 0, UFRACTION16_1X };

		void FragmentShade(WindowRasterizer& rasterizer, const triangle_fragment_t& fragment, ISceneShader* sceneShader) final
		{
			FragmentShade(rasterizer, fragment);
		}

		void FragmentShade(WindowRasterizer& rasterizer, const triangle_fragment_t& fragment) final
		{
			const uint16_t z = int16_t(((int32_t)fragment.triangleScreenA.z + fragment.triangleScreenB.z + fragment.triangleScreenC.z) / 3);
			const ufraction16_t proximityFraction = AbstractPixelShader::GetZFraction(z, 1, VERTEX16_RANGE / 2);
			ColorFraction::ColorInterpolateLinear(FragmentColor, FarColor, NearColor, proximityFraction);
			rasterizer.DrawTriangle(FragmentColor, fragment.triangleScreenA, fragment.triangleScreenB, fragment.triangleScreenC);
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
			FragmentColor = { AbstractPixelShader::GetNormalFraction((int16_t)fragment.normal.x),
								AbstractPixelShader::GetNormalFraction((int16_t)fragment.normal.y),
								AbstractPixelShader::GetNormalFraction((int16_t)fragment.normal.z) };
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