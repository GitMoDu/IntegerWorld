#ifndef _INTEGER_WORLD_MODEL_h
#define _INTEGER_WORLD_MODEL_h

#include <IntegerSignal.h>
#include <IntegerTrigonometry16.h>

#include "ColorFraction.h"
#include "Vertex.h"
#include "Transform.h"

namespace IntegerWorld
{
	using namespace ColorFraction;

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
		color_fraction16_t color;
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

	struct triangle_fragment_t : base_fragment_t
	{
		vertex16_t world;
		vertex16_t normal;
		vertex16_t triangleScreenA;
		vertex16_t triangleScreenB;
		vertex16_t triangleScreenC;
	};

	struct flat_background_fragment_t : base_fragment_t
	{
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

	/// <summary>
	/// Minimal render information for each frame.
	/// </summary>
	struct render_status_struct
	{
		uint32_t Render = 0;
		uint32_t Rasterize = 0;
		uint16_t FragmentsDrawn = 0;

		uint32_t GetRenderDuration() const
		{
			return Render;
		}

		void Clear()
		{
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
		uint32_t FramePreparation = 0;
		uint32_t VertexShade = 0;
		uint32_t WorldShade = 0;
		uint32_t ScreenShade = 0;
		uint32_t CameraTransform = 0;
		uint32_t ScreenProject = 0;
		uint32_t FragmentCollect = 0;
		uint32_t FragmentSort = 0;
		uint32_t RasterizeWait = 0;
		uint32_t Rasterize = 0;

		uint16_t WorldShades = 0;
		uint16_t VertexShades = 0;
		uint16_t CameraTransforms = 0;
		uint16_t ScreenProjects = 0;
		uint16_t ScreenShades = 0;

		uint16_t FragmentsDrawn = 0;

		uint32_t GetRenderDuration() const
		{
			return FramePreparation +
				VertexShade +
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
			VertexShades = 0;
			WorldShades = 0;
			CameraTransforms = 0;
			ScreenProjects = 0;
			ScreenShades = 0;

			FramePreparation = 0;
			VertexShade = 0;
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