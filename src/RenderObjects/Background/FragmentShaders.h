#ifndef _INTEGER_WORLD_OBJECT_RENDER_OBJECTS_BACKGROUND_FRAGMENT_SHADERS_h
#define _INTEGER_WORLD_OBJECT_RENDER_OBJECTS_BACKGROUND_FRAGMENT_SHADERS_h

#include "../AbstractObject.h"

namespace IntegerWorld
{
	namespace RenderObjects
	{
		namespace Background
		{
			namespace FragmentShaders
			{
				struct FillShader final : public IFragmentShader<color_fragment_t>
				{
					void FragmentShade(WindowRasterizer& rasterizer, const color_fragment_t& fragment)
					{
						rasterizer.Fill(Rgb8::Color(fragment.red, fragment.green, fragment.blue));
					}
				};
			}
		}
	}
}
#endif