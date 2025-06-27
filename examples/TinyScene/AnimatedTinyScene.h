#ifndef _ANIMATED_TINY_SCENE_h
#define _ANIMATED_TINY_SCENE_h

//#define DEMO_SCENE_EDGE_OBJECT // Enable for monochrome and low RAM devices.

#include "TinyAssets.h"
#include <IntegerWorld.h>

using namespace IntegerWorld;

class AnimatedTinyScene : private TS::Task
{
public:
	static constexpr uint16_t RenderObjectCount = 2;
#if defined(DEMO_SCENE_EDGE_OBJECT)
	static constexpr uint16_t MaxDrawCallCount = Assets::Shapes::Cube::TriangleCount + Assets::Shapes::Octahedron::EdgeCount;
#else
	static constexpr uint16_t MaxDrawCallCount = Assets::Shapes::Cube::TriangleCount + Assets::Shapes::Octahedron::TriangleCount;
#endif
private:
	static constexpr uint32_t ShapeColorPeriodMicros = 19000000;
	static constexpr uint32_t ShapeRotatePeriodMicros = 65000000;
	static constexpr uint32_t ShapeMovePeriodMicros = 15111111;

	static constexpr int16_t BaseDistance = -((int32_t)VERTEX16_UNIT * 5) / 10;
	static constexpr int16_t ShapeMove = ((int32_t)Assets::Shapes::SHAPE_UNIT * 30) / 10;
	static constexpr int16_t ShapeMoveX = ((((int32_t)ShapeMove * 4) / 10) * 128) / 64;
	static constexpr int16_t ShapeMoveZ = ShapeMove / 3;

private:
#if defined(DEMO_SCENE_EDGE_OBJECT)
	// Shared shader for objects.
	EdgeFragmentShader ObjectsShader{};

	// Edge line objects.
	Assets::Objects::CubeEdgeObject ObjectCube{};
	Assets::Objects::OctahedronEdgeObject ObjectOctahedron{};
#else
	// Shared shader for objects.
	TriangleFillFragmentShader ObjectsShader{};

	// Mesh triangle objects.
	Assets::Objects::CubeMeshObject ObjectCube{};
	Assets::Objects::OctahedronMeshObject ObjectOctahedron{};
#endif

	// Track animation color style.
	uint8_t ColorDepth = 1;

public:
	AnimatedTinyScene(TS::Scheduler& scheduler)
		: TS::Task(20, TASK_FOREVER, &scheduler, false)
	{
	}

	bool Callback() final
	{
		AnimateObjects(micros());

		return true;
	}

	void Start(IEngineRenderer& engineRenderer, const uint8_t colorDepth)
	{
		// Attach shaders to objects for rendering.
		ObjectCube.FragmentShader = &ObjectsShader;
		ObjectOctahedron.FragmentShader = &ObjectsShader;

#if defined(DEMO_SCENE_EDGE_OBJECT)
		// Adjust animation on 1 bit color output.
		ColorDepth = colorDepth;
		if (ColorDepth <= 1)
		{
			ObjectCube.Color = ColorFraction::COLOR_WHITE;
			ObjectOctahedron.Color = ColorFraction::COLOR_WHITE;
		}

		// Set edge draw mode for each object.
		ObjectCube.EdgeDrawMode = EdgeDrawModeEnum::NoCulling;
		ObjectOctahedron.EdgeDrawMode = EdgeDrawModeEnum::NoCulling;
#else
		// Adjust animation on 1 bit color output.
		ColorDepth = colorDepth;
		if (ColorDepth <= 1)
		{
			ObjectOctahedron.Color = ColorFraction::COLOR_WHITE;
		}
#endif	

		// Add all render objects to the pipeline.
		engineRenderer.ClearObjects();
		engineRenderer.AddObject(&ObjectCube);
		engineRenderer.AddObject(&ObjectOctahedron);

		TS::Task::enable();
	}

private:
	void AnimateObjects(const uint32_t timestamp)
	{
		if (ColorDepth > 1)
		{
			// Rainbow color pattern with HSV color.
			const ufraction16_t colorFraction = GetUFraction16((uint32_t)(timestamp % (ShapeColorPeriodMicros + 1)), ShapeColorPeriodMicros);
#if defined(DEMO_SCENE_EDGE_OBJECT)
			ObjectCube.Color = ColorFraction::HsvToColorFraction(colorFraction, UFRACTION16_1X, UFRACTION16_1X);
#endif
			ObjectOctahedron.Color = ColorFraction::HsvToColorFraction(UFRACTION16_1X - colorFraction, UFRACTION16_1X, UFRACTION16_1X);
		}

		// Continuous rotation on all 3 axis.
		const ufraction16_t xRotateFraction = GetUFraction16((uint32_t)(timestamp % (ShapeRotatePeriodMicros + 1)), ShapeRotatePeriodMicros);
		const ufraction16_t yRotateFraction = GetUFraction16((uint32_t)(timestamp % (((ShapeRotatePeriodMicros * 1) / 3) + 1)), (ShapeRotatePeriodMicros * 1) / 3);
		const ufraction16_t zRotateFraction = GetUFraction16((uint32_t)(timestamp % (((ShapeRotatePeriodMicros * 2) / 3) + 1)), (ShapeRotatePeriodMicros * 2) / 3);

		ObjectCube.Rotation.x = Fraction::Scale(xRotateFraction, (uint16_t)ANGLE_RANGE);
		ObjectCube.Rotation.y = Fraction::Scale(yRotateFraction, (uint16_t)ANGLE_RANGE);
		ObjectCube.Rotation.z = Fraction::Scale(zRotateFraction, (uint16_t)ANGLE_RANGE);

		ObjectOctahedron.Rotation.x = ObjectCube.Rotation.x + Trigonometry::ANGLE_180;
		ObjectOctahedron.Rotation.y = ObjectCube.Rotation.y + Trigonometry::ANGLE_180;
		ObjectOctahedron.Rotation.z = ObjectCube.Rotation.z + Trigonometry::ANGLE_180;

		// Circle animation.
		const ufraction16_t circleFraction = GetUFraction16((uint32_t)(timestamp % (ShapeMovePeriodMicros + 1)), ShapeMovePeriodMicros);
		const angle_t circleAngle = Fraction::Scale(circleFraction, ANGLE_RANGE);

		const fraction16_t xMoveFraction1 = Sine16(circleAngle);
		const fraction16_t xMoveFraction2 = Sine16((int32_t)circleAngle + Trigonometry::ANGLE_180);

		const fraction16_t zMoveFraction1 = Cosine16((int32_t)circleAngle);
		const fraction16_t zMoveFraction2 = Cosine16((int32_t)circleAngle + Trigonometry::ANGLE_180);

		ObjectCube.Translation.x = Scale(xMoveFraction1, ShapeMoveX);
		ObjectCube.Translation.z = BaseDistance + Scale(zMoveFraction1, ShapeMoveZ);

		ObjectOctahedron.Translation.x = Scale(xMoveFraction2, ShapeMoveX);
		ObjectOctahedron.Translation.z = BaseDistance + Scale(zMoveFraction2, ShapeMoveZ);
	}
};

#endif

