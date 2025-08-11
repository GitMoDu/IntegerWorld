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
	static constexpr uint32_t ShapeRotatePeriodMicros = 55555555;
	static constexpr uint32_t ShapeMovePeriodMicros = 11111111;

	static constexpr int16_t DistanceUnit = Assets::Shapes::SHAPE_UNIT;
	static constexpr int16_t BaseDistance = (VERTEX16_UNIT * 7) / 10;
	static constexpr int16_t ShapeMove = (DistanceUnit * 15) / 10;
	static constexpr int16_t ShapeMoveZ = ShapeMove / 3;

	int16_t ShapeMoveX = (((int32_t)ShapeMove * 4) / 10);

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
	Assets::Objects::TemplateFakeYShadeMeshObject<16, Assets::Objects::OctahedronMeshObject> ObjectOctahedron{};
#endif

	// Track animation color style.
	bool Monochrome = false;

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

	bool Start(IEngineRenderer& engineRenderer, const int16_t width, const int16_t height, const bool monochrome)
	{
		// Add all render objects to the pipeline.
		engineRenderer.ClearObjects();

		uint8_t fovPercent = 75;
		engineRenderer.SetFov((uint32_t(UFRACTION16_1X) * (100 - fovPercent)) / 100);

		if (!engineRenderer.AddObject(&ObjectCube)
			|| !engineRenderer.AddObject(&ObjectOctahedron))
		{
			TS::Task::disable();
			return false;
		}

		// Configure animation based on surface dimensions.
		ShapeMoveX = ((((int32_t)ShapeMove * 4) / 10) * width) / height;

		// Attach shaders to objects for rendering.
		ObjectCube.FragmentShader = &ObjectsShader;
		ObjectOctahedron.FragmentShader = &ObjectsShader;

		Monochrome = monochrome;
#if defined(DEMO_SCENE_EDGE_OBJECT)
		// Adjust animation on 1 bit color output.
		if (Monochrome)
		{
			ObjectCube.Color = Rgb8::COLOR_WHITE;
			ObjectOctahedron.Color = Rgb8::COLOR_WHITE;
		}

		// Set edge draw mode for each object.
		ObjectCube.EdgeDrawMode = EdgeDrawModeEnum::NoCulling;
		ObjectOctahedron.EdgeDrawMode = EdgeDrawModeEnum::NoCulling;
#endif	

		TS::Task::enable();

		return true;
	}

private:
	void AnimateObjects(const uint32_t timestamp)
	{
#if defined(DEMO_SCENE_EDGE_OBJECT)
		if (!Monochrome)
		{
			// Rainbow color pattern with HSV color.
			const ufraction16_t colorFraction = GetUFraction16((uint32_t)(timestamp % (ShapeColorPeriodMicros + 1)), ShapeColorPeriodMicros);
			ObjectOctahedron.Color = Rgb8::HsvColor(colorFraction, UINT8_MAX, UINT8_MAX);
		}
#endif

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

