#ifndef _INTEGER_WORLD_VIEWPORT_h
#define _INTEGER_WORLD_VIEWPORT_h

#include "../Framework/Model.h"

namespace IntegerWorld
{
	class ViewportProjector
	{
	private:
		static constexpr uint16_t RangeMin = VERTEX16_UNIT;
		static constexpr uint16_t RangeMax = (((uint32_t)VERTEX16_RANGE * 6) / 10);
		static constexpr uint16_t Range = RangeMax - RangeMin;
		static constexpr uint8_t DownShift = 12;
		static constexpr uint8_t DownShift2 = GetBitShifts(RangeMax) - GetBitShifts(RangeMin);

	private:
		int16_t ViewWidthHalf = 0;
		int16_t ViewHeightHalf = 0;

		int16_t verticalNum = 0;
		int16_t verticalDenum = 0;

		uint16_t distanceNum = (RangeMin + RangeMax) / 2;

	public:
		ViewportProjector()
		{
		}

		void SetDimensions(uint16_t viewWidth, const uint16_t viewHeight)
		{
			verticalNum = viewWidth;
			verticalDenum = viewHeight;
			ViewWidthHalf = MinValue(uint16_t(INT16_MAX), uint16_t(viewWidth >> 1));
			ViewHeightHalf = MinValue(uint16_t(INT16_MAX), uint16_t(viewHeight >> 1));
		}

		/// <summary>
		/// </summary>
		/// <param name="fovFraction">0 -> Minimum FoV; UFRACTION16_1X -> Max FoV.</param>
		void SetFov(const ufraction16_t fovFraction)
		{
			distanceNum = RangeMin + Fraction::Scale(ufraction16_t(fovFraction), Range);
		}

		void Project(vertex16_t& cameraToscreen)
		{
			const int32_t distanceDenum = int32_t(distanceNum + cameraToscreen.z);

			int32_t ix, iy;
			if (distanceDenum == 0)
			{
				ix = cameraToscreen.x;
				ix = SignedRightShift((ix * ViewWidthHalf), DownShift);

				iy = cameraToscreen.y;
				iy = (iy * verticalNum) / verticalDenum;
				iy = SignedRightShift((iy * ViewHeightHalf), DownShift);
			}
			else
			{
				ix = ((int32_t)cameraToscreen.x * distanceNum) / distanceDenum;
				ix = SignedRightShift((ix * ViewWidthHalf), DownShift);

				iy = ((int32_t)cameraToscreen.y * distanceNum) / distanceDenum;
				iy = (iy * verticalNum) / verticalDenum;
				iy = SignedRightShift((iy * ViewHeightHalf), DownShift);
			}

			cameraToscreen.x = ViewWidthHalf + ix;
			cameraToscreen.y = ViewHeightHalf + iy;
			cameraToscreen.z = distanceDenum;
		}
	};	
}
#endif