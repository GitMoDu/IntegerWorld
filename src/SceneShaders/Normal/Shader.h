#ifndef _INTEGER_WORLD_SCENE_SHADERS_NORMAL_SHADER_h
#define _INTEGER_WORLD_SCENE_SHADERS_NORMAL_SHADER_h

#include "../../Framework/Interface.h"

namespace IntegerWorld
{
	namespace SceneShaders
	{
		namespace Normal
		{
			/// <summary>
			/// Shades the surface based on its normal vector, 
			/// standard normal visualization: R=X, G=Y, B=Z.
			/// </summary>
			class Shader : public ISceneShader
			{
			public:
				Shader() : ISceneShader() {}

				virtual Rgb8::color_t GetLitColor(const Rgb8::color_t /*albedo*/, const material_t& /*material*/, const vertex16_t& /*position*/, const vertex16_t& normal)
				{
					return Rgb8::Color(
						U8Normal(normal.x),
						U8Normal(normal.y),
						U8Normal(normal.z));
				}

			private:
				/// <summary>
				/// Biases, clamps, and scales a signed 16-bit value to an 8-bit unsigned normalized value in the range [0, 255].
				/// </summary>
				/// <param name="n">Input 16-bit signed value (typically a vertex/component value). The function biases the value by VERTEX16_UNIT, clamps the result to [0, 2*VERTEX16_UNIT], and then scales with rounding into the 0..255 range.</param>
				/// <returns>An 8-bit unsigned value (0..255) representing the biased, clamped, and rounded result of the input.</returns>
				inline uint8_t U8Normal(const int16_t n)
				{
					// Bias to [0 .. 2*UNIT] and clamp.
					int32_t s = int32_t(n) + int32_t(VERTEX16_UNIT);
					if (s < 0) s = 0;
					const int32_t S_MAX = uint32_t(VERTEX16_UNIT) << 1;
					if (s > S_MAX) s = S_MAX;

					// Scale to [0..255] exactly: (s * 255) >> (log2(2*UNIT)).
					// This reaches 255 at +UNIT (no half-brightness).
					constexpr uint8_t UNIT_BITS = GetBitShifts(VERTEX16_UNIT);
					const uint32_t rounded = (uint32_t(s) * UINT8_MAX + (1u << UNIT_BITS)) >> (UNIT_BITS + 1);
					return static_cast<uint8_t>(rounded);
				}
			};
		}
	}
}
#endif