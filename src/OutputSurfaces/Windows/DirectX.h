#ifndef _INTEGER_WORLD_DIRECTX_h
#define _INTEGER_WORLD_DIRECTX_h

#include <stdint.h>

namespace IntegerWorld
{
	namespace DirectX
	{
		// HLSL source for vertex and pixel shaders
		static const char* vsSource = R"(
			struct VS_INPUT {
				float3 pos : POSITION;
				float2 tex : TEXCOORD0;
			};
			struct PS_INPUT {
				float4 pos : SV_POSITION;
				float2 tex : TEXCOORD0;
			};
			PS_INPUT main(VS_INPUT input) {
				PS_INPUT output;
				output.pos = float4(input.pos, 1.0);
				output.tex = input.tex;
				return output;
			}
			)";

		static const char* psSource = R"(
			Texture2D tex : register(t0);
			SamplerState samLinear : register(s0);
			float4 main(float4 pos : SV_POSITION, float2 texCoord : TEXCOORD0) : SV_TARGET {
				return tex.Sample(samLinear, texCoord);
			}
			)";
	}
}

#endif