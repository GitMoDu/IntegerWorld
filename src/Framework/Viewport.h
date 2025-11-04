#ifndef _INTEGER_WORLD_VIEWPORT_h
#define _INTEGER_WORLD_VIEWPORT_h

#include "../Framework/Model.h"

namespace IntegerWorld
{
	class ViewportProjector
	{
	private:
		static constexpr uint16_t RangeMin = VERTEX16_UNIT;
		static constexpr uint16_t RangeMax = DRAW_DISTANCE_MAX;

		static constexpr uint16_t Range = RangeMax - RangeMin;
		static constexpr uint8_t DownShift = GetBitShifts(Range);
		static constexpr uint8_t UnitShift = GetBitShifts(VERTEX16_UNIT);

	private:
		// Near plane corners in world space.
		vertex16_t topLeft{};
		vertex16_t topRight{};
		vertex16_t bottomLeft{};
		vertex16_t bottomRight{};

		// Camera-space basis vectors in world space.
		vertex16_t forward{};
		vertex16_t right{};
		vertex16_t up{};

	private:
		int16_t ViewWidthHalf = 0;
		int16_t ViewHeightHalf = 0;

		int16_t verticalNum = 0;
		int16_t verticalDenum = 0;

		uint16_t distanceNum = (static_cast<uint32_t>(RangeMin) + RangeMax) / 2;

		uint16_t drawDistance = RangeMax;

		uint8_t frustumShifts = 5;

	public:
		ViewportProjector() {}

		void SetDimensions(const uint16_t viewWidth, const uint16_t viewHeight)
		{
			verticalNum = viewWidth;
			verticalDenum = viewHeight;
			ViewWidthHalf = MinValue<uint16_t>(INT16_MAX, static_cast<uint16_t>(viewWidth) >> 1);
			ViewHeightHalf = MinValue<uint16_t>(INT16_MAX, static_cast<uint16_t>(viewHeight) >> 1);
			frustumShifts = UnitShift - GetBitShifts(MaxValue<uint16_t>(viewWidth, viewHeight) / 2);
		}

		/// <summary>
		/// </summary>
		/// <param name="fovFraction">0 -> Minimum FoV; UFraction16::FRACTION_1X -> Max FoV.</param>
		void SetFov(const ufraction16_t fovFraction)
		{
			distanceNum = RangeMin + Fraction(static_cast<ufraction16_t>(UFRACTION16_1X - fovFraction), Range);
		}

		void SetDrawDistance(const uint16_t distance)
		{
			drawDistance = MinValue<uint16_t>(RangeMax, distance);
		}

		uint16_t GetFocalDistance() const
		{
			return distanceNum;
		}

		/// <summary>
		/// Initializes a frustum structure based on the current camera state, computing its origin, orientation, culling radius, and culling planes for view frustum culling operations.
		/// Note: inner scoping is done to avoid deep stack usage.
		/// </summary>
		/// <param name="cameraControls">A constant reference to the camera state, providing the camera's position and rotation.</param>
		/// <param name="frustum">A reference to the frustum structure to be populated with the computed frustum parameters.</param>
		void GetFrustum(const camera_state_t& cameraControls, frustum_t& frustum)
		{
			// Set frustum culling radius squared.
			frustum.radiusSquared = (static_cast<uint32_t>(drawDistance) * drawDistance);

			// Set frustum origin to apparent camera position.
			frustum.origin = cameraControls.Position;

			// Set the camera rotation.
			frustum.rotation = cameraControls.Rotation;

			// Calculate camera basis vectors in world space.
			{
				// Build camera rotation cos/sin transform.
				transform16_rotate_t camRot{};
				CalculateTransformRotation(camRot, cameraControls.Rotation);

				// Camera-space basis vectors (forward, right, up)
				forward = { 0, 0, VERTEX16_UNIT };
				right = { VERTEX16_UNIT, 0, 0 };
				up = { 0, VERTEX16_UNIT, 0 };

				// Rotate them into world space.
				ApplyTransformRotation(camRot, forward);
				ApplyTransformRotation(camRot, right);
				ApplyTransformRotation(camRot, up);
			}

			// Calculate the four corners of the near plane.
			{
				// Near distance.
				const int16_t nearDist = distanceNum >> frustumShifts;

				// Calculate plane center.
				const vertex16_t nearCenter
				{
					static_cast<int16_t>(frustum.origin.x + static_cast<int16_t>(SignedRightShift(static_cast<int32_t>(forward.x) * nearDist, DownShift))),
					static_cast<int16_t>(frustum.origin.y + static_cast<int16_t>(SignedRightShift(static_cast<int32_t>(forward.y) * nearDist, DownShift))),
					static_cast<int16_t>(frustum.origin.z + static_cast<int16_t>(SignedRightShift(static_cast<int32_t>(forward.z) * nearDist, DownShift)))
				};

				// Calculate corners.
				CalculateNearPlaneCorners(nearCenter, right, up, topLeft, topRight, bottomLeft, bottomRight);
			}

			// Calculate culling planes.
			// Near plane (facing inside the frustum)
			CalculatePlane(topLeft, topRight, bottomLeft, frustum.cullingNearPlane);

			// Left plane (using origin, top-left and bottom-left)
			CalculatePlane(frustum.origin, topLeft, bottomLeft, frustum.cullingLeftPlane);

			// Right plane (using origin, bottom-right and top-right)
			CalculatePlane(frustum.origin, bottomRight, topRight, frustum.cullingRightPlane);

			// Top plane (using origin, top-right and top-left)
			CalculatePlane(frustum.origin, topRight, topLeft, frustum.cullingTopPlane);

			// Bottom plane (using origin, bottom-left and bottom-right)
			CalculatePlane(frustum.origin, bottomLeft, bottomRight, frustum.cullingBottomPlane);
		}

		void Project(vertex16_t& cameraToscreen)
		{
			const int32_t distanceDenum = static_cast<int32_t>(distanceNum) + cameraToscreen.z;

			int32_t ix, iy;
			if (distanceDenum == 0)
			{
				ix = SignedRightShift(static_cast<int32_t>(cameraToscreen.x) * ViewWidthHalf, DownShift);

				iy = (static_cast<int32_t>(cameraToscreen.y) * verticalNum) / verticalDenum;
				iy = SignedRightShift(iy * ViewHeightHalf, DownShift);
			}
			else
			{
				ix = (static_cast<int32_t>(cameraToscreen.x) * distanceNum) / distanceDenum;
				ix = SignedRightShift(ix * ViewWidthHalf, DownShift);

				iy = (static_cast<int32_t>(cameraToscreen.y) * distanceNum) / distanceDenum;
				iy = (iy * verticalNum) / verticalDenum;
				iy = SignedRightShift(iy * ViewHeightHalf, DownShift);
			}

			// Convert to screen space (top-left is (0,0).
			cameraToscreen.x = ViewWidthHalf - static_cast<int16_t>(ix);
			cameraToscreen.y = ViewHeightHalf - static_cast<int16_t>(iy);
			cameraToscreen.z = distanceDenum;
		}

	private:
		/// <summary>
		/// Calculate the four corners of the plane using the center point and the right/up vectors
		/// </summary>
		void CalculateNearPlaneCorners(const vertex16_t& center, const vertex16_t& right, const vertex16_t& up,
			vertex16_t& topLeft, vertex16_t& topRight, vertex16_t& bottomLeft, vertex16_t& bottomRight)
		{
			// Top-left corner.
			topLeft.x = center.x - static_cast<int16_t>(SignedRightShift(static_cast<int32_t>(right.x) * ViewWidthHalf, UnitShift))
				- static_cast<int16_t>(SignedRightShift(static_cast<int32_t>(up.x) * ViewHeightHalf, UnitShift));
			topLeft.y = center.y - static_cast<int16_t>(SignedRightShift(static_cast<int32_t>(right.y) * ViewWidthHalf, UnitShift))
				- static_cast<int16_t>(SignedRightShift(static_cast<int32_t>(up.y) * ViewHeightHalf, UnitShift));
			topLeft.z = center.z - static_cast<int16_t>(SignedRightShift(static_cast<int32_t>(right.z) * ViewWidthHalf, UnitShift))
				- static_cast<int16_t>(SignedRightShift(static_cast<int32_t>(up.z) * ViewHeightHalf, UnitShift));

			// Top-right corner.
			topRight.x = center.x + static_cast<int16_t>(SignedRightShift(static_cast<int32_t>(right.x) * ViewWidthHalf, UnitShift))
				- static_cast<int16_t>(SignedRightShift(static_cast<int32_t>(up.x) * ViewHeightHalf, UnitShift));
			topRight.y = center.y + static_cast<int16_t>(SignedRightShift(static_cast<int32_t>(right.y) * ViewWidthHalf, UnitShift))
				- static_cast<int16_t>(SignedRightShift(static_cast<int32_t>(up.y) * ViewHeightHalf, UnitShift));
			topRight.z = center.z + static_cast<int16_t>(SignedRightShift(static_cast<int32_t>(right.z) * ViewWidthHalf, UnitShift))
				- static_cast<int16_t>(SignedRightShift(static_cast<int32_t>(up.z) * ViewHeightHalf, UnitShift));

			// Bottom-right corner.
			bottomRight.x = center.x + static_cast<int16_t>(SignedRightShift(static_cast<int32_t>(right.x) * ViewWidthHalf, UnitShift))
				+ static_cast<int16_t>(SignedRightShift(static_cast<int32_t>(up.x) * ViewHeightHalf, UnitShift));
			bottomRight.y = center.y + static_cast<int16_t>(SignedRightShift(static_cast<int32_t>(right.y) * ViewWidthHalf, UnitShift))
				+ static_cast<int16_t>(SignedRightShift(static_cast<int32_t>(up.y) * ViewHeightHalf, UnitShift));
			bottomRight.z = center.z + static_cast<int16_t>(SignedRightShift(static_cast<int32_t>(right.z) * ViewWidthHalf, UnitShift))
				+ static_cast<int16_t>(SignedRightShift(static_cast<int32_t>(up.z) * ViewHeightHalf, UnitShift));

			// Bottom-left corner.
			bottomLeft.x = center.x - static_cast<int16_t>(SignedRightShift(static_cast<int32_t>(right.x) * ViewWidthHalf, UnitShift))
				+ static_cast<int16_t>(SignedRightShift(static_cast<int32_t>(up.x) * ViewHeightHalf, UnitShift));
			bottomLeft.y = center.y - static_cast<int16_t>(SignedRightShift(static_cast<int32_t>(right.y) * ViewWidthHalf, UnitShift))
				+ static_cast<int16_t>(SignedRightShift(static_cast<int32_t>(up.y) * ViewHeightHalf, UnitShift));
			bottomLeft.z = center.z - static_cast<int16_t>(SignedRightShift(static_cast<int32_t>(right.z) * ViewWidthHalf, UnitShift))
				+ static_cast<int16_t>(SignedRightShift(static_cast<int32_t>(up.z) * ViewHeightHalf, UnitShift));
		}

		/// <summary>
		/// Calculate a plane from three points
		/// </summary>
		void CalculatePlane(const vertex16_t& a, const vertex16_t& b, const vertex16_t& c, plane16_t& plane)
		{
			vertex16_t normal16;
			{
				// Calculate two vectors in the plane.
				const vertex16_t v1
				{
					static_cast<int16_t>(b.x - a.x),
					static_cast<int16_t>(b.y - a.y),
					static_cast<int16_t>(b.z - a.z)
				};

				const vertex16_t v2
				{
					static_cast<int16_t>(c.x - a.x),
					static_cast<int16_t>(c.y - a.y),
					static_cast<int16_t>(c.z - a.z)
				};

				// Calculate the normal using cross product.
				int32_t normalX = (static_cast<int32_t>(v1.y) * v2.z) - (static_cast<int32_t>(v1.z) * v2.y);
				int32_t normalY = (static_cast<int32_t>(v1.z) * v2.x) - (static_cast<int32_t>(v1.x) * v2.z);
				int32_t normalZ = (static_cast<int32_t>(v1.x) * v2.y) - (static_cast<int32_t>(v1.y) * v2.x);

				// Reduce to 16 bit vertex by dividing by 2 until all values are in range.
				while (normalX < INT16_MIN
					|| normalX > INT16_MAX
					|| normalY < INT16_MIN
					|| normalY > INT16_MAX
					|| normalZ < INT16_MIN
					|| normalZ > INT16_MAX)
				{
					normalX = SignedRightShift(normalX, 1);
					normalY = SignedRightShift(normalY, 1);
					normalZ = SignedRightShift(normalZ, 1);
				}

				normal16.x = static_cast<int16_t>(normalX);
				normal16.y = static_cast<int16_t>(normalY);
				normal16.z = static_cast<int16_t>(normalZ);
			}

			// Normalize the normal vector.
			NormalizeVertex16(normal16);
			plane.x = normal16.x;
			plane.y = normal16.y;
			plane.z = normal16.z;

			// Calculate the plane distance: -dot(normal, point).
			plane.distance = -static_cast<int32_t>(SignedRightShift((normal16.x * a.x) +
				(normal16.y * a.y) +
				(normal16.z * a.z), UnitShift));
		}
	};
}
#endif