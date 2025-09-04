#ifndef _INTEGER_WORLD_POINT_LIGHT_h
#define _INTEGER_WORLD_POINT_LIGHT_h

#include "AbstractLight.h"

namespace IntegerWorld
{
	class PointLightSource : public AbstractProximityLight
	{
	private:
		vertex32_t IlluminationVector{ 0, 0, 0 };
		vertex16_t CameraVector{ 0, 0, 0 };

	public:
		Rgb8::color_t Color = Rgb8::WHITE;

	public:
		PointLightSource(const vertex16_t* cameraPosition = nullptr)
			: AbstractProximityLight(cameraPosition)
		{
		}

		void SetCameraPosition(const vertex16_t* cameraPosition)
		{
			CameraPosition = cameraPosition;
		}

	public:
		void GetLightColor(Rgb8::color_t& color) final
		{
			color = Color;
		}

		void GetLighting(Rgb8::color_t& lightColor, ufraction16_t& diffuse, ufraction16_t& specular, const world_position_shade_t& shade) final
		{
			diffuse = 0;
			lightColor = Color;

			if (Color != 0)
			{
				// Calculate illumination vector from this point to the light source world position.
				IlluminationVector = { WorldPosition.x - shade.positionWorld.x,
												WorldPosition.y - shade.positionWorld.y,
												WorldPosition.z - shade.positionWorld.z };

				diffuse = GetProximityFraction(IlluminationVector);
			}
		}

		void GetLighting(Rgb8::color_t& lightColor, ufraction16_t& diffuse, ufraction16_t& specular, const world_position_normal_shade_t& shade) final
		{
			diffuse = 0;
			specular = 0;
			lightColor = Color;

			if (Color != 0)
			{
				// Calculate illumination vector from this point to the light source world position.
				IlluminationVector = { WorldPosition.x - shade.positionWorld.x,
												WorldPosition.y - shade.positionWorld.y,
												WorldPosition.z - shade.positionWorld.z };

				// Get the distance weight before normalizing the vector.
				const ufraction16_t proximityFraction = GetProximityFraction(IlluminationVector);

				if (proximityFraction > 0)
				{
					// Normalize vector for dot magnitude calculation.
					NormalizeVertex32Fast(IlluminationVector);

					// Get correlation between the illumination vector and surface normal.
					const int32_t lightDot = DotProduct16(IlluminationVector, shade.normalWorld);

					if (lightDot > 0)
					{
						diffuse = ufraction16_t(Fraction::Scale(proximityFraction, uint16_t(uint32_t(lightDot) >> AbstractLightSource::DOT_CONVERT_SHIFTS)));
					}

					if (CameraPosition == nullptr)
					{
						// No specular component without a valid camera position.
						return;
					}

					// Calculate camera vector from this point to the camera world position.
					CameraVector = { int16_t(CameraPosition->x - shade.positionWorld.x)  ,
												 int16_t(CameraPosition->y - shade.positionWorld.y),
												  int16_t(CameraPosition->z - shade.positionWorld.z) };

					// Normalize vector for dot magnitude calculation.
					NormalizeVertex16(CameraVector);

					// Blinn–Phong: Compute the half–vector as the sum of the illumination vector and the view vector.
					const vertex32_t halfVector{ SignedRightShift((int32_t)IlluminationVector.x + CameraVector.x, 1),
											SignedRightShift((int32_t)IlluminationVector.y + CameraVector.y, 1),
											SignedRightShift((int32_t)IlluminationVector.z + CameraVector.z, 1) };

					// Compute the dot product between the surface normal and the half–vector.
					// In the Blinn–Phong model, a higher dot product corresponds to a stronger specular highlight.
					const int32_t dotProduct = DotProduct16(shade.normalWorld, halfVector);

					// Get the specular component.
					if (dotProduct > 0)
					{
						// Use a squared falloff for the specular highlight.
						specular = ufraction16_t(uint32_t(dotProduct) >> (AbstractLightSource::DOT_CONVERT_SHIFTS - 1));
						specular = (uint32_t(specular) * specular) >> (GetBitShifts(UFRACTION16_1X) + 2);

						// Scale by proximity to light source.
						specular = Fraction::Scale(proximityFraction, specular);
					}
				}
			}
		}
	};
}
#endif