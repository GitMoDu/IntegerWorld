#ifndef _INTEGER_WORLD_ABSTRACT_OBJECT_h
#define _INTEGER_WORLD_ABSTRACT_OBJECT_h

#include "../Framework/Model.h"
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

	public:

	};

	static bool IsVertexIn2DBounds(const uint16_t boundsWidth, const uint16_t boundsHeight, const vertex16_t vertex)
	{
		return vertex.x >= 0
			&& vertex.x < boundsWidth
			&& vertex.x >= 0
			&& vertex.y < boundsHeight;
	}
}
#endif