#ifndef _INTEGER_WORLD_INCLUDE_h
#define _INTEGER_WORLD_INCLUDE_h

// Core framework interfaces.
#include "Framework/Interface.h"
#include "Framework/Model.h"

// Primitives for shaders.
#include "Shaders/Primitive/TriangleSampler.h"
#include "Shaders/Primitive/LineSampler.h"
#include "Shaders/Primitive/DepthSampler.h"

// Template building blocks for render objects.
#include "PrimitiveSources/Vertex.h"
#include "PrimitiveSources/Triangle.h"
#include "PrimitiveSources/Albedo.h"
#include "PrimitiveSources/Material.h"
#include "PrimitiveSources/Normal.h"
#include "PrimitiveSources/Uv.h"
#include "PrimitiveSources/Texture.h"
#include "PrimitiveSources/Edge.h"

// Primitives for fragment and scene shaders.
#include "Shaders/Primitive/TriangleSampler.h"
#include "Shaders/Primitive/LineSampler.h"
#include "Shaders/Primitive/DepthSampler.h"

// Abstract classes for render objects.
#include "RenderObjects/AbstractObject.h"

// Background render objects.
#include "RenderObjects/Background/Object.h"
#include "RenderObjects/Background/FragmentShaders.h"

// Point Cloud render objects.
#include "RenderObjects/PointCloud/Object.h"
#include "RenderObjects/PointCloud/FragmentShaders.h"

// Billboard render objects.
#include "RenderObjects/Billboard/Object.h"
#include "RenderObjects/Billboard/FragmentShaders.h"

// Edge render objects.
#include "RenderObjects/Edge/AbstractObject.h"
#include "RenderObjects/Edge/LineShadeObject.h"
#include "RenderObjects/Edge/VertexShadeObject.h"
#include "RenderObjects/Edge/PixelShaders.h"
#include "RenderObjects/Edge/FragmentShaders.h"

// Mesh render objects.
#include "RenderObjects/Mesh/AbstractObject.h"
#include "RenderObjects/Mesh/TriangleShadeObject.h"
#include "RenderObjects/Mesh/VertexShadeObject.h"
#include "RenderObjects/Mesh/PixelShaders.h"
#include "RenderObjects/Mesh/FragmentShaders.h"

// Scene shaders, including lights.
#include "SceneShaders/Abstract.h"
#include "SceneShaders/LightSource/Model.h"
#include "SceneShaders/LightSource/Shader.h"
#include "SceneShaders/Normal/Shader.h"

// Render tasks.
#include "Engine/EngineRenderTask.h"
#include "Engine/PerformanceLogTask.h"

// Sample shape assets.
#include "Assets/Shapes.h"

// Sample cameras.
#include "Assets/Cameras.h"

#endif