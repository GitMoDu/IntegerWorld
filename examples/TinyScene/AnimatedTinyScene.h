#ifndef _ANIMATED_TINY_SCENE_h
#define _ANIMATED_TINY_SCENE_h


//#define DEMO_SCENE_EDGE_OBJECT // Enable for monochrome and low RAM devices.

#include "TinyAssets.h"
#include <IntegerWorld.h>

using namespace IntegerWorld;

class AnimatedTinyScene : private TS::Task
{
public:
	static constexpr uint8_t RenderObjectCount = 2;

#if defined(DEMO_SCENE_EDGE_OBJECT)
	static constexpr uint16_t MaxDrawCallCount = Assets::Shapes::Cube::TriangleCount + Assets::Shapes::Octahedron::EdgeCount;
#else
	static constexpr uint16_t MaxDrawCallCount = Assets::Shapes::Cube::TriangleCount + Assets::Shapes::Octahedron::TriangleCount;
#endif
private:
	static constexpr uint32_t ShapeColorPeriodMicros = 19000000;
	static constexpr uint32_t ShapeRotatePeriodMicros = 35000000;
	static constexpr uint32_t ShapeMovePeriodMicros = 15111111;

	static constexpr int16_t DistanceUnit = Assets::Shapes::SHAPE_UNIT;
	static constexpr int16_t BaseDistance = (int32_t(VERTEX16_UNIT) * 9) / 10;
	static constexpr int16_t ShapeMove = (int32_t(DistanceUnit) * 20) / 10;
	static constexpr int16_t ShapeMoveZ = ShapeMove / 2;

	int16_t ShapeMoveX = (((int32_t)ShapeMove * 4) / 10);

private:
#if defined(DEMO_SCENE_EDGE_OBJECT)
	// Shared shader for objects.
	RenderObjects::Edge::FragmentShaders::LineShade::FillShader ObjectsShader{};

	// Edge line objects.
	Assets::RenderObjects::CubeEdgeObject ObjectCube{};
	Assets::RenderObjects::OctahedronEdgeObject ObjectOctahedron{};
#else
	// Shared shaders for objects.
	RenderObjects::Mesh::FragmentShaders::TriangleShade::FillShader ObjectsShader{};

	// Mesh triangle objects.
	Assets::RenderObjects::CubeMeshObject ObjectCube{};
	Assets::RenderObjects::OctahedronMeshObject ObjectOctahedron{};
#endif

	Assets::SceneShaders::HeightSceneShader SceneShader{};

	// Track animation color style.
	bool Monochrome = false;

public:
	AnimatedTinyScene(TS::Scheduler& scheduler)
		: TS::Task(25, TASK_FOREVER, &scheduler, false)
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

		if (!engineRenderer.AddObject(&ObjectCube)
			|| !engineRenderer.AddObject(&ObjectOctahedron)
			)
		{
			TS::Task::disable();
			return false;
		}

		const uint8_t fovPercent = 30;
		engineRenderer.SetFov((uint32_t(UFRACTION16_1X) * (100 - fovPercent)) / 100);

		// Configure animation based on surface dimensions.
		ShapeMoveX = ((((int32_t)ShapeMove * 4) / 10) * width) / height;

		// Attach shaders to objects for rendering.
		ObjectCube.FragmentShader = &ObjectsShader;
		ObjectOctahedron.FragmentShader = &ObjectsShader;

		if (!Monochrome)
		{
			ObjectCube.SceneShader = &SceneShader;
			ObjectOctahedron.SceneShader = &SceneShader;
		}

		Monochrome = monochrome;

#if defined(DEMO_SCENE_EDGE_OBJECT)
		// Adjust animation on 1 bit color output.
		if (Monochrome)
		{
			ObjectCube.SetAlbedo(Rgb8::WHITE);
			ObjectOctahedron.SetAlbedo(Rgb8::WHITE);
		}
#endif

		TS::Task::enable();

		AnimateObjects(micros());

		return true;
	}

private:
	void AnimateObjects(const uint32_t timestamp)
	{
#if defined(DEMO_SCENE_EDGE_OBJECT)
		if (!Monochrome)
		{
			// Rainbow color pattern with HSV color.
			const ufraction16_t colorFraction = UFraction16::GetScalar((uint32_t)(timestamp % (ShapeColorPeriodMicros + 1)), ShapeColorPeriodMicros);
			ObjectOctahedron.SetAlbedo(Rgb8::ColorHsvFraction(colorFraction, UFRACTION16_1X, UFRACTION16_1X));
		}
#endif

		// Continuous rotation on all 3 axis.
		const ufraction16_t xRotateFraction = UFraction16::GetScalar<uint32_t>((timestamp % (ShapeRotatePeriodMicros + 1)), ShapeRotatePeriodMicros);
		const ufraction16_t yRotateFraction = UFraction16::GetScalar<uint32_t>((timestamp % (((ShapeRotatePeriodMicros * 1) / 3) + 1)), (ShapeRotatePeriodMicros * 1) / 3);
		const ufraction16_t zRotateFraction = UFraction16::GetScalar<uint32_t>((timestamp % (((ShapeRotatePeriodMicros * 2) / 3) + 1)), (ShapeRotatePeriodMicros * 2) / 3);

		ObjectCube.Rotation.x = Fraction(xRotateFraction, (uint16_t)ANGLE_RANGE);
		ObjectCube.Rotation.y = Fraction(yRotateFraction, (uint16_t)ANGLE_RANGE);
		ObjectCube.Rotation.z = Fraction(zRotateFraction, (uint16_t)ANGLE_RANGE);

		ObjectOctahedron.Rotation.x = ObjectCube.Rotation.x + Trigonometry::ANGLE_180;
		ObjectOctahedron.Rotation.y = ObjectCube.Rotation.y + Trigonometry::ANGLE_180;
		ObjectOctahedron.Rotation.z = ObjectCube.Rotation.z + Trigonometry::ANGLE_180;

		// Circle animation.
		const ufraction16_t circleFraction = UFraction16::GetScalar((uint32_t)(timestamp % (ShapeMovePeriodMicros + 1)), ShapeMovePeriodMicros);
		const angle_t circleAngle = Fraction(circleFraction, ANGLE_RANGE);

		const fraction16_t xMoveFraction1 = Sine16(circleAngle);
		const fraction16_t xMoveFraction2 = Sine16(circleAngle + Trigonometry::ANGLE_180);

		const fraction16_t zMoveFraction1 = Cosine16(circleAngle);
		const fraction16_t zMoveFraction2 = Cosine16(circleAngle + Trigonometry::ANGLE_180);

		ObjectCube.Translation.x = Fraction(xMoveFraction1, ShapeMoveX);
		ObjectCube.Translation.z = BaseDistance + Fraction(zMoveFraction1, ShapeMoveZ);

		ObjectOctahedron.Translation.x = Fraction(xMoveFraction2, ShapeMoveX);
		ObjectOctahedron.Translation.z = BaseDistance + Fraction(zMoveFraction2, ShapeMoveZ);
	}
};

#endif

