#ifndef _INTEGER_WORLD_EXPERIMENTAL_ANIMATED_DEMO_SCENE_h
#define _INTEGER_WORLD_EXPERIMENTAL_ANIMATED_DEMO_SCENE_h


#define _TASK_OO_CALLBACKS
#include <TSchedulerDeclarations.hpp>

#include "DemoSceneAssets.h"
#include "../Framework/Interface.h"

using namespace IntegerWorld;

/// <summary>
/// Integer World Animated Demo Scene.
/// Cross-platform demo scene showcasing various renderable objects, materials, lighting and animation.
/// 
/// Configuration options:
/// #define INTEGER_WORLD_FRUSTUM_DEBUG // Enable engine frustum visualization in scene.
/// #define INTEGER_WORLD_LIGHTS_SHADER_DEBUG // Enable light component toggles in the scene lights shader.
/// #define INTEGER_WORLD_TEXTURED_CUBE_DEMO // Use textured cube object in the demo scene instead of colored cube.
/// #define INTEGER_WORLD_TEXTURED_CUBE_HIGH_QUALITY // Use vertex lit cube object with perspective correct texture mapping.
/// </summary>
class AnimatedDemoScene : private TS::Task
{
public:
	static constexpr uint8_t ObjectsCount = 7
#if defined(INTEGER_WORLD_FRUSTUM_DEBUG)
		+Assets::Debug::FrustumDebugger::ObjectCount
#endif
		;

	// Worst case scenario for all objects.
	static constexpr uint16_t MaxDrawCallCount =
		Assets::Shapes::Sphere::TriangleCount
		+ Assets::Shapes::Star::TriangleCount
		+ Assets::Shapes::Cube::TriangleCount
		+ Assets::Shapes::Grid8x8::VertexCount;

private:
	enum class SceneLightsEnum : uint8_t
	{
		DirectionalLightGlobal,
		PointLightRed,
		PointLightGreen,
		EnumCount
	};

	static constexpr Rgb8::color_t Light1Color = Rgb8::RED;
	static constexpr Rgb8::color_t Light2Color = Rgb8::GREEN;
	static constexpr Rgb8::color_t Light3Color = Rgb8::BLUE;
	static constexpr Rgb8::color_t GlobalLightColor = 0xEDE9CB;
	static constexpr Rgb8::color_t AmbientLightColor = 0x0B1014;

	static constexpr uint32_t ShapeColorPeriodMicros = 19000000;
	static constexpr uint32_t ShapeRotatePeriodMicros = 35000000;
	static constexpr uint32_t ShapeMovePeriodMicros = 15111111;

	static constexpr int16_t DistanceUnit = Assets::Shapes::SHAPE_UNIT;
	static constexpr int16_t BaseDistance = (VERTEX16_UNIT * 9) / 10;
	static constexpr int16_t ShapeMove = (DistanceUnit * 30) / 10;
	static constexpr int16_t ShapeMoveZ = ShapeMove / 2;
	static constexpr uint16_t LightDimension = DistanceUnit / 3;

	static constexpr uint16_t LightMinDistance = 0;
	static constexpr uint16_t LightMaxDistance = DistanceUnit * 8;

	int16_t ShapeMoveX = (((int32_t)ShapeMove * 4) / 10);

private:
	// Light sources.
	SceneShaders::LightSource::light_source_t SceneLights[uint8_t(SceneLightsEnum::EnumCount)]{};

	// Billboard objects to track point light sources in the scene.
	RenderObjects::Billboard::AxisAlignedObject<> LightRedBillboardObject{};
	RenderObjects::Billboard::AxisAlignedObject<> LightGreenBillboardObject{};

	// Simple screen fill background.
	RenderObjects::Background::FillObject ObjectBackground{};

	// Sphere has a diffuse material.
	Assets::RenderObjects::SphereMeshVertexObject ObjectSphere{};

	// Star has a reflective, slightly metallic material with animated color.
	Assets::RenderObjects::StarMeshTriangleObject ObjectStar{};

#if defined(INTEGER_WORLD_TEXTURED_CUBE_DEMO)
	// Cube has a texture mapped on all faces.
#if defined(INTEGER_WORLD_TEXTURED_CUBE_HIGH_QUALITY)
	Assets::RenderObjects::CubeTexturedVertexObject ObjectCube{};
#else
	Assets::RenderObjects::CubeTexturedTriangleObject ObjectCube{};
#endif
#else
	// Cube has a palleted diffuse color for each face.
	Assets::RenderObjects::CubeMeshTriangleObject ObjectCube{};
#endif

	// Floor is a large grid point cloud.
	Assets::RenderObjects::FloorPointCloudObject ObjectFloor{};

	// Shared mesh triangle shaders.
	RenderObjects::Mesh::FragmentShaders::TriangleShade::FillShader TriangleShader{};
	RenderObjects::Mesh::FragmentShaders::TriangleShade::ZInterpolateShader TriangleZShader{};
	RenderObjects::Mesh::FragmentShaders::TriangleShade::WireframeShader TriangleWireframeShader{};
#if defined(INTEGER_WORLD_TEXTURED_CUBE_DEMO)
#if defined(INTEGER_WORLD_TEXTURED_CUBE_HIGH_QUALITY)
	Assets::FragmentShaders::CubeTexturedVertexLitShader TriangleTextureShader{};
	RenderObjects::Mesh::FragmentShaders::VertexShade::WireframeShader TriangleVertexWireframeShader{};
	RenderObjects::Mesh::FragmentShaders::VertexShade::ZInterpolateShader TriangleVertexZShader{};
#else
	Assets::FragmentShaders::CubeTexturedTriangleLitShader TriangleTextureShader{};
#endif
#endif

	// Shared mesh vertex shaders.
	RenderObjects::Mesh::FragmentShaders::VertexShade::ColorInterpolateShader VertexShader{};
	RenderObjects::Mesh::FragmentShaders::VertexShade::ZInterpolateShader VertexZShader{};
	RenderObjects::Mesh::FragmentShaders::VertexShade::WireframeShader VertexWireframeShader{};

	// Custom billboard shaders for point light sources.
	Assets::FragmentShaders::LightBillboardFragmentShader BillboardShader{};
	RenderObjects::Billboard::FragmentShaders::PlaceholderShader BillboardWireframeShade{};
	RenderObjects::Billboard::FragmentShaders::FillZShader BillboardZShader{};

	// Floor has a custom shader to draw the diffuse floor object.
	Assets::FragmentShaders::FloorFragmentShader FloorShader{};
	RenderObjects::PointCloud::FragmentShaders::PointShader PointShader{};
	RenderObjects::PointCloud::FragmentShaders::PointZShader PointZShader{};

	RenderObjects::Background::FragmentShaders::FillShader BackgroundShader{};

	// Lights shader for this scene, shared by objects.
	SceneShaders::LightSource::Shader LightSourceShader{};

	// Normal shader for debug visualization.
	SceneShaders::Normal::Shader NormalShader{};

	// Resumable animation trackers.
	uint32_t AnimationStart = 0;
	uint32_t AnimationPause = 0;

#if defined(INTEGER_WORLD_FRUSTUM_DEBUG)
	// Frustum debugger to visualize the camera frustum in the scene.
	Assets::Debug::FrustumDebugger FrustumVisualizer{};
#endif

public:
	AnimatedDemoScene(TS::Scheduler& scheduler)
		: TS::Task(4, TASK_FOREVER, &scheduler, false)
	{
	}

	bool Callback() final
	{
		const uint32_t timestamp = micros();
		AnimateObjects(timestamp - AnimationStart);

		return true;
	}

	bool Start(IEngineRenderer& engineRenderer, const int16_t width, const int16_t height)
	{
		// Clear all objects from the renderer.
		engineRenderer.ClearObjects();

		// Add all scene objects to the pipeline.
		if (!engineRenderer.AddObject(&ObjectSphere)
			|| !engineRenderer.AddObject(&ObjectStar)
			|| !engineRenderer.AddObject(&ObjectCube)
			|| !engineRenderer.AddObject(&ObjectBackground)
			|| !engineRenderer.AddObject(&LightRedBillboardObject)
			|| !engineRenderer.AddObject(&LightGreenBillboardObject)
			|| !engineRenderer.AddObject(&ObjectFloor)
			)
		{
			SetAnimationEnabled(false);
			return false;
		}

		uint8_t fovPercent = 30;
		engineRenderer.SetFov((uint32_t(UFRACTION16_1X) * (100 - fovPercent)) / 100);

		// Configure animation based on surface dimensions.
		ShapeMoveX = ((((int32_t)ShapeMove * 4) / 10) * width) / height;

		// Setup lights.
		SceneLights[uint8_t(SceneLightsEnum::DirectionalLightGlobal)] = SceneShaders::LightSource::DirectionalLightSource(GlobalLightColor,
			{ VERTEX16_UNIT, VERTEX16_UNIT, VERTEX16_UNIT }, UFRACTION16_1X / 4);

		SceneLights[uint8_t(SceneLightsEnum::PointLightRed)] = SceneShaders::LightSource::PointLightSource(Light1Color,
			{ int16_t(-(ShapeMoveX * 4) / 10) , int16_t(-(DistanceUnit * 3) / 20) , int16_t((DistanceUnit * 9) / 10) },
			LightMinDistance, LightMaxDistance);

		SceneLights[uint8_t(SceneLightsEnum::PointLightGreen)] = SceneShaders::LightSource::PointLightSource(Light2Color * 0,
			{ int16_t((ShapeMoveX * 4) / 10) , int16_t(-(DistanceUnit * 3) / 20) , int16_t((DistanceUnit * 9) / 10) },
			LightMinDistance, LightMaxDistance);

		// Attach light sources to lights shader.
		LightSourceShader.SetLights(SceneLights, uint8_t(SceneLightsEnum::EnumCount));

		// Set the ambient color.
		LightSourceShader.AmbientLight = AmbientLightColor;

		// Lights track the camera position.
		LightSourceShader.CameraPosition = &engineRenderer.GetCameraControls()->Position;

		// Place the billboards on the lights' position.
		LightRedBillboardObject.Translation = SceneLights[uint8_t(SceneLightsEnum::PointLightRed)].Position;
		LightGreenBillboardObject.Translation = SceneLights[uint8_t(SceneLightsEnum::PointLightGreen)].Position;
		LightRedBillboardObject.SetDimensions(LightDimension, LightDimension);
		LightGreenBillboardObject.SetDimensions(LightDimension, LightDimension);

		// Configure and place floor.
		FloorShader.Radius = MaxValue(1, MinValue(width, height) / 96);
		ObjectFloor.Resize = Scale16::GetFactor(uint8_t(28), uint8_t(1));
		ObjectFloor.SetNormal({ 0, -VERTEX16_UNIT, 0 });
		ObjectFloor.Translation.y = (DistanceUnit * 3) / 5;
		ObjectFloor.Translation.z = BaseDistance;

		// Configure object dynamic materials.
		ObjectSphere.SetMaterial(material_t{ 0, UFRACTION8_1X, 0, 0 });
		ObjectSphere.SetMaterial(material_t{ 0, UFRACTION8_1X, 0, 0 });
		ObjectStar.SetMaterial(material_t{ 0, UFRACTION8_1X / 8, UFRACTION8_1X, 0 });

		// Configure background shader.
		ObjectBackground.FragmentShader = &BackgroundShader;

		// Attach shaders to objects for rendering.
		SetFragmentShaderLights();
		SetLightsShaderLightSource();

		// Set the lights.
		SetLightRedEnabled(true);
		SetLightGreenEnabled(true);
		SetLightGlobalEnabled(true);

#if defined(INTEGER_WORLD_FRUSTUM_DEBUG)
		// Start the frustum visualizer.
		if (!FrustumVisualizer.Start(engineRenderer))
		{
			return false;
		}
#endif

		// Start the animation.
		SetAnimationEnabled(true);

		return true;
	}

private:
	void AnimateObjects(const uint32_t timestamp)
	{
		// Start rainbow color pattern with HSV color.
		const ufraction16_t colorFraction = UFraction16::GetScalar((uint32_t)(timestamp % (ShapeColorPeriodMicros + 1)), ShapeColorPeriodMicros);
		ObjectStar.SetAlbedo(Rgb8::ColorHsvFraction(colorFraction, UFRACTION16_1X, UFRACTION16_1X));

		// Continuous rotation on all 3 axis.
		const ufraction32_t xRotateFraction = UFraction32::GetScalar<uint32_t>((timestamp % (ShapeRotatePeriodMicros + 1)), ShapeRotatePeriodMicros);
		const ufraction32_t yRotateFraction = UFraction32::GetScalar<uint32_t>((timestamp % (((ShapeRotatePeriodMicros * 1) / 3) + 1)), (ShapeRotatePeriodMicros * 1) / 3);
		const ufraction32_t zRotateFraction = UFraction32::GetScalar<uint32_t>((timestamp % (((ShapeRotatePeriodMicros * 2) / 3) + 1)), (ShapeRotatePeriodMicros * 2) / 3);

		ObjectSphere.Rotation.x = Fraction(xRotateFraction, (uint16_t)ANGLE_RANGE);
		ObjectSphere.Rotation.y = Fraction(yRotateFraction, (uint16_t)ANGLE_RANGE);
		ObjectSphere.Rotation.z = Fraction(zRotateFraction, (uint16_t)ANGLE_RANGE);

		ObjectStar.Rotation.x = ObjectSphere.Rotation.x + GetAngle(120);
		ObjectStar.Rotation.y = ObjectSphere.Rotation.y + GetAngle(120);
		ObjectStar.Rotation.z = ObjectSphere.Rotation.z + GetAngle(120);;

		ObjectCube.Rotation.x = ObjectSphere.Rotation.x + GetAngle(240);
		ObjectCube.Rotation.y = ObjectSphere.Rotation.y + GetAngle(240);
		ObjectCube.Rotation.z = ObjectSphere.Rotation.z + GetAngle(240);

		// Circle animation.
		const ufraction32_t circleFraction = UFraction32::GetScalar((uint32_t)(timestamp % (ShapeMovePeriodMicros + 1)), ShapeMovePeriodMicros);
		const angle_t circleAngle = Fraction(circleFraction, ANGLE_RANGE);

		const fraction32_t xMoveFraction1 = Sine32(circleAngle);
		const fraction32_t xMoveFraction2 = Sine32((int32_t)circleAngle + GetAngle(120));
		const fraction32_t xMoveFraction3 = Sine32((int32_t)circleAngle + GetAngle(240));

		const fraction32_t zMoveFraction1 = Cosine32((int32_t)circleAngle);
		const fraction32_t zMoveFraction2 = Cosine32((int32_t)circleAngle + GetAngle(120));
		const fraction32_t zMoveFraction3 = Cosine32((int32_t)circleAngle + GetAngle(240));

		ObjectSphere.Translation.x = Fraction(xMoveFraction1, ShapeMoveX);
		ObjectStar.Translation.x = Fraction(xMoveFraction2, ShapeMoveX);
		ObjectCube.Translation.x = Fraction(xMoveFraction3, ShapeMoveX);

		ObjectSphere.Translation.z = BaseDistance + Fraction(zMoveFraction1, ShapeMoveZ);
		ObjectStar.Translation.z = BaseDistance + Fraction(zMoveFraction2, ShapeMoveZ);
		ObjectCube.Translation.z = BaseDistance + Fraction(zMoveFraction3, ShapeMoveZ);
	}

public:
	void CaptureViewFrustum()
	{
#if defined(INTEGER_WORLD_FRUSTUM_DEBUG)
		FrustumVisualizer.CaptureViewFrustum();
#endif
	}

	void SetFragmentShaderWireframe()
	{
		ObjectBackground.Color = Rgb8::BLACK;
		ObjectFloor.FragmentShader = &PointShader;
		ObjectSphere.FragmentShader = &VertexWireframeShader;
		ObjectStar.FragmentShader = &TriangleWireframeShader;
#if defined(INTEGER_WORLD_TEXTURED_CUBE_HIGH_QUALITY)
		ObjectCube.FragmentShader = &TriangleVertexWireframeShader;
#else
		ObjectCube.FragmentShader = &TriangleWireframeShader;
#endif

		LightRedBillboardObject.FragmentShader = &BillboardWireframeShade;
		LightGreenBillboardObject.FragmentShader = &BillboardWireframeShade;
	}

	void SetFragmentShaderZ()
	{
		ObjectBackground.Color = Rgb8::BLACK;
		ObjectFloor.FragmentShader = &PointZShader;
		ObjectSphere.FragmentShader = &VertexZShader;
		ObjectStar.FragmentShader = &TriangleZShader;

#if defined(INTEGER_WORLD_TEXTURED_CUBE_HIGH_QUALITY)
		ObjectCube.FragmentShader = &TriangleVertexZShader;
#else
		ObjectCube.FragmentShader = &TriangleZShader;
#endif

		LightRedBillboardObject.FragmentShader = &BillboardZShader;
		LightGreenBillboardObject.FragmentShader = &BillboardZShader;
	}

	void SetFragmentShaderLights()
	{
		// Set background color as half the ambient light.
		ObjectBackground.Color = Rgb8::Color(Rgb8::Red(AmbientLightColor) >> 1,
			Rgb8::Green(AmbientLightColor) >> 1,
			Rgb8::Blue(AmbientLightColor) >> 1);

		ObjectSphere.FragmentShader = &VertexShader;
		ObjectStar.FragmentShader = &TriangleShader;
#if defined(INTEGER_WORLD_TEXTURED_CUBE_DEMO)
		ObjectCube.FragmentShader = &TriangleTextureShader;
#else
		ObjectCube.FragmentShader = &TriangleShader;
#endif
		ObjectFloor.FragmentShader = &FloorShader;

		LightRedBillboardObject.FragmentShader = &BillboardShader;
		LightGreenBillboardObject.FragmentShader = &BillboardShader;
	}

	void SetLightsShaderLightSource()
	{
		ObjectCube.SceneShader = &LightSourceShader;
		ObjectSphere.SceneShader = &LightSourceShader;
		ObjectStar.SceneShader = &LightSourceShader;
		ObjectFloor.SceneShader = &LightSourceShader;
	}

	void SetLightsShaderNone()
	{
		ObjectCube.SceneShader = nullptr;
		ObjectSphere.SceneShader = nullptr;
		ObjectStar.SceneShader = nullptr;
		ObjectFloor.SceneShader = nullptr;
	}

	void SetLightsShaderNormal()
	{
		ObjectCube.SceneShader = &NormalShader;
		ObjectSphere.SceneShader = &NormalShader;
		ObjectStar.SceneShader = &NormalShader;
		ObjectFloor.SceneShader = &NormalShader;
	}

	void SetLightRedEnabled(const bool enabled)
	{
		if (enabled)
		{
			SceneLights[uint8_t(SceneLightsEnum::PointLightRed)].Color = Light1Color;
		}
		else
		{
			SceneLights[uint8_t(SceneLightsEnum::PointLightRed)].Color = 0;
		}

		LightRedBillboardObject.SetAlbedo(SceneLights[uint8_t(SceneLightsEnum::PointLightRed)].Color);
	}

	void SetLightGreenEnabled(const bool enabled)
	{
		if (enabled)
		{
			SceneLights[uint8_t(SceneLightsEnum::PointLightGreen)].Color = Light2Color;
		}
		else
		{
			SceneLights[uint8_t(SceneLightsEnum::PointLightGreen)].Color = 0;
		}

		LightGreenBillboardObject.SetAlbedo(SceneLights[uint8_t(SceneLightsEnum::PointLightGreen)].Color);
	}

	void SetLightGlobalEnabled(const bool enabled)
	{
		if (enabled)
		{
			SceneLights[uint8_t(SceneLightsEnum::DirectionalLightGlobal)].Color = GlobalLightColor;
		}
		else
		{
			SceneLights[uint8_t(SceneLightsEnum::DirectionalLightGlobal)].Color = 0;
		}
	}

	void SetAmbientShadeEnabled(const bool enabled)
	{
#if defined(INTEGER_WORLD_LIGHTS_SHADER_DEBUG)
		LightSourceShader.Ambient = enabled;
#endif
	}

	void SetEmissiveShadeEnabled(const bool enabled)
	{
#if defined(INTEGER_WORLD_LIGHTS_SHADER_DEBUG)
		LightSourceShader.Emissive = enabled;
#endif
	}

	void SetDiffuseShadeEnabled(const bool enabled)
	{
#if defined(INTEGER_WORLD_LIGHTS_SHADER_DEBUG)
		LightSourceShader.Diffuse = enabled;
#endif
	}

	void SetSpecularShadeEnabled(const bool enabled)
	{
#if defined(INTEGER_WORLD_LIGHTS_SHADER_DEBUG)
		LightSourceShader.Specular = enabled;
#endif
	}

	void SetAnimationEnabled(const bool enabled)
	{
		if (enabled != TS::Task::isEnabled())
		{
			const uint32_t timestamp = micros();

			if (enabled)
			{
				const uint32_t skipped = timestamp - AnimationPause;

				AnimationStart += skipped;
				TS::Task::enable();

				AnimateObjects(micros() - AnimationStart);
			}
			else
			{
				AnimationPause = timestamp;
				TS::Task::disable();
			}
		}
	}
};
#endif