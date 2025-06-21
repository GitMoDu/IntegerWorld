#ifndef _INTEGER_WORLD_POINT_LIGHT_h
#define _INTEGER_WORLD_POINT_LIGHT_h

#include "AbstractLight.h"

namespace IntegerWorld
{
	class PointLightSource : public AbstractProximityLight
	{
	public:
		color_fraction16_t Color{ UFRACTION16_1X, UFRACTION16_1X, UFRACTION16_1X };

	private:
		uint32_t RangeMin = (uint32_t)VERTEX16_UNIT * VERTEX16_UNIT;
		uint32_t RangeMax = (uint32_t)VERTEX16_RANGE * VERTEX16_RANGE;

	public:
		PointLightSource() : AbstractProximityLight()
		{
		}

	public:
		void GetLighting(color_fraction16_t& lightColor, ufraction16_t& diffuse, ufraction16_t& specular, const world_position_shade_t& shade) final
		{
			lightColor = Color;

			// Calculate illumination vector from this point to the light source world position.
			const vertex32_t illuminationVector{ WorldPosition.x - shade.positionWorld.x,
											WorldPosition.y - shade.positionWorld.y,
											WorldPosition.z - shade.positionWorld.z };

			diffuse = GetProximityFraction(illuminationVector);
		}

		void GetLighting(color_fraction16_t& lightColor, ufraction16_t& diffuse, ufraction16_t& specular, const world_position_normal_shade_t& shade) final
		{
			lightColor = Color;

			// Calculate illumination vector from this point to the light source world position.
			vertex32_t illuminationVector{ WorldPosition.x - shade.positionWorld.x,
											WorldPosition.y - shade.positionWorld.y,
											WorldPosition.z - shade.positionWorld.z };

			// Get the distance weight before normalizing the vector.
			const ufraction16_t proximityFraction = GetProximityFraction(illuminationVector);

			if (proximityFraction > 0)
			{
				// Normalize vector for dot magnitude calculation.
				NormalizeVertex32Fast(illuminationVector);

				// Get correlation between the illumination vector and surface normal.
				const int32_t lightDot = DotProduct16(illuminationVector, shade.normalWorld);

				if (lightDot > 0)
				{
					// Scale the diffuse light output with proximity of surface to light.
					diffuse = Fraction::Scale(proximityFraction, AbstractLightSource::GetDotProductFraction(lightDot));
				}

				// Get the specular component.
				specular = Fraction::Scale(proximityFraction, AbstractLightSource::GetSpecularFraction(illuminationVector, ObjectPosition, shade.normalWorld));
			}
		}
	};
}
#endif