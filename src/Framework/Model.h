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

	struct render_status_struct
	{
		uint16_t VertexShades = 0;
		uint32_t VertexShadeDuration = 0;

		uint16_t PrimitiveWorldShades = 0;
		uint32_t PrimitiveWorldShadeDuration = 0;

		uint16_t PrimitiveScreenShades = 0;
		uint32_t PrimitiveScreenShadeDuration = 0;

		uint16_t CameraTransforms = 0;
		uint32_t CameraTransformDuration = 0;

		uint16_t ScreenProjects = 0;
		uint32_t ScreenProjectDuration = 0;

		uint32_t EngineSortDuration = 0;

		uint16_t ItemSorts = 0;
		uint32_t ItemSortDuration = 0;

		uint32_t RasterizeWaitDuration = 0;

		uint32_t EngineCameraDuration = 0;

		uint16_t Rasterizes = 0;
		uint32_t RasterizeDuration = 0;

		uint32_t GetRenderDuration() const
		{
			return VertexShadeDuration +
				PrimitiveWorldShadeDuration +
				PrimitiveScreenShadeDuration +
				CameraTransformDuration +
				ScreenProjectDuration +
				EngineSortDuration +
				EngineCameraDuration +
				ItemSortDuration;
		}

		void Clear()
		{
			PrimitiveWorldShades = 0;
			PrimitiveWorldShadeDuration = 0;
			PrimitiveScreenShades = 0;
			PrimitiveScreenShadeDuration = 0;
			VertexShades = 0;
			VertexShadeDuration = 0;
			CameraTransforms = 0;
			CameraTransformDuration = 0;
			ScreenProjects = 0;
			ScreenProjectDuration = 0;
			EngineSortDuration = 0;
			EngineCameraDuration = 0;
			ItemSorts = 0;
			ItemSortDuration = 0;
			RasterizeWaitDuration = 0;
			Rasterizes = 0;
			RasterizeDuration = 0;
		}
	};
}
#endif