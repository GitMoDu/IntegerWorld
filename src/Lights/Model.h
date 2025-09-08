#ifndef _INTEGER_WORLD_LIGHTS_MODEL_h
#define _INTEGER_WORLD_LIGHTS_MODEL_h

#include "../Scene/AbstractSceneShader.h"

namespace IntegerWorld
{
	/// <summary>
	/// Defines the types of light sources available in the rendering system.
	/// </summary>
	enum class LightTypeEnum
	{
		/// <summary>Light with parallel rays (like sunlight) that affects all objects equally regardless of distance.</summary>
		Directional,

		/// <summary>Light that emits in all directions from a single point with distance-based attenuation.</summary>
		Point,

		/// <summary>Light that emits in a cone from a point with both direction and distance-based attenuation.</summary>
		Spot,

		/// <summary>Light attached to the camera position that moves with the viewer.</summary>
		Camera,

		/// <summary>Disabled light source.</summary>
		None
	};

	/// <summary>
	/// Represents a complete light source definition with position, direction, color, and type-specific parameters.
	/// </summary>
	struct light_source_t
	{
		/// <summary>
		/// The position of the light source in world space.
		/// Has no effect on directional lights.
		/// </summary>
		vertex16_t Position;

		/// <summary>
		/// The orientation vector of the light source.
		/// For directional lights, this is the direction the light is traveling.
		/// For spot lights, this is the center axis of the light cone.
		/// Has no effect on Point or Camera lights.
		/// </summary>
		vertex16_t Direction;

		/// <summary>
		/// The squared minimum distance for attenuation calculations.
		/// Within this distance, the light maintains full intensity.
		/// Has no effect on Directional lights.
		/// </summary>
		uint32_t RangeSquaredMin;

		/// <summary>
		/// The squared maximum distance for attenuation calculations.
		/// Beyond this distance, the light has no effect.
		/// Has no effect on Directional lights.
		/// </summary>
		uint32_t RangeSquaredMax;

		/// <summary>
		/// The RGB color of the light source that determines the illumination color.
		/// </summary>
		Rgb8::color_t Color;

		/// <summary>
		/// Multi-purpose parameter with different meanings based on light type:
		/// - For Spot lights: Controls the radius/angle of the light cone
		/// - For Directional lights: Controls diffuse factor when normals are not available
		/// </summary>
		ufraction16_t Parameter;

		/// <summary>
		/// The type of light source which determines its behavior in the scene.
		/// </summary>
		LightTypeEnum Type;

		/// <summary>
		/// Sets the minimum and maximum range for light attenuation.
		/// </summary>
		/// <param name="rangeMin">The minimum distance where attenuation begins</param>
		/// <param name="rangeMax">The maximum distance where light has no effect</param>
		void SetRange(const uint16_t rangeMin, const uint16_t rangeMax)
		{
			RangeSquaredMin = (uint32_t)rangeMin * rangeMin;
			RangeSquaredMax = MaxValue<uint32_t>((uint32_t)rangeMax * rangeMax, RangeSquaredMin);
		}

		/// <summary>
		/// Sets and normalizes the direction vector for the light source.
		/// </summary>
		/// <param name="illuminationVector">The vector indicating the light direction (will be normalized)</param>
		void SetDirectionVector(const vertex16_t& illuminationVector)
		{
			Direction = illuminationVector;
			NormalizeVertex16(Direction);
		}
	};

	/// <summary>
	/// Creates a point light source with the specified parameters.
	/// Point light emits light equally in all directions from a single position with distance-based attenuation.
	/// </summary>
	/// <param name="color">The RGB color of the light source</param>
	/// <param name="position">The position in world space from which light will emit</param>
	/// <param name="rangeMin">The minimum distance before attenuation begins (full intensity within this range)</param>
	/// <param name="rangeMax">The maximum distance where light has effect (no illumination beyond this range)</param>
	/// <returns>A fully configured point light source</returns>
	static constexpr light_source_t PointLightSource(
		const Rgb8::color_t color = Rgb8::WHITE,
		const vertex16_t position = { int16_t(0), int16_t(0), int16_t(0) },
		const uint16_t rangeMin = 0,
		const uint16_t rangeMax = VERTEX16_RANGE)
	{
		return light_source_t{
			position,
			{ int16_t(0), int16_t(0), int16_t(0)}, // direction is unused for point lights
			((uint32_t)rangeMin * rangeMin),
			MaxValue<uint32_t>((uint32_t)rangeMax * rangeMax, ((uint32_t)rangeMin * rangeMin)),
			color,
			0, // parameter not used for point lights
			LightTypeEnum::Point };
	}

	/// <summary>
	/// Creates a directional light source with the specified parameters.
	/// Directional light has parallel rays and no distance attenuation, affecting all objects equally.
	/// </summary>
	/// <param name="color">The RGB color of the light source</param>
	/// <param name="direction">The direction vector for light rays (will not be normalized)</param>
	/// <param name="parameter">Controls diffuse factor when normals are not available</param>
	/// <returns>A fully configured directional light source</returns>
	static constexpr light_source_t DirectionalLightSource(
		const Rgb8::color_t color = Rgb8::WHITE,
		const vertex16_t direction = { 0, VERTEX16_UNIT, 0 },
		const ufraction16_t parameter = ufraction16_t(0))
	{
		return light_source_t{
			{ int16_t(0), int16_t(0), int16_t(0) }, // position not used for directional lights
			direction,
			0,     // range not used
			0,     // range not used
			color,
			parameter,
			LightTypeEnum::Directional };
	}

	/// <summary>
	/// Creates a spot light source with the specified parameters.
	/// Spot light emits in a cone from a point with both direction and distance-based attenuation.
	/// </summary>
	/// <param name="color">The RGB color of the light source</param>
	/// <param name="position">The position in world space</param>
	/// <param name="direction">The direction vector for the center of light cone (will not be normalized)</param>
	/// <param name="rangeMin">The minimum distance before attenuation begins</param>
	/// <param name="rangeMax">The maximum distance where light has effect</param>
	/// <param name="parameter">Controls the radius/angle of the light cone</param>
	/// <returns>A fully configured spot light source</returns>
	static constexpr light_source_t SpotLightSource(
		const Rgb8::color_t color = Rgb8::WHITE,
		const vertex16_t position = { int16_t(0), int16_t(0), int16_t(0) },
		const vertex16_t direction = { int16_t(0), VERTEX16_UNIT, int16_t(0) },
		const uint16_t rangeMin = 0,
		const uint16_t rangeMax = VERTEX16_RANGE,
		const ufraction16_t parameter = UFRACTION16_1X)
	{
		return light_source_t{
			position,
			direction,
			((uint32_t)rangeMin * rangeMin),
			MaxValue<uint32_t>((uint32_t)rangeMax * rangeMax, ((uint32_t)rangeMin * rangeMin)),
			color,
			parameter,
			LightTypeEnum::Spot };
	}

	/// <summary>
	/// Creates a camera light source with the specified parameters.
	/// Camera light moves with the camera and illuminates in the viewing direction.
	/// </summary>
	/// <param name="color">The RGB color of the light source</param>
	/// <param name="rangeMin">The minimum distance before attenuation begins</param>
	/// <param name="rangeMax">The maximum distance where light has effect</param>
	/// <returns>A fully configured camera light source</returns>
	static constexpr light_source_t CameraLightSource(
		const Rgb8::color_t color = Rgb8::WHITE,
		const uint16_t rangeMin = 0,
		const uint16_t rangeMax = VERTEX16_RANGE)
	{
		return light_source_t{
			{ int16_t(0), int16_t(0), int16_t(0) }, // position is determined by camera
			{ int16_t(0), int16_t(0), int16_t(0) }, // direction is determined by camera
			((uint32_t)rangeMin * rangeMin),
			MaxValue<uint32_t>((uint32_t)rangeMax * rangeMax, ((uint32_t)rangeMin * rangeMin)),
			color,
			0, // parameter not used for camera lights
			LightTypeEnum::Camera };
	}
}
#endif