#ifndef _INTEGER_WORLD_ABSTRACT_SURFACE_RASTERIZER_h
#define _INTEGER_WORLD_ABSTRACT_SURFACE_RASTERIZER_h

#include <stdint.h>

namespace IntegerWorld
{
	/// <summary>
	/// Implements a random dithering algorithm for alpha values using a simple pseudo-random number generator.
	/// </summary>
	class AlphaRandomDitherer
	{
	private:
		uint16_t rng = 42;

	public:
		/// <summary>
		/// Returns true when the given alpha value should produce a visible pixel after random dithering.
		/// </summary>
		/// <param name="alpha">Alpha value in [0, 255]. Higher means more likely to pass.</param>
		/// <returns>True to draw the pixel; false to skip it.</returns>
		bool Dither(const uint8_t alpha)
		{
			rng ^= rng << 7;
			rng ^= rng >> 9;
			rng ^= rng << 8;

			return alpha > uint8_t(rng);
		}
	};

	enum class pixel_blend_mode_t : uint8_t
	{
		Alpha,		// Blends the new color with the existing pixel color using alpha blending.
		Add,		// Adds the new color to the existing pixel color.
		Subtract,	// Subtracts the new color from the existing pixel color.
		Multiply,	// Multiplies the new color with the existing pixel color.
		Screen		// Blends the new color with the existing pixel color using the screen blend mode.
	};

	/// <summary>
	/// Abstract base class template for rasterizing onto a 2D surface.
	/// - Owns a reference to the surface.
	/// - Tracks current window size (width/height).
	/// - Provides boundary checks and small utility helpers used by higher-level rasterizers.
	/// </summary>
	/// <typeparam name="SurfaceType">
	/// The output surface type. It is expected to behave like IOutputSurface (Pixel/Line/TriangleFill/etc.)
	/// but can be any type that exposes a compatible API.
	/// </typeparam>
	template<typename SurfaceType>
	class AbstractSurfaceRasterizer
	{
	protected:
		/// <summary>
		/// Fixed-point scale used by line and triangle rendering.
		/// </summary>
		static constexpr uint8_t BRESENHAM_SCALE = 8;

		// Fixed-point rounding helper: add half-unit for the current Bresenham scale, then arithmetic right-shift.
		static constexpr int32_t FP_ROUND_HALF = (int32_t(1) << (BRESENHAM_SCALE - 1));

		/// <summary>
		/// Rounds a 32-bit fixed-point value to the nearest integer using a signed right shift.
		/// </summary>
		/// <param name="fx">A fixed-point value represented as a 32-bit signed integer. The number of fractional bits is implied by BRESENHAM_SCALE; FP_ROUND_HALF is used to perform rounding.</param>
		/// <returns>The rounded integer value as a 16-bit signed (int16_t).</returns>
		static constexpr int16_t FixedRoundToInt(const int32_t fx)
		{
			return SignedRightShift(fx + FP_ROUND_HALF, BRESENHAM_SCALE);
		}

		static constexpr int32_t IntToFixed(const int16_t x)
		{
			return SignedLeftShift<int32_t>(x, BRESENHAM_SCALE);
		}

	protected:
		/// <summary>
		/// Outcode bit mask for Cohen-Sutherland style region tests.
		/// </summary>
		enum class OutcodeEnum : uint8_t
		{
			OUT_LEFT = 1,
			OUT_RIGHT = 2,
			OUT_BOTTOM = 4,
			OUT_TOP = 8
		};

	protected:
		int16_t SurfaceWidth = 1;
		int16_t SurfaceHeight = 1;

		SurfaceType& Surface;

	public:
		AbstractSurfaceRasterizer(SurfaceType& surface)
			: Surface(surface)
		{
		}

		/// <summary>
		/// Current drawing window width in pixels.
		/// </summary>
		uint16_t Width() const
		{
			return SurfaceWidth;
		}

		/// <summary>
		/// Current drawing window height in pixels.
		/// </summary>
		uint16_t Height() const
		{
			return SurfaceHeight;
		}

	protected:
		/// <summary>
		/// Determines whether the specified 2D point (x, y) is within the current drawing window boundaries.
		/// </summary>
		/// <param name="x">The x-coordinate of the point to test.</param>
		/// <param name="y">The y-coordinate of the point to test.</param>
		/// <returns>True if the point is inside the window; otherwise, false.</returns>
		bool IsInsideWindow(const int16_t x, const int16_t y) const
		{
			return x >= 0 && x < SurfaceWidth
				&& y >= 0 && y < SurfaceHeight;
		}

		/// <summary>
		/// Clamps the specified coordinates to ensure they remain within the current drawing window boundaries.
		/// </summary>
		/// <param name="x">Reference to the x-coordinate to clamp. Modified in place if out of bounds.</param>
		/// <param name="y">Reference to the y-coordinate to clamp. Modified in place if out of bounds.</param>
		void LimitToWindow(int16_t& x, int16_t& y) const
		{
			x = LimitValue<int16_t>(x, 0, SurfaceWidth - 1);
			y = LimitValue<int16_t>(y, 0, SurfaceHeight - 1);
		}

		/// <summary>
		/// Computes the outcode for a point based on its position relative to the frame boundaries.
		/// </summary>
		/// <param name="x">The x-coordinate of the point.</param>
		/// <param name="y">The y-coordinate of the point.</param>
		/// <returns>A uint8_t mask indicating which sides (left, right, top, bottom) the point is outside of.</returns>
		uint8_t ComputeOutCode(const int16_t x, const int16_t y) const
		{
			uint8_t code = 0;
			if (x < 0) code |= (uint8_t)OutcodeEnum::OUT_LEFT;
			else if (x >= SurfaceWidth) code |= (uint8_t)OutcodeEnum::OUT_RIGHT;
			if (y < 0) code |= (uint8_t)OutcodeEnum::OUT_TOP;
			else if (y >= SurfaceHeight) code |= (uint8_t)OutcodeEnum::OUT_BOTTOM;

			return code;
		}
	};
}
#endif