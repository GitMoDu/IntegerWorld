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

	struct transform32_rotate_t
	{
		fraction32_t CosX{};
		fraction32_t SinX{};
		fraction32_t CosY{};
		fraction32_t SinY{};
		fraction32_t CosZ{};
		fraction32_t SinZ{};
	};

	struct transform16_rotate_translate_t : public transform16_rotate_t
	{
		vertex16_t Translation{};
	};

	struct transform16_scale_rotate_translate_t : public transform16_rotate_translate_t
	{
		resize16_t Resize = RESIZE16_1X;
	};

	struct transform32_rotate_translate_t : public transform32_rotate_t
	{
		vertex16_t Translation{};
	};

	struct transform32_scale_rotate_translate_t : public transform32_rotate_translate_t
	{
		resize16_t Resize = RESIZE16_1X;
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

	static void CalculateTransformRotation(transform32_rotate_t& transform, const angle_t angleX, const angle_t angleY, const angle_t angleZ)
	{
		transform.CosX = Cosine32(angleX);
		transform.SinX = Sine32(angleX);
		transform.CosY = Cosine32(angleY);
		transform.SinY = Sine32(angleY);
		transform.CosZ = Cosine32(angleZ);
		transform.SinZ = Sine32(angleZ);
	}

	static void CalculateTransformRotation(transform32_rotate_t& transform, const rotation_angle_t rotation)
	{
		transform.CosX = Cosine32(rotation.x);
		transform.SinX = Sine32(rotation.x);
		transform.CosY = Cosine32(rotation.y);
		transform.SinY = Sine32(rotation.y);
		transform.CosZ = Cosine32(rotation.z);
		transform.SinZ = Sine32(rotation.z);
	}

	static void ApplyTransform(const transform16_rotate_translate_t& transform, vertex16_t& vertex)
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

		// Apply translation.
		vertex.x += transform.Translation.x;
		vertex.y += transform.Translation.y;
		vertex.z += transform.Translation.z;
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

	static void ApplyRotation(const transform32_rotate_translate_t& transform, vertex16_t& vertex)
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

	static void ApplyCameraTransform(const transform32_rotate_translate_t& transform, vertex16_t& vertex)
	{
		static constexpr int16_t CameraShift = VERTEX16_UNIT;

		vertex.z += CameraShift;

		// Apply translation.
		vertex.x += transform.Translation.x;
		vertex.y += transform.Translation.y;
		vertex.z += transform.Translation.z;

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

		vertex.z -= CameraShift;
	}

	static void ApplyTransform(const transform32_rotate_translate_t& transform, vertex16_t& vertex)
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

		// Apply translation.
		vertex.x += transform.Translation.x;
		vertex.y += transform.Translation.y;
		vertex.z += transform.Translation.z;
	}

	static void ApplyTransform(const transform32_scale_rotate_translate_t& transform, vertex16_t& vertex)
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

	static void ApplyRotate(const transform32_scale_rotate_translate_t& transform, vertex16_t& vertex)
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

	static void ApplyRotate(const transform16_scale_rotate_translate_t& transform, vertex16_t& vertex)
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
}
#endif