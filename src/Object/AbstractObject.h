#ifndef _INTEGER_WORLD_ABSTRACT_OBJECT_h
#define _INTEGER_WORLD_ABSTRACT_OBJECT_h

#include "../Framework/Interface.h"
#include "Shader.h"

namespace IntegerWorld
{
	/// <summary>
	/// Abstract Render Object.
	/// Host this object's SceneShader SurfaceShader (material+lights).
	/// </summary>
	class AbstractObject : public IRenderObject
	{
	public:
		ISceneShader* SceneShader = nullptr;

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