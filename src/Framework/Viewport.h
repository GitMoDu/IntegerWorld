#ifndef _INTEGER_WORLD_VIEWPORT_h
#define _INTEGER_WORLD_VIEWPORT_h

#include "../Framework/Model.h"

namespace IntegerWorld
{
	class ViewportProjector
	{
	private:
		static constexpr uint16_t RangeMin = VERTEX16_UNIT;
		static constexpr uint8_t RangeUnits = VERTEX16_RANGE / VERTEX16_UNIT;
		static constexpr uint16_t RangeMax = (((uint32_t)VERTEX16_UNIT) * RangeUnits);
		static constexpr uint16_t Range = RangeMax - RangeMin;
		static constexpr uint8_t DownShift = GetBitShifts(Range);

	private:
		int16_t ViewWidthHalf = 0;
		int16_t ViewHeightHalf = 0;

		int16_t verticalNum = 0;
		int16_t verticalDenum = 0;

		uint16_t distanceNum = (RangeMin + RangeMax) / 2;

		uint16_t drawDistance = RangeMax;

	public:
		ViewportProjector()
		{
		}

		void SetDimensions(const uint16_t viewWidth, const uint16_t viewHeight)
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
			distanceNum = RangeMin + Fraction::Scale(fovFraction, Range);
		}

		void SetDrawDistance(const uint16_t distance)
		{
			drawDistance = MinValue(distance, RangeMax);
		}

		uint16_t GetFocalDistance() const
		{
			return distanceNum;
		}

		void GetFrustumView(const vertex16_t& cameraPosition,
			const rotation_angle_t& cameraRotation,
			frustum_view_t& frustum)
		{
			// View distance in world units
			const uint16_t viewShift = distanceNum;

			// Set frustum origin to apparent camera position.
			frustum.origin = cameraPosition;

			// Set the camera rotation.
			frustum.rotation = cameraRotation;

			// Build camera rotation transform.
			transform16_rotate_t camRot{};
			CalculateTransformRotation(camRot, cameraRotation);

			// Camera-space basis vectors.
			vertex16_t forward = { 0, 0, VERTEX16_UNIT };
			vertex16_t right = { VERTEX16_UNIT, 0, 0 };
			vertex16_t up = { 0, VERTEX16_UNIT, 0 };

			// Rotate them into world space using the same intrinsic XYZ convention.
			ApplyTransformRotation(camRot, forward);
			ApplyTransformRotation(camRot, right);
			ApplyTransformRotation(camRot, up);

			// Near / far distances (keeps behavior consistent with the other overload)
			const int16_t nearDist = viewShift >> 5;
			const int16_t farDist = Range / 2;
			const int16_t scaleDenum = viewShift >> 5;

			// View dimensions at near/far planes.
			const int16_t nearHalfWidth = (int32_t(nearDist) * ViewWidthHalf) / scaleDenum;
			const int16_t nearHalfHeight = (int32_t(nearDist) * ViewHeightHalf) / scaleDenum;
			const int16_t farHalfWidth = (int32_t(farDist) * ViewWidthHalf) / scaleDenum;
			const int16_t farHalfHeight = (int32_t(farDist) * ViewHeightHalf) / scaleDenum;

			// Plane centers.
			const vertex16_t nearCenter
			{
				int16_t(frustum.origin.x + int16_t(SignedRightShift(int32_t(forward.x) * nearDist, DownShift))),
				int16_t(frustum.origin.y + int16_t(SignedRightShift(int32_t(forward.y) * nearDist, DownShift))),
				int16_t(frustum.origin.z + int16_t(SignedRightShift(int32_t(forward.z) * nearDist, DownShift)))
			};

			const vertex16_t farCenter
			{
				int16_t(frustum.origin.x + int16_t(SignedRightShift(int32_t(forward.x) * farDist, DownShift))),
				int16_t(frustum.origin.y + int16_t(SignedRightShift(int32_t(forward.y) * farDist, DownShift))),
				int16_t(frustum.origin.z + int16_t(SignedRightShift(int32_t(forward.z) * farDist, DownShift)))
			};

			// Calculate plane corners.
			CalculateFrustumCorners(nearCenter, right, up, nearHalfWidth, nearHalfHeight, frustum.nearPlane);
			CalculateFrustumCorners(farCenter, right, up, farHalfWidth, farHalfHeight, frustum.farPlane);
		}

		void GetFrustum(const camera_state_t& cameraControls, frustum_t& frustum)
		{
			// Keep original radius squared calculation
			frustum.radiusSquared = (uint32_t(drawDistance) * drawDistance);

			// Set frustum origin to apparent camera position.
			frustum.origin = cameraControls.Position;

			// Set the camera rotation.
			frustum.rotation = cameraControls.Rotation;

			// View distance in world units
			const uint16_t viewShift = distanceNum;

			// Build camera rotation cos/sin transform.
			transform16_rotate_t camRot{};
			CalculateTransformRotation(camRot, cameraControls.Rotation);

			// Camera-space basis vectors (forward, right, up)
			vertex16_t forward = { 0, 0, VERTEX16_UNIT };
			vertex16_t right = { VERTEX16_UNIT, 0, 0 };
			vertex16_t up = { 0, VERTEX16_UNIT, 0 };

			// Rotate them into world space.
			ApplyTransformRotation(camRot, forward);
			ApplyTransformRotation(camRot, right);
			ApplyTransformRotation(camRot, up);

			// Near distance.
			const int16_t nearDist = viewShift >> 5;
			const int16_t scaleDenum = viewShift >> 5;

			// View dims at near plane.
			const int16_t nearHalfWidth = (int32_t(nearDist) * ViewWidthHalf) / scaleDenum;
			const int16_t nearHalfHeight = (int32_t(nearDist) * ViewHeightHalf) / scaleDenum;

			// Calculate plane center.
			const vertex16_t nearCenter =
			{
				int16_t(frustum.origin.x + int16_t(SignedRightShift(int32_t(forward.x) * nearDist, DownShift))),
				int16_t(frustum.origin.y + int16_t(SignedRightShift(int32_t(forward.y) * nearDist, DownShift))),
				int16_t(frustum.origin.z + int16_t(SignedRightShift(int32_t(forward.z) * nearDist, DownShift)))
			};

			// Calculate plane corners.
			frustum_plane_t nearPlane{};
			CalculateFrustumCorners(nearCenter, right, up, nearHalfWidth, nearHalfHeight, nearPlane);

			//TODO: Calculate culling planes.
		}

		void Project(vertex16_t& cameraToscreen)
		{
			const int32_t distanceDenum = int32_t(distanceNum) + cameraToscreen.z;

			int32_t ix, iy;
			if (distanceDenum == 0)
			{
				ix = SignedRightShift(int32_t(cameraToscreen.x) * ViewWidthHalf, DownShift);

				iy = (int32_t(cameraToscreen.y) * verticalNum) / verticalDenum;
				iy = SignedRightShift(iy * ViewHeightHalf, DownShift);
			}
			else
			{
				ix = (int32_t(cameraToscreen.x) * distanceNum) / distanceDenum;
				ix = SignedRightShift(ix * ViewWidthHalf, DownShift);

				iy = (int32_t(cameraToscreen.y) * distanceNum) / distanceDenum;
				iy = (iy * verticalNum) / verticalDenum;
				iy = SignedRightShift(iy * ViewHeightHalf, DownShift);
			}

			cameraToscreen.x = ViewWidthHalf + int16_t(ix);
			cameraToscreen.y = ViewHeightHalf + int16_t(iy);
			cameraToscreen.z = distanceDenum;
		}

		// Calculate the four corners of the plane using the center point and the right/up vectors
		void CalculateFrustumCorners(const vertex16_t& center, const vertex16_t& right, const vertex16_t& up,
			const int16_t halfWidth, const int16_t halfHeight, frustum_plane_t& plane)
		{
			// Top-left corner
			plane.topLeft.x = center.x - int16_t(int32_t(right.x) * halfWidth >> GetBitShifts(VERTEX16_UNIT)) - int16_t(int32_t(up.x) * halfHeight >> GetBitShifts(VERTEX16_UNIT));
			plane.topLeft.y = center.y - int16_t(int32_t(right.y) * halfWidth >> GetBitShifts(VERTEX16_UNIT)) - int16_t(int32_t(up.y) * halfHeight >> GetBitShifts(VERTEX16_UNIT));
			plane.topLeft.z = center.z - int16_t(int32_t(right.z) * halfWidth >> GetBitShifts(VERTEX16_UNIT)) - int16_t(int32_t(up.z) * halfHeight >> GetBitShifts(VERTEX16_UNIT));

			// Top-right corner
			plane.topRight.x = center.x + int16_t(int32_t(right.x) * halfWidth >> GetBitShifts(VERTEX16_UNIT)) - int16_t(int32_t(up.x) * halfHeight >> GetBitShifts(VERTEX16_UNIT));
			plane.topRight.y = center.y + int16_t(int32_t(right.y) * halfWidth >> GetBitShifts(VERTEX16_UNIT)) - int16_t(int32_t(up.y) * halfHeight >> GetBitShifts(VERTEX16_UNIT));
			plane.topRight.z = center.z + int16_t(int32_t(right.z) * halfWidth >> GetBitShifts(VERTEX16_UNIT)) - int16_t(int32_t(up.z) * halfHeight >> GetBitShifts(VERTEX16_UNIT));

			// Bottom-right corner
			plane.bottomRight.x = center.x + int16_t(int32_t(right.x) * halfWidth >> GetBitShifts(VERTEX16_UNIT)) + int16_t(int32_t(up.x) * halfHeight >> GetBitShifts(VERTEX16_UNIT));
			plane.bottomRight.y = center.y + int16_t(int32_t(right.y) * halfWidth >> GetBitShifts(VERTEX16_UNIT)) + int16_t(int32_t(up.y) * halfHeight >> GetBitShifts(VERTEX16_UNIT));
			plane.bottomRight.z = center.z + int16_t(int32_t(right.z) * halfWidth >> GetBitShifts(VERTEX16_UNIT)) + int16_t(int32_t(up.z) * halfHeight >> GetBitShifts(VERTEX16_UNIT));

			// Bottom-left corner
			plane.bottomLeft.x = center.x - int16_t(int32_t(right.x) * halfWidth >> GetBitShifts(VERTEX16_UNIT)) + int16_t(int32_t(up.x) * halfHeight >> GetBitShifts(VERTEX16_UNIT));
			plane.bottomLeft.y = center.y - int16_t(int32_t(right.y) * halfWidth >> GetBitShifts(VERTEX16_UNIT)) + int16_t(int32_t(up.y) * halfHeight >> GetBitShifts(VERTEX16_UNIT));
			plane.bottomLeft.z = center.z - int16_t(int32_t(right.z) * halfWidth >> GetBitShifts(VERTEX16_UNIT)) + int16_t(int32_t(up.z) * halfHeight >> GetBitShifts(VERTEX16_UNIT));
		}
	};
}
#endif