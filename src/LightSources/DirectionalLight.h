#ifndef _INTEGER_WORLD_DIRECTIONAL_LIGHT_h
#define _INTEGER_WORLD_DIRECTIONAL_LIGHT_h

#include "AbstractLight.h"

namespace IntegerWorld
{
	class DirectionalLightSource : public ILightSource
	{
	public:
		color_fraction16_t Color{ UFRACTION16_1X, UFRACTION16_1X, UFRACTION16_1X };

	private:
		vertex16_t IlluminationVector{ 0, 0, -VERTEX16_UNIT };
		vertex32_t IlluminationNormal{ 0, 0, 0 };

	public:
		DirectionalLightSource() : ILightSource()
		{
		}

		void SetIlluminationVector(const vertex16_t& illuminationVector)
		{
			IlluminationVector.x = illuminationVector.x;
			IlluminationVector.y = illuminationVector.y;
			IlluminationVector.z = illuminationVector.z;
			NormalizeVertex16(IlluminationVector);
		}

		virtual void GetLightColor(color_fraction16_t& color)
		{
			color = Color;
		}

	public:
		void GetLighting(color_fraction16_t& lightColor, ufraction16_t& diffuse, ufraction16_t& specular, const world_position_shade_t& shade) final
		{
			lightColor = Color;
		}

		void GetLighting(color_fraction16_t& lightColor, ufraction16_t& diffuse, ufraction16_t& specular, const world_position_normal_shade_t& shade) final
		{
			lightColor = Color;
			// Get correlation between the illumination vector and surface normal.
			const int32_t lightDot = DotProduct16(IlluminationVector, shade.normalWorld);

			// Clamp negative values (i.e. when the illumination vector is on the wrong side) to 0.
			if (lightDot > 0)
			{
				// Scale the diffuse light output.
				diffuse = AbstractLightSource::GetDotFraction(lightDot);
			}

			// Get the specular component.
			specular = AbstractLightSource::GetSpecularFraction(IlluminationNormal, shade.positionWorld, shade.normalWorld);
		}
	};
}
#endif