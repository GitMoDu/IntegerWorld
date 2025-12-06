#ifndef _INTEGER_WORLD_MODEL_h
#define _INTEGER_WORLD_MODEL_h

#include <IntegerSignal.h>
#include <IntegerTrigonometry16.h>

#include "Vertex.h"
#include "Transform.h"

namespace IntegerWorld
{
	using namespace IntegerSignal::FixedPoint::ScalarFraction;
	using namespace IntegerSignal::FixedPoint::FactorScale;

	/// <summary>
	/// Specifies modes for frustum culling to determine which scene elements are considered for rendering.
	/// </summary>
	enum class FrustumCullingEnum : uint8_t
	{
		// No frustum culling is applied; all objects and primitives are considered for rendering.
		NoCulling,

		// Frustum culling is applied at the object level; entire objects outside the frustum are not rendered.
		ObjectCulling,

		// Frustum culling is applied at the primitive level; individual primitives outside the frustum are not rendered.
		PrimitiveCulling
	};

	/// <summary>
	/// Mesh culling modes applied after projection based on face orientation.
	/// </summary>
	enum class FaceCullingEnum : uint8_t
	{
		// Triangles whose normal is facing away from the camera are not drawn.
		BackfaceCulling,

		// All triangles are drawn, regardless of orientation.
		NoCulling,

		// Triangles whose normal is facing towards the camera are not drawn.
		FrontfaceCulling
	};

	/// <summary>
	/// Billboard scale mode defines how its size is calculated relative to the camera.
	/// </summary>
	enum class BillboardScaleModeEnum : uint8_t
	{
		// Fixed pixel size
		ScreenSpace,

		// Scales with distance.
		WorldSpace
	};

	/// <summary>
	/// Texture dimensions as powers of two.
	/// </summary>
	enum class TextureDimensionEnum : uint8_t
	{
		Dimension1 = 0,
		Dimension2 = 1,
		Dimension4 = 2,
		Dimension8 = 3,
		Dimension16 = 4,
		Dimension32 = 5,
		Dimension64 = 6,
		Dimension128 = 7,
		Dimension256 = 8
	};

	struct material_t
	{
		// Surface emission color intensity.
		ufraction8_t Emit;

		// Surface roughness affecting light scattering.
		ufraction8_t Rough;

		// Surface shininess affecting specular intensity.
		ufraction8_t Shine;

		// Surface glossiness affecting specular and fresnel focus.
		ufraction8_t Gloss;

		// Specular tinting towards albedo color at low specular angles.
		ufraction8_t SpecularTint;

		// Fresnel control for energy redistribution between diffuse and specular at grazing angles.
		fraction8_t Fresnel;
	};


	/// <summary>
	/// Compile-time texture size struct that computes power-of-two Width and Height from enum-based dimension exponents.
	/// </summary>
	/// <typeparam name="widthDimension">A TextureDimensionEnum value used as the exponent for the width.</typeparam>
	/// <typeparam name="heightDimension">A TextureDimensionEnum value used as the exponent for the height</typeparam>
	template<TextureDimensionEnum widthDimension,
		TextureDimensionEnum heightDimension>
	struct TemplateTextureSize
	{
		// Power-of-two texture dimensions derived from enum values.
		static constexpr uint16_t Width = static_cast<uint16_t>(1) << static_cast<uint8_t>(widthDimension);
		static constexpr uint16_t Height = static_cast<uint16_t>(1) << static_cast<uint8_t>(heightDimension);
	};

	// Common texture size presets.
	using TextureSize8x8 = TemplateTextureSize<TextureDimensionEnum::Dimension8, TextureDimensionEnum::Dimension8>;
	using TextureSize16x16 = TemplateTextureSize<TextureDimensionEnum::Dimension16, TextureDimensionEnum::Dimension16>;
	using TextureSize32x32 = TemplateTextureSize<TextureDimensionEnum::Dimension32, TextureDimensionEnum::Dimension32>;
	using TextureSize64x64 = TemplateTextureSize<TextureDimensionEnum::Dimension64, TextureDimensionEnum::Dimension64>;
	using TextureSize128x128 = TemplateTextureSize<TextureDimensionEnum::Dimension128, TextureDimensionEnum::Dimension128>;
	using TextureSize64x128 = TemplateTextureSize<TextureDimensionEnum::Dimension64, TextureDimensionEnum::Dimension128>;
	using TextureSize128x64 = TemplateTextureSize<TextureDimensionEnum::Dimension128, TextureDimensionEnum::Dimension64>;
	using TextureSize256x256 = TemplateTextureSize<TextureDimensionEnum::Dimension256, TextureDimensionEnum::Dimension256>;

	struct edge_line_t
	{
		uint16_t a;
		uint16_t b;
	};

	struct triangle_face_t
	{
		uint16_t a;
		uint16_t b;
		uint16_t c;
	};

	struct uv_t
	{
		uint8_t x;
		uint8_t y;
	};

	struct triangle_uv_t
	{
		uv_t a;
		uv_t b;
		uv_t c;
	};

	struct billboard_fragment_t
	{
		int16_t topLeftX;
		int16_t topLeftY;
		int16_t bottomRightX;
		int16_t bottomRightY;

		int16_t z;
		uint8_t red;
		uint8_t green;
		uint8_t blue;
	};

	struct point_cloud_fragment_t
	{
		uint16_t index;

		int16_t x;
		int16_t y;
		int16_t z;

		uint8_t red;
		uint8_t green;
		uint8_t blue;
	};

	struct edge_line_fragment_t
	{
		vertex16_t vertexA;
		vertex16_t vertexB;

		uint16_t index;
		int16_t z;

		uint8_t red;
		uint8_t green;
		uint8_t blue;
	};

	struct edge_vertex_fragment_t
	{
		vertex16_t vertexA;
		vertex16_t vertexB;

		uint16_t index;
		int16_t z;

		uint8_t redA;
		uint8_t greenA;
		uint8_t blueA;

		uint8_t redB;
		uint8_t greenB;
		uint8_t blueB;
	};

	struct mesh_triangle_fragment_t
	{
		vertex16_t vertexA;
		vertex16_t vertexB;
		vertex16_t vertexC;

		uv_t uvA;
		uv_t uvB;
		uv_t uvC;

		uint16_t index;
		int16_t z;

		uint8_t red;
		uint8_t green;
		uint8_t blue;
	};

	struct mesh_vertex_fragment_t
	{
		vertex16_t vertexA;
		vertex16_t vertexB;
		vertex16_t vertexC;

		uv_t uvA;
		uv_t uvB;
		uv_t uvC;

		uint16_t index;
		int16_t z;

		uint8_t redA;
		uint8_t greenA;
		uint8_t blueA;

		uint8_t redB;
		uint8_t greenB;
		uint8_t blueB;

		uint8_t redC;
		uint8_t greenC;
		uint8_t blueC;
	};

	struct color_fragment_t
	{
		uint8_t red;
		uint8_t green;
		uint8_t blue;
		int16_t z;
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
		int32_t radiusSquared;

		bool IsPointInside(const vertex16_t& point, const uint16_t planeTolerance = VERTEX16_UNIT / 16) const
		{
			// Sphere culling - early distance check.
			{
				const int16_t dx = point.x - origin.x;
				const int16_t dy = point.y - origin.y;
				const int16_t dz = point.z - origin.z;
				const int32_t squareDistance = (static_cast<int32_t>(dx) * dx) + (static_cast<int32_t>(dy) * dy) + (static_cast<int32_t>(dz) * dz);

				// If point is outside the bounding sphere, it's definitely outside the frustum.
				if (squareDistance > radiusSquared)
					return false;
			}

			// Check against near plane. Z axis points forward, so point must be in front of near plane.
			if (PlaneDistanceToPoint(cullingNearPlane, point) < 0)
				return false;

			// Check against left plane. Point must be on the "inside" side of the plane.
			if (PlaneDistanceToPoint(cullingLeftPlane, point) > static_cast<int16_t>(planeTolerance))
				return false;

			// Check against right plane. Point must be on the "inside" side of the plane.
			if (PlaneDistanceToPoint(cullingRightPlane, point) > static_cast<int16_t>(planeTolerance))
				return false;

			// Check against top plane. Point must be on the "inside" side of the plane.
			if (PlaneDistanceToPoint(cullingTopPlane, point) > static_cast<int16_t>(planeTolerance))
				return false;

			// Check against bottom plane. Point must be on the "inside" side of the plane.
			if (PlaneDistanceToPoint(cullingBottomPlane, point) > static_cast<int16_t>(planeTolerance))
				return false;

			// If it passed all plane tests, the point is inside the frustum.
			return true;
		}

	private:
		static int16_t PlaneDistanceToPoint(const plane16_t& plane, const vertex16_t& point)
		{
			// Calculate dot product between normal and point, normalized by VERTEX16_UNIT.
			const int32_t dotProduct = SignedRightShift(DotProduct16(plane, point), GetBitShifts(VERTEX16_UNIT));

			// Add the plane distance (which is negative of dot(normal, planePoint))
			return dotProduct + plane.distance;
		}
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