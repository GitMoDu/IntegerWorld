#ifndef _ANIMATED_DEMO_SCENE_h
#define _ANIMATED_DEMO_SCENE_h

#define _TASK_OO_CALLBACKS
#include <TSchedulerDeclarations.hpp>

#include <IntegerWorld.h>
#include "DemoSceneAssets.h"

using namespace IntegerWorld;


class AnimatedDemoScene : private TS::Task
{
public:
	static constexpr uint16_t ObjectsCount = 7;

	// Worst case scenario for all objects, with minimal back-face culling there's more than enough room for the few extra calls (lights).
	static constexpr uint16_t MaxDrawCallCount = Assets::Shapes::Sphere::TriangleCount
		+ Assets::Shapes::Star::TriangleCount
		+ Assets::Shapes::Cube::TriangleCount
		+ Assets::Shapes::Grid8x8::VertexCount
		;

private:
	static constexpr uint32_t FloorColorPeriodMicros = 3000000000;
	static constexpr uint32_t FlickerPeriodMicros = 6400000;
	static constexpr uint32_t FlashColorPeriodMicros = 3000000;
	static constexpr uint32_t ShapeColorPeriodMicros = 19000000;
	static constexpr uint32_t ShapeRotatePeriodMicros = 35000000;
	static constexpr uint32_t ShapeMovePeriodMicros = 15111111;

	static constexpr int16_t DistanceUnit = Assets::Shapes::SHAPE_UNIT;
	static constexpr int16_t BaseDistance = -(VERTEX16_UNIT * 8) / 10;
	static constexpr int16_t ShapeMove = (DistanceUnit * 30) / 10;
	static constexpr int16_t ShapeMoveZ = ShapeMove / 3;

	static constexpr uint16_t LightMinDistance = DistanceUnit / 10;
	static constexpr uint16_t LightMaxDistance = DistanceUnit * 5;

	int16_t ShapeMoveX = (((int32_t)ShapeMove * 4) / 10);

private:
	// World tracked light sources with sprite representation.
	Assets::Objects::ShadedLightSourceObject<
		PointLightSource> Light1{};
	Assets::Objects::ShadedLightSourceObject<
		PointLightSource> Light2{};

	// Global light source.
	DirectionalLightSource GlobalLight{};

	// Custom shader to draw the light source objects.
	Assets::Shaders::LightSourceFragmentShader LightSourceShader{};

	// Shader for this scene.
	static constexpr uint8_t LightsCount = 3;
	LightsShader<LightsCount> SceneShader{};

	// Sphere has a diffuse material.
	Assets::Objects::SphereMeshObject ObjectSphere{};

	// Star has a reflective, slightly metallic material with animated color.
	Assets::Objects::StarMeshObject ObjectStar{};

	// Cube has a palleted diffuse color for each face.
	Assets::Objects::CubeMeshObject ObjectCube{};

	// Shared mesh objects shaders.
	TriangleFillFragmentShader MeshShader{};

	// Floor has a custom shader to draw the diffuse floor object.
	Assets::Objects::FloorPointCloudObject ObjectFloor{};
	Assets::Shaders::FloorFragmentShader FloorShader{};

	// Simple screen fill background.
	FlatBackgroundObject Background{};
	BackgroundFlatFillShader BackgroundShader{};

public:
	AnimatedDemoScene(TS::Scheduler& scheduler) : TS::Task(10, TASK_FOREVER, &scheduler, true)
	{
	}

	bool Callback() final
	{
		AnimateObjects(micros());

		return true;
	}

	void Start(IEngineRenderer& engineRenderer, const int16_t width, const int16_t height)
	{
		// Configure animation based on surface dimensions.
		ShapeMoveX = ((((int32_t)ShapeMove * 4) / 10) * width) / height;

		// Place lights.
		Light1.Translation.x = -((ShapeMoveX * 2) / 3);
		Light1.Translation.y = -(DistanceUnit * 2) / 10;
		Light1.Translation.z = BaseDistance - ShapeMoveZ - ((DistanceUnit * 20) / 10);
		Light2.Translation.x = -Light1.Translation.x;
		Light2.Translation.y = Light1.Translation.y;
		Light2.Translation.z = Light1.Translation.z;

		// Configure and place floor.
		FloorShader.Radius = MaxValue(1, MinValue(width, height) / 96);
		ObjectFloor.Resize = Resize::GetResize16(uint8_t(22), uint8_t(1));
		ObjectFloor.Resize = Resize::GetResize16(uint8_t(25), uint8_t(1));
		ObjectFloor.Translation.x = Scale(ObjectFloor.Resize, int16_t(-DistanceUnit / 5));
		ObjectFloor.Translation.y = (DistanceUnit * 3) / 5;
		ObjectFloor.Translation.z = BaseDistance + Scale(ObjectFloor.Resize, int16_t(-DistanceUnit / 4));
		ObjectFloor.Rotation.x = Trigonometry::ANGLE_90;

		// Set the ambient color.
		SceneShader.AmbientLight = ColorFraction::RgbToColorFraction((uint32_t)0x18232D);

		// Setup the light sources.
		Light1.Color = ColorFraction::COLOR_RED;
		Light2.Color = ColorFraction::COLOR_GREEN;
		GlobalLight.Color = ColorFraction::RgbToColorFraction((uint32_t)0x9E8C76);
		LightSourceShader.MaxSize = MaxValue(2, MinValue(width, height) / 16);

		// Short range for a small scene.
		Light1.SetLightRange(LightMinDistance, LightMaxDistance);
		Light2.SetLightRange(LightMinDistance, LightMaxDistance);
		GlobalLight.SetIlluminationVector({ -VERTEX16_UNIT / 4, -VERTEX16_UNIT / 2, -VERTEX16_UNIT / 8 });

		// Attach light source to shader.
		SceneShader.ClearLights();
		SceneShader.AddLight(&GlobalLight);
		SceneShader.AddLight(&Light1);
		SceneShader.AddLight(&Light2);

		// Configure object materials.
		ObjectStar.Material = material_t{ 0, UFRACTION8_1X / 4, UFRACTION8_1X / 2 , UFRACTION8_1X / 8 };
		ObjectCube.Material = material_t{ 0, UFRACTION8_1X, 0, 0 };
		ObjectSphere.Material = material_t{ 0,UFRACTION8_1X, 0, 0 };

		// Configure background.
		Background.FragmentShader = &BackgroundShader;

		// Add all render objects to the pipeline, including light sources.
		engineRenderer.ClearObjects();
		engineRenderer.AddObject(&Background);
		engineRenderer.AddObject(&Light1);
		engineRenderer.AddObject(&Light2);
		engineRenderer.AddObject(&ObjectSphere);
		engineRenderer.AddObject(&ObjectStar);
		engineRenderer.AddObject(&ObjectCube);
		engineRenderer.AddObject(&ObjectFloor);

		// Attach shaders to objects for rendering.
		SetSceneShader(true);
		SetPixelShader();
	}

private:
	void AnimateObjects(const uint32_t timestamp)
	{
		// Start rainbow color pattern with HSV color.
		const ufraction16_t colorFraction = GetUFraction16((uint32_t)(timestamp % (ShapeColorPeriodMicros + 1)), ShapeColorPeriodMicros);
		ObjectStar.Color = ColorFraction::HsvToColorFraction(colorFraction, UFRACTION16_1X, UFRACTION16_1X);

		// Continuous rotation on all 3 axis.
		const ufraction32_t xRotateFraction = GetUFraction32((uint32_t)(timestamp % (ShapeRotatePeriodMicros + 1)), ShapeRotatePeriodMicros);
		const ufraction32_t yRotateFraction = GetUFraction32((uint32_t)(timestamp % (((ShapeRotatePeriodMicros * 1) / 3) + 1)), (ShapeRotatePeriodMicros * 1) / 3);
		const ufraction32_t zRotateFraction = GetUFraction32((uint32_t)(timestamp % (((ShapeRotatePeriodMicros * 2) / 3) + 1)), (ShapeRotatePeriodMicros * 2) / 3);

		ObjectSphere.Rotation.x = Fraction::Scale(xRotateFraction, (uint16_t)ANGLE_RANGE);
		ObjectSphere.Rotation.y = Fraction::Scale(yRotateFraction, (uint16_t)ANGLE_RANGE);
		ObjectSphere.Rotation.z = Fraction::Scale(zRotateFraction, (uint16_t)ANGLE_RANGE);

		ObjectStar.Rotation.x = ObjectSphere.Rotation.x + GetAngle(120);
		ObjectStar.Rotation.y = ObjectSphere.Rotation.y + GetAngle(120);
		ObjectStar.Rotation.z = ObjectSphere.Rotation.z + GetAngle(120);;

		ObjectCube.Rotation.x = ObjectSphere.Rotation.x + GetAngle(240);
		ObjectCube.Rotation.y = ObjectSphere.Rotation.y + GetAngle(240);
		ObjectCube.Rotation.z = ObjectSphere.Rotation.z + GetAngle(240);

		// Circle animation.
		const ufraction32_t circleFraction = GetUFraction32((uint32_t)(timestamp % (ShapeMovePeriodMicros + 1)), ShapeMovePeriodMicros);
		const angle_t circleAngle = Fraction::Scale(circleFraction, ANGLE_RANGE);

		const fraction32_t xMoveFraction1 = Sine32(circleAngle);
		const fraction32_t xMoveFraction2 = Sine32((int32_t)circleAngle + GetAngle(120));
		const fraction32_t xMoveFraction3 = Sine32((int32_t)circleAngle + GetAngle(240));

		const fraction32_t zMoveFraction1 = Cosine32((int32_t)circleAngle);
		const fraction32_t zMoveFraction2 = Cosine32((int32_t)circleAngle + GetAngle(120));
		const fraction32_t zMoveFraction3 = Cosine32((int32_t)circleAngle + GetAngle(240));

		ObjectSphere.Translation.x = Scale(xMoveFraction1, ShapeMoveX);
		ObjectStar.Translation.x = Scale(xMoveFraction2, ShapeMoveX);
		ObjectCube.Translation.x = Scale(xMoveFraction3, ShapeMoveX);

		ObjectSphere.Translation.z = BaseDistance + Scale(zMoveFraction1, ShapeMoveZ);
		ObjectStar.Translation.z = BaseDistance + Scale(zMoveFraction2, ShapeMoveZ);
		ObjectCube.Translation.z = BaseDistance + Scale(zMoveFraction3, ShapeMoveZ);
	}


public:
	void SetSceneShader(const bool enabled = true)
	{
		if (enabled)
		{
			ObjectSphere.SceneShader = &SceneShader;
			ObjectStar.SceneShader = &SceneShader;
			ObjectCube.SceneShader = &SceneShader;
			ObjectFloor.SceneShader = &SceneShader;
			Light1.SceneShader = &SceneShader;
			Light2.SceneShader = &SceneShader;
			Background.SceneShader = &SceneShader;
		}
		else
		{
			ObjectSphere.SceneShader = nullptr;
			ObjectStar.SceneShader = nullptr;
			ObjectCube.SceneShader = nullptr;
			ObjectFloor.SceneShader = nullptr;
			Light1.SceneShader = nullptr;
			Light2.SceneShader = nullptr;
			Background.SceneShader = nullptr;
		}
	}

	void SetPixelShader()
	{
		ObjectSphere.FragmentShader = &MeshShader;
		ObjectStar.FragmentShader = &MeshShader;
		ObjectCube.FragmentShader = &MeshShader;
		ObjectFloor.FragmentShader = &FloorShader;

		Light1.FragmentShader = &LightSourceShader;
		Light2.FragmentShader = &LightSourceShader;

		// Set background color as half the ambient light.
		Background.Color.r = SceneShader.AmbientLight.r >> 1;
		Background.Color.g = SceneShader.AmbientLight.g >> 1;
		Background.Color.b = SceneShader.AmbientLight.b >> 1;
	}
};
#endif