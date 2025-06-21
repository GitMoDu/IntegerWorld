#ifndef _ANIMATED_TINY_SCENE_h
#define _ANIMATED_TINY_SCENE_h

#include "TinyAssets.h"
#include <IntegerWorld.h>

using namespace IntegerWorld;

class AnimatedTinyScene : private TS::Task
{
public:
	static constexpr uint8_t RenderObjectCount = 2;

private:
	static constexpr uint32_t ShapeRotatePeriodMicros = 65000000;
	static constexpr uint32_t ShapeMovePeriodMicros = 35111111;

	static constexpr int16_t BaseDistance = -((int32_t)VERTEX16_UNIT * 5) / 10;
	static constexpr int16_t ShapeMove = ((int32_t)Assets::Shapes::SHAPE_UNIT * 30) / 10;
	static constexpr int16_t ShapeMoveX = ((((int32_t)ShapeMove * 4) / 10) * 128) / 64;
	static constexpr int16_t ShapeMoveZ = ShapeMove / 3;

private:
	EdgeFragmentShader ObjectsShader{};

	Assets::Objects::CubeEdgeObject ObjectCube{};
	Assets::Objects::OctahedronEdgeObject ObjectOctahedron{};

public:
	AnimatedTinyScene(TS::Scheduler& scheduler)
		: TS::Task(10, TASK_FOREVER, &scheduler, false)
	{		
	}

	bool Callback() final
	{
		AnimateObjects(micros());

		return true;
	}

	void Start(IEngineRenderer* EngineRenderer)
	{
		// Attach shaders to objects for rendering.
		ObjectCube.FragmentShader = &ObjectsShader;
		ObjectOctahedron.FragmentShader = &ObjectsShader;

		// Set edge draw mode for each object.
		ObjectCube.EdgeDrawMode = EdgeDrawModeEnum::CullAllBehind;
		ObjectOctahedron.EdgeDrawMode = EdgeDrawModeEnum::NoCulling;

		// Add all render objects to the pipeline.
		EngineRenderer->ClearObjects();
		EngineRenderer->AddObject(&ObjectCube);
		EngineRenderer->AddObject(&ObjectOctahedron);

		TS::Task::enable();
	}

private:
	void AnimateObjects(const uint32_t timestamp)
	{
		// Continuous rotation on all 3 axis.
		const ufraction16_t xRotateFraction = GetUFraction16((uint32_t)(timestamp % (ShapeRotatePeriodMicros + 1)), ShapeRotatePeriodMicros);
		const ufraction16_t yRotateFraction = GetUFraction16((uint32_t)(timestamp % (((ShapeRotatePeriodMicros * 1) / 3) + 1)), (ShapeRotatePeriodMicros * 1) / 3);
		const ufraction16_t zRotateFraction = GetUFraction16((uint32_t)(timestamp % (((ShapeRotatePeriodMicros * 2) / 3) + 1)), (ShapeRotatePeriodMicros * 2) / 3);

		ObjectCube.Rotation.x = Fraction::Scale(xRotateFraction, (uint16_t)ANGLE_RANGE);
		ObjectCube.Rotation.y = Fraction::Scale(yRotateFraction, (uint16_t)ANGLE_RANGE);
		ObjectCube.Rotation.z = Fraction::Scale(zRotateFraction, (uint16_t)ANGLE_RANGE);

		ObjectOctahedron.Rotation.x = ObjectCube.Rotation.x + GetAngle(180);
		ObjectOctahedron.Rotation.y = ObjectCube.Rotation.y + GetAngle(180);
		ObjectOctahedron.Rotation.z = ObjectCube.Rotation.z + GetAngle(180);

		// Circle animation.
		const ufraction16_t circleFraction = GetUFraction16((uint32_t)(timestamp % (ShapeMovePeriodMicros + 1)), ShapeMovePeriodMicros);
		const angle_t circleAngle = Fraction::Scale(circleFraction, ANGLE_RANGE);

		const fraction16_t xMoveFraction1 = Sine16(circleAngle);
		const fraction16_t xMoveFraction2 = Sine16((int32_t)circleAngle + GetAngle(180));

		const fraction16_t zMoveFraction1 = Cosine16((int32_t)circleAngle);
		const fraction16_t zMoveFraction2 = Cosine16((int32_t)circleAngle + GetAngle(180));

		ObjectCube.Translation.x = Scale(xMoveFraction1, ShapeMoveX);
		ObjectCube.Translation.z = BaseDistance + Scale(zMoveFraction1, ShapeMoveZ);

		ObjectOctahedron.Translation.x = Scale(xMoveFraction2, ShapeMoveX);
		ObjectOctahedron.Translation.z = BaseDistance + Scale(zMoveFraction2, ShapeMoveZ);
	}
};

#endif

