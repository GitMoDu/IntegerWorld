#ifndef _INTEGER_WORLD_MODEL_h
#define _INTEGER_WORLD_MODEL_h

#include <IntegerSignal.h>
#include <IntegerTrigonometry16.h>

#include "Vertex.h"
#include "Transform.h"

namespace IntegerWorld
{
	struct edge_line_t
	{
		uint16_t start;
		uint16_t end;
	};

	struct triangle_face_t
	{
		uint16_t v1;
		uint16_t v2;
		uint16_t v3;
	};

	struct material_t
	{
		ufraction8_t Emissive;
		ufraction8_t Diffuse;
		ufraction8_t Specular;
		ufraction8_t Metallic;
	};

	struct world_position_shade_t
	{
		vertex16_t positionWorld;
	};

	struct world_position_normal_shade_t : world_position_shade_t
	{
		vertex16_t normalWorld;
	};

	struct base_fragment_t
	{
		material_t material;
		Rgb8::color_t color;
	};

	struct point_fragment_t : base_fragment_t
	{
		vertex16_t world;
		vertex16_t screen;
	};

	struct point_normal_fragment_t : point_fragment_t
	{
		vertex16_t normal;
	};

	struct edge_fragment_t : base_fragment_t
	{
		vertex16_t world;
		vertex16_t start;
		vertex16_t end;
	};

	struct billboard_fragment_t
	{
		vertex16_t world;
		int16_t topLeftX;
		int16_t topLeftY;
		int16_t bottomRightX;
		int16_t bottomRightY;
	};

	struct triangle_fragment_t : base_fragment_t
	{
		vertex16_t world;
		vertex16_t normalWorld;
		vertex16_t triangleScreenA;
		vertex16_t triangleScreenB;
		vertex16_t triangleScreenC;
	};

	struct flat_background_fragment_t : base_fragment_t
	{
	};

	struct base_vertex_t : vertex16_t
	{
	};

	struct normal_vertex_t : vertex16_t
	{
		vertex16_t normal;
	};

	struct color_vertex_t : vertex16_t
	{
		Rgb8::color_t color;
	};

	struct normal_color_vertex_t : vertex16_t
	{
		vertex16_t normal;
		Rgb8::color_t color;
	};

	struct mesh_vertex_t : base_vertex_t
	{
	};

	struct billboard_vertex_t : base_vertex_t
	{
	};

	struct base_primitive_t
	{
		int16_t z;
	};

	struct mesh_primitive_t : base_primitive_t
	{
	};

	struct mesh_world_primitive_t : base_primitive_t
	{
		vertex16_t worldPosition;
		vertex16_t worldNormal;
	};

	struct billboard_primitive_t : base_primitive_t
	{
		int16_t topLeftX;
		int16_t topLeftY;
		int16_t bottomRightX;
		int16_t bottomRightY;
	};

	struct camera_state_t
	{
		vertex16_t Position{};
		rotation_angle_t Rotation{};
	};

	struct ordered_fragment_t
	{
		uint16_t ObjectIndex;
		uint16_t FragmentIndex;
		int16_t Z;
	};

	struct plane16_t : vertex16_t
	{
		int16_t distance;
	};

	struct frustum_t
	{
		plane16_t cullingNearPlane;
		plane16_t cullingLeftPlane;
		plane16_t cullingRightPlane;
		plane16_t cullingTopPlane;
		plane16_t cullingBottomPlane;

		rotation_angle_t rotation;
		vertex16_t origin;
		uint32_t radiusSquared;

		bool IsPointInside(const vertex16_t& point, const uint16_t planeTolerance = VERTEX16_UNIT / 10) const
		{
			// Check against near plane. Z axis points forward, so point must be in front of near plane.
			if (PlaneDistanceToPoint(cullingNearPlane, point) < 0)
				return false;

			// Check against left plane. Point must be on the "inside" side of the plane.
			if (PlaneDistanceToPoint(cullingLeftPlane, point) - planeTolerance > 0)
				return false;

			// Check against right plane. Point must be on the "inside" side of the plane.
			if (PlaneDistanceToPoint(cullingRightPlane, point) - planeTolerance > 0)
				return false;

			// Check against top plane. Point must be on the "inside" side of the plane.
			if (PlaneDistanceToPoint(cullingTopPlane, point) - planeTolerance > 0)
				return false;

			// Check against bottom plane. Point must be on the "inside" side of the plane.
			if (PlaneDistanceToPoint(cullingBottomPlane, point) - planeTolerance > 0)
				return false;

			// Sphere culling - distance check.
			const int16_t dx = point.x - origin.x;
			const int16_t dy = point.y - origin.y;
			const int16_t dz = point.z - origin.z;
			const uint32_t squareDistance = (uint32_t(int32_t(dx) * dx) + uint32_t(int32_t(dy) * dy) + uint32_t(int32_t(dz) * dz));

			// If point is outside the bounding sphere, it's definitely outside the frustum.
			if (squareDistance > radiusSquared)
				return false;

			// If it passed all plane tests, the point is inside the frustum.
			return true;
		}

	private:
		static int32_t PlaneDistanceToPoint(const plane16_t& plane, const vertex16_t& point)
		{
			// Calculate dot product between normal and point, normalized by VERTEX16_UNIT.
			const int32_t dotProduct = SignedRightShift(DotProduct16(plane, point), GetBitShifts(VERTEX16_UNIT));

			// Add the plane distance (which is negative of dot(normal, planePoint))
			return dotProduct + plane.distance;
		}
	};

	enum class FrustumCullingEnum : uint8_t
	{
		NoCulling,
		ObjectCulling,
		PrimitiveCulling,
		ObjectAndPrimitiveCulling
	};


	/// <summary>
	/// Minimal render information for each frame.
	/// </summary>
	struct render_status_struct
	{
		uint32_t FrameDuration = 0;
		uint32_t Render = 0;
		uint32_t Rasterize = 0;
		uint16_t FragmentsDrawn = 0;

		uint32_t GetRenderDuration() const
		{
			return Render;
		}

		void Clear()
		{
			FrameDuration = 0;
			FragmentsDrawn = 0;
			Rasterize = 0;
			Render = 0;
		}
	};

	/// <summary>
	/// Full render information for each frame.
	/// </summary>
	struct render_debug_status_struct
	{
		uint32_t FrameDuration = 0;
		uint32_t FramePreparation = 0;
		uint32_t ObjectShade = 0;
		uint32_t VertexShade = 0;
		uint32_t WorldTransform = 0;
		uint32_t WorldShade = 0;
		uint32_t ScreenShade = 0;
		uint32_t CameraTransform = 0;
		uint32_t ScreenProject = 0;
		uint32_t FragmentCollect = 0;
		uint32_t FragmentSort = 0;
		uint32_t RasterizeWait = 0;
		uint32_t Rasterize = 0;

		uint16_t ObjectShades = 0;
		uint16_t VertexShades = 0;
		uint16_t WorldShades = 0;
		uint16_t WorldTransforms = 0;
		uint16_t CameraTransforms = 0;
		uint16_t ScreenProjects = 0;
		uint16_t ScreenShades = 0;

		uint16_t FragmentsDrawn = 0;

		uint32_t GetRenderDuration() const
		{
			return FramePreparation +
				ObjectShade +
				VertexShade +
				WorldTransform +
				WorldShade +
				CameraTransform +
				ScreenShade +
				ScreenProject +
				FragmentCollect +
				FragmentSort;
		}

		void Clear()
		{
			FragmentsDrawn = 0;
			ObjectShades = 0;
			WorldTransforms = 0;
			VertexShades = 0;
			WorldShades = 0;
			CameraTransforms = 0;
			ScreenProjects = 0;
			ScreenShades = 0;

			FrameDuration = 0;
			FramePreparation = 0;
			ObjectShade = 0;
			VertexShade = 0;
			WorldTransform = 0;
			WorldShade = 0;
			CameraTransform = 0;
			ScreenShade = 0;
			ScreenProject = 0;
			FragmentCollect = 0;
			FragmentSort = 0;
			RasterizeWait = 0;
			Rasterize = 0;
		}
	};
}
#endif