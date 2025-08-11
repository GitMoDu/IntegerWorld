#ifndef _INTEGER_WORLD_DIRECTIONAL_LIGHT_h
#define _INTEGER_WORLD_DIRECTIONAL_LIGHT_h

#include "AbstractLight.h"

namespace IntegerWorld
{
	class DirectionalLightSource : public ILightSource
	{
	public:
		Rgb8::color_t Color = Rgb8::WHITE;

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

		void GetLightColor(Rgb8::color_t& color) final
		{
			color = Color;
		}

	public:
		void GetLighting(Rgb8::color_t& lightColor, ufraction16_t& diffuse, ufraction16_t& specular, const world_position_shade_t& shade) final
		{
			lightColor = Color;
			diffuse = 0;
			specular = 0;
		}

		void GetLighting(Rgb8::color_t& lightColor, ufraction16_t& diffuse, ufraction16_t& specular, const world_position_normal_shade_t& shade) final
		{
			diffuse = 0;
			specular = 0;
			lightColor = Color;

			if (Color != 0)
			{
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
		}
	};
}
#endif