#ifndef _INTEGER_WORLD_COLOR_FRACTION_h
#define _INTEGER_WORLD_COLOR_FRACTION_h

#include <IntegerSignal.h>

namespace IntegerWorld
{
	using namespace IntegerSignal;
	using namespace IntegerSignal::Fraction;

	namespace ColorFraction
	{
		struct color_fraction16_t
		{
			ufraction16_t r;
			ufraction16_t g;
			ufraction16_t b;
		};

		static constexpr color_fraction16_t COLOR_BLACK = { 0,0,0 };
		static constexpr color_fraction16_t COLOR_WHITE = { UFRACTION16_1X, UFRACTION16_1X, UFRACTION16_1X };
		static constexpr color_fraction16_t COLOR_RED = { UFRACTION16_1X, 0, 0 };
		static constexpr color_fraction16_t COLOR_GREEN = { 0, UFRACTION16_1X, 0 };
		static constexpr color_fraction16_t COLOR_BLUE = { 0, 0, UFRACTION16_1X };

		static constexpr uint8_t RgbDownShiftDown = GetBitShifts(UFRACTION16_1X - 1) - GetBitShifts(UINT8_MAX);
		static constexpr uint8_t RgbUpShiftUp = GetBitShifts(UFRACTION16_1X) - GetBitShifts(uint16_t(UINT8_MAX) + 1);
		static constexpr uint8_t RgbUpShiftDown = GetBitShifts(uint16_t(UINT8_MAX + 1)) - RgbUpShiftUp;

		static constexpr ufraction16_t RgbValueToFraction(const uint8_t rgbValue)
		{
			return Fraction::GetUFraction16(rgbValue, UINT8_MAX);
		}

		static constexpr uint8_t FractionToRgbValue(const ufraction16_t fraction)
		{
			return (fraction >= UFRACTION16_1X) ? (UINT8_MAX) : (fraction >> RgbDownShiftDown);
		}

		static constexpr color_fraction16_t RgbToColorFraction(const uint8_t r, const uint8_t g, const uint8_t b)
		{
			return color_fraction16_t{
				ufraction16_t((uint16_t(r) << RgbUpShiftUp) | uint16_t(r >> RgbUpShiftDown)),
				ufraction16_t((uint16_t(g) << RgbUpShiftUp) | uint16_t(g >> RgbUpShiftDown)),
				ufraction16_t((uint16_t(b) << RgbUpShiftUp) | uint16_t(b >> RgbUpShiftDown)),
			};
		}

		static void RgbToColorFraction(color_fraction16_t& color, const uint8_t r, const uint8_t g, const uint8_t b)
		{
			color.r = ufraction16_t((uint16_t(r) << RgbUpShiftUp) | (r >> RgbUpShiftDown));
			color.g = ufraction16_t((uint16_t(g) << RgbUpShiftUp) | (g >> RgbUpShiftDown));
			color.b = ufraction16_t((uint16_t(b) << RgbUpShiftUp) | (b >> RgbUpShiftDown));
		}

		static constexpr color_fraction16_t RgbToColorFraction(const uint32_t rgb)
		{
			return RgbToColorFraction(
				uint8_t(rgb >> 16),
				uint8_t(rgb >> 8),
				uint8_t(rgb));
		}

		static void ColorInterpolateLinear(color_fraction16_t& output, const color_fraction16_t& a, const color_fraction16_t& b, const ufraction16_t fraction)
		{
			const ufraction16_t inverse = UFRACTION16_1X - fraction;

			output.r = Fraction::Scale(inverse, a.r) + Fraction::Scale(fraction, b.r);
			output.g = Fraction::Scale(inverse, a.g) + Fraction::Scale(fraction, b.g);
			output.b = Fraction::Scale(inverse, a.b) + Fraction::Scale(fraction, b.b);
		}

		static void ColorInterpolate(color_fraction16_t& output, const color_fraction16_t& a, const color_fraction16_t& b, const ufraction16_t fraction)
		{
			const ufraction16_t inverse = UFRACTION16_1X - fraction;

			int32_t x = Fraction::Scale(inverse, a.r);
			int32_t y = Fraction::Scale(fraction, b.r);
			x = x * x;
			y = y * y;
			output.r = SquareRoot32(x + y);

			x = Fraction::Scale(inverse, a.g);
			y = Fraction::Scale(fraction, b.g);
			x = x * x;
			y = y * y;
			output.g = SquareRoot32(x + y);

			x = Fraction::Scale(inverse, a.b);
			y = Fraction::Scale(fraction, b.b);
			x = x * x;
			y = y * y;
			output.b = SquareRoot32(x + y);
		}

		/// <summary>
		/// Converts ufraction16_t HSV to color_fraction16_t (all channels in ufraction16_t).
		/// This implementation is optimized for fixed-point math and does not use division or modulo at runtime.
		/// </summary>
		/// <param name="hue">Hue angle [0 ; UFRACTION16_1X], corresponds to the 360 degrees color wheel.</param>
		/// <param name="saturation">Saturation value [0 ; UFRACTION16_1X].</param>
		/// <param name="value">Brightness value [0 ; UFRACTION16_1X].</param>
		/// <returns>color_fraction16_t in ufraction16_t color space.</returns>
		static color_fraction16_t HsvToColorFraction(const ufraction16_t hue, const ufraction16_t saturation, const ufraction16_t value)
		{
			static constexpr uint8_t Segments = 6;
			static constexpr uint32_t FullScale = uint32_t(Segments) * UFRACTION16_1X;

			if (saturation == 0)
			{
				// Achromatic (gray).
				return color_fraction16_t{ value, value, value };
			}
			else
			{
				// Scale hue to FullScale.
				const uint32_t hueScaled = Fraction::Scale(hue, FullScale);

				// Determine which segment of the color wheel the hue is in.
				const uint8_t hueSegment = Fraction::Scale(hue, Segments);

				// Fractional part within the segment.
				const ufraction16_t segmentHue = hueScaled - (uint32_t(hueSegment) * UFRACTION16_1X);

				// Calculate intermediate values for RGB conversion. 
				const ufraction16_t valueMinSaturation = value - Fraction::Scale(saturation, value);
				// Cache saturationPortion intermediate to avoid an extra scale operation.
				const ufraction16_t saturationPortion = Fraction::Scale(segmentHue, saturation);
				const ufraction16_t valueMinusSaturationPortion = value - Fraction::Scale(saturationPortion, value);
				const ufraction16_t valueMinusInverseSaturationPortion = value - Fraction::Scale(ufraction16_t(UFRACTION16_1X - saturationPortion), value);

				// Determine the RGB values based on the hue segment.
				switch (hueSegment % Segments)
				{
				case 0:
					return color_fraction16_t{ value, valueMinusInverseSaturationPortion, valueMinSaturation };
				case 1:
					return color_fraction16_t{ valueMinusSaturationPortion, value, valueMinSaturation };
				case 2:
					return color_fraction16_t{ valueMinSaturation, value, valueMinusInverseSaturationPortion };
				case 3:
					return color_fraction16_t{ valueMinSaturation, valueMinusSaturationPortion, value };
				case 4:
					return color_fraction16_t{ valueMinusInverseSaturationPortion, valueMinSaturation, value };
				case (Segments - 1):
				default:
					return color_fraction16_t{ value, valueMinSaturation, valueMinusSaturationPortion };
				}
			}
		}
	}
}
#endif