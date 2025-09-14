#ifndef _INTEGER_WORLD_VERTEX_TRANSFORM_h
#define _INTEGER_WORLD_VERTEX_TRANSFORM_h

#include "Vertex.h"

#include <IntegerSignal.h>
#include <IntegerTrigonometry16.h>

namespace IntegerWorld
{
	using namespace IntegerSignal::Trigonometry;
	using namespace IntegerSignal::FixedPoint::FactorScale;
	using namespace IntegerSignal::FixedPoint::ScalarFraction;

	struct rotation_angle_t
	{
		angle_t x;
		angle_t y;
		angle_t z;
	};

	struct transform16_rotate_t
	{
		fraction16_t CosX;
		fraction16_t SinX;
		fraction16_t CosY;
		fraction16_t SinY;
		fraction16_t CosZ;
		fraction16_t SinZ;
	};

	struct transform16_rotate_translate_t : transform16_rotate_t
	{
		vertex16_t Translation;
	};

	struct transform16_scale_translate_t : transform16_rotate_t
	{
		scale16_t Resize = Scale16::SCALE_1X;
	};

	struct transform16_scale_rotate_translate_t : transform16_rotate_translate_t
	{
		scale16_t Resize = Scale16::SCALE_1X;
	};

	struct transform16_camera_t : transform16_rotate_translate_t
	{
		uint16_t FocalDistance;
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

	// Applies rotation in intrinsic XYZ order: X, then Y, then Z.
	static void ApplyTransform(const transform16_rotate_t& transform, vertex16_t& vertex)
	{
		// Rotate around X-axis (affects Y,Z)
		const int16_t y1 = vertex.y;
		vertex.y = Fraction(transform.CosX, y1) - Fraction(transform.SinX, vertex.z);
		vertex.z =Fraction(transform.SinX, y1) +Fraction(transform.CosX, vertex.z);

		// Rotate around Y-axis (affects X,Z)
		const int16_t x1 = vertex.x;
		vertex.x = Fraction(transform.CosY, x1) + Fraction(transform.SinY, vertex.z);
		vertex.z = -Fraction(transform.SinY, x1) + Fraction(transform.CosY, vertex.z);

		// Rotate around Z-axis (affects X,Y)
		const int16_t x2 = vertex.x;
		vertex.x = Fraction(transform.CosZ, x2) - Fraction(transform.SinZ, vertex.y);
		vertex.y = Fraction(transform.SinZ, x2) + Fraction(transform.CosZ, vertex.y);
	}

	// Applies: Scale, then rotation in XYZ order, then Translation.
	static void ApplyTransform(const transform16_scale_rotate_translate_t& transform, vertex16_t& vertex)
	{
		// Scale geometry.
		vertex.x = Scale(transform.Resize, vertex.x);
		vertex.y = Scale(transform.Resize, vertex.y);
		vertex.z = Scale(transform.Resize, vertex.z);

		// Apply rotation.
		ApplyTransform(static_cast<const transform16_rotate_t&>(transform), vertex);

		// Apply translation.
		vertex.x += transform.Translation.x;
		vertex.y += transform.Translation.y;
		vertex.z += transform.Translation.z;
	}

	// Rotation only, same XYZ order (X, then Y, then Z).
	static void ApplyTransformRotation(const transform16_rotate_t& transform, vertex16_t& vertex)
	{
		ApplyTransform(transform, vertex);
	}

	// Rotation only, same XYZ order (X, then Y, then Z) using the rotation part of the scale/translate transform.
	static void ApplyTransformRotation(const transform16_scale_rotate_translate_t& transform, vertex16_t& vertex)
	{
		ApplyTransform(static_cast<const transform16_rotate_t&>(transform), vertex);
	}

	// Applies the inverse of the object rotation (intrinsic XYZ) to go world -> camera.
	// Order: Z^-1 (roll), then Y^-1 (yaw), then X^-1 (pitch).
	static void ApplyCameraTransformRotation(const transform16_rotate_t& transform, vertex16_t& vertex)
	{
		// Z^-1 (Roll)
		const int16_t x2 = vertex.x;
		vertex.x = Fraction(transform.CosZ, x2) + Fraction(transform.SinZ, vertex.y);
		vertex.y = -Fraction(transform.SinZ, x2) + Fraction(transform.CosZ, vertex.y);

		// Y^-1 (Yaw)
		const int16_t x1 = vertex.x;
		vertex.x = Fraction(transform.CosY, x1) - Fraction(transform.SinY, vertex.z);
		vertex.z = Fraction(transform.SinY, x1) + Fraction(transform.CosY, vertex.z);

		// X^-1 (Pitch)
		const int16_t y1 = vertex.y;
		vertex.y = Fraction(transform.CosX, y1) + Fraction(transform.SinX, vertex.z);
		vertex.z = -Fraction(transform.SinX, y1) + Fraction(transform.CosX, vertex.z);
	}

	// Camera transform (world -> camera):
	// 1) Subtract camera position
	// 2) Apply inverse rotation in order Z^-1, Y^-1, X^-1
	// 3) Shift z to compensate for projection focal distance.
	static void ApplyCameraTransform(const transform16_camera_t& transform, vertex16_t& vertex)
	{
		// Subtract camera world position
		vertex.x -= transform.Translation.x;
		vertex.y -= transform.Translation.y;
		vertex.z -= transform.Translation.z;

		// Inverse rotation (use reference cast to avoid temporary)
		ApplyCameraTransformRotation(static_cast<const transform16_rotate_t&>(transform), vertex);

		// Shift depth by focal distance (matches Project())
		vertex.z -= transform.FocalDistance;
	}
}
#endif