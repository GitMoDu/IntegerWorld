#ifndef _INTEGER_WORLD_ABSTRACT_OBJECT_h
#define _INTEGER_WORLD_ABSTRACT_OBJECT_h

#include "../Framework/Interface.h"
#include "Shader.h"

namespace IntegerWorld
{
	/// <summary>
	/// Abstract Render Object.
	/// </summary>
	class AbstractObject : public IRenderObject
	{
	public:
		AbstractObject() : IRenderObject() {}

		// Default implementation since most objects do not have per-vertex animation.
		virtual bool VertexShade(const uint16_t vertexIndex)
		{
			return true;
		}
	};
}
#endif