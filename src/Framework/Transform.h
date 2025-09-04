#ifndef _INTEGER_WORLD_VERTEX_TRANSFORM_h
#define _INTEGER_WORLD_VERTEX_TRANSFORM_h

#include "Vertex.h"

#include <IntegerSignal.h>
#include <IntegerTrigonometry16.h>

namespace IntegerWorld
{
	using namespace IntegerSignal::Trigonometry;
	using namespace IntegerSignal::Fraction;
	using namespace IntegerSignal::Resize;

	struct rotation_angle_t
	{
		angle_t x;
		angle_t y;
		angle_t z;
	};

	struct transform16_rotate_t
	{
		fraction16_t CosX{};
		fraction16_t SinX{};
		fraction16_t CosY{};
		fraction16_t SinY{};
		fraction16_t CosZ{};
		fraction16_t SinZ{};
	};

	struct transform16_rotate_translate_t : transform16_rotate_t
	{
		vertex16_t Translation{};
	};

	struct transform16_scale_translate_t : transform16_rotate_t
	{
		resize16_t Resize = RESIZE16_1X;
	};

	struct transform16_scale_rotate_translate_t : transform16_rotate_translate_t
	{
		resize16_t Resize = RESIZE16_1X;
	};

	struct transform16_camera_t : transform16_rotate_translate_t
	{
		uint16_t ViewDistance;
	};

	static void CalculateTransformRotation(transform16_rotate_t& transform, const angle_t angleX, const angle_t angleY, const angle_t angleZ)
	{
		transform.CosX = Cosine16(angleX);
		transform.SinX = Sine16(angleX);
		transform.CosY = Cosine16(angleY);
		transform.SinY = Sine16(angleY);
		transform.CosZ = Cosine16(angleZ);
		transform.SinZ = Sine16(angleZ);
	}

	static void CalculateTransformRotation(transform16_rotate_t& transform, const rotation_angle_t rotation)
	{
		transform.CosX = Cosine16(rotation.x);
		transform.SinX = Sine16(rotation.x);
		transform.CosY = Cosine16(rotation.y);
		transform.SinY = Sine16(rotation.y);
		transform.CosZ = Cosine16(rotation.z);
		transform.SinZ = Sine16(rotation.z);
	}

	static void ApplyTransform(const transform16_rotate_t& transform, vertex16_t& vertex)
	{
		// Rotation around X-axis
		const int16_t y1 = vertex.y;
		vertex.y = Fraction::Scale(transform.CosX, y1) - Fraction::Scale(transform.SinX, vertex.z);
		vertex.z = Fraction::Scale(transform.SinX, y1) + Fraction::Scale(transform.CosX, vertex.z);

		// Rotation around Y-axis
		const int16_t x1 = vertex.x;
		vertex.x = Fraction::Scale(transform.CosY, x1) + Fraction::Scale(transform.SinY, vertex.z);
		vertex.z = -Fraction::Scale(transform.SinY, x1) + Fraction::Scale(transform.CosY, vertex.z);

		// Rotation around Z-axis
		const int16_t x2 = vertex.x;
		vertex.x = Fraction::Scale(transform.CosZ, x2) - Fraction::Scale(transform.SinZ, vertex.y);
		vertex.y = Fraction::Scale(transform.SinZ, x2) + Fraction::Scale(transform.CosZ, vertex.y);
	}

	static void ApplyTransform(const transform16_scale_rotate_translate_t& transform, vertex16_t& vertex)
	{
		// Scale geometry.
		vertex.x = Resize::Scale(transform.Resize, vertex.x);
		vertex.y = Resize::Scale(transform.Resize, vertex.y);
		vertex.z = Resize::Scale(transform.Resize, vertex.z);

		// Rotation around X-axis
		const int16_t y1 = vertex.y;
		vertex.y = Fraction::Scale(transform.CosX, y1) - Fraction::Scale(transform.SinX, vertex.z);
		vertex.z = Fraction::Scale(transform.SinX, y1) + Fraction::Scale(transform.CosX, vertex.z);

		// Rotation around Y-axis
		const int16_t x1 = vertex.x;
		vertex.x = Fraction::Scale(transform.CosY, x1) + Fraction::Scale(transform.SinY, vertex.z);
		vertex.z = -Fraction::Scale(transform.SinY, x1) + Fraction::Scale(transform.CosY, vertex.z);

		// Rotation around Z-axis
		const int16_t x2 = vertex.x;
		vertex.x = Fraction::Scale(transform.CosZ, x2) - Fraction::Scale(transform.SinZ, vertex.y);
		vertex.y = Fraction::Scale(transform.SinZ, x2) + Fraction::Scale(transform.CosZ, vertex.y);

		// Apply translation.
		vertex.x += transform.Translation.x;
		vertex.y += transform.Translation.y;
		vertex.z += transform.Translation.z;
	}

	static void ApplyTransformRotation(const transform16_rotate_t& transform, vertex16_t& vertex)
	{
		ApplyTransform(transform, vertex);
	}

	static void ApplyTransformRotation(const transform16_scale_rotate_translate_t& transform, vertex16_t& vertex)
	{
		ApplyTransform(static_cast<const transform16_rotate_t&>(transform), vertex);
	}

	static void ApplyInverseCameraRotation(const transform16_rotate_t& transform, vertex16_t& vertex)
	{
		// Rotation around Z-axis (Roll - tilting the view)
		const int16_t x2 = vertex.x;
		vertex.x = Fraction::Scale(transform.CosZ, x2) - Fraction::Scale(transform.SinZ, vertex.y);
		vertex.y = Fraction::Scale(transform.SinZ, x2) + Fraction::Scale(transform.CosZ, vertex.y);

		// Rotation around X-axis (Pitch - looking up/down)
		const int16_t y1 = vertex.y;
		vertex.y = Fraction::Scale(transform.CosX, y1) - Fraction::Scale(transform.SinX, vertex.z);
		vertex.z = Fraction::Scale(transform.SinX, y1) + Fraction::Scale(transform.CosX, vertex.z);

		// Rotation around Y-axis (Yaw - looking left/right)
		const int16_t x1 = vertex.x;
		vertex.x = Fraction::Scale(transform.CosY, x1) + Fraction::Scale(transform.SinY, vertex.z);
		vertex.z = -Fraction::Scale(transform.SinY, x1) + Fraction::Scale(transform.CosY, vertex.z);
	}

	static void ApplyCameraTransform(const transform16_camera_t& transform, vertex16_t& vertex)
	{
		// Apply translation first (camera position in world)
		vertex.x += transform.Translation.x;
		vertex.y += transform.Translation.y;
		vertex.z += transform.Translation.z;

		// Rotation around Y-axis (Yaw - looking left/right)
		const int16_t x1 = vertex.x;
		vertex.x = Fraction::Scale(transform.CosY, x1) + Fraction::Scale(transform.SinY, vertex.z);
		vertex.z = -Fraction::Scale(transform.SinY, x1) + Fraction::Scale(transform.CosY, vertex.z);

		// Rotation around X-axis (Pitch - looking up/down)
		const int16_t y1 = vertex.y;
		vertex.y = Fraction::Scale(transform.CosX, y1) - Fraction::Scale(transform.SinX, vertex.z);
		vertex.z = Fraction::Scale(transform.SinX, y1) + Fraction::Scale(transform.CosX, vertex.z);

		// Rotation around Z-axis (Roll - tilting the view)
		const int16_t x2 = vertex.x;
		vertex.x = Fraction::Scale(transform.CosZ, x2) - Fraction::Scale(transform.SinZ, vertex.y);
		vertex.y = Fraction::Scale(transform.SinZ, x2) + Fraction::Scale(transform.CosZ, vertex.y);

		// Apply view distance
		vertex.z -= transform.ViewDistance;
	}
}
#endif