#ifndef _INTEGER_WORLD_CHRISTMAS_SCENE_h
#define _INTEGER_WORLD_CHRISTMAS_SCENE_h

#include <IntegerWorld.h>
#include "Assets.h"


/// <summary>
/// </summary>
class ChristmasScene : public IFrameListener
{
public:
	static constexpr uint8_t ObjectsCount = 12;

	// Worst case scenario for all objects.
	static constexpr uint16_t MaxDrawCallCount = 1024;

private:
	enum class LightsEnum : uint8_t
	{
		// Start the lights after the Christmas tree lights.
		StarLight = Assets::ChristmasLights::LightsCount,
		GlobalDirectionalLight = StarLight + 1,
		TrunkLight = GlobalDirectionalLight + 1,
		EnumCount
	};

private:
	// Scene lights array.
	SceneShaders::LightSource::light_source_t SceneLights[uint8_t(LightsEnum::EnumCount)]{};

	// Lights shader for this scene, shared by objects.
	SceneShaders::LightSource::Shader LightSourceShader{};

	// Scene objects.
	Assets::RenderObjects::CarpetTextured32x32TriangleObject ObjectCarpet{};
	Assets::RenderObjects::TreeTrunkTriangleObject ObjectTrunk{};
	Assets::RenderObjects::TreeLeavesVertexObject ObjectLeaves{};
	Assets::RenderObjects::GlobesTriangleObject ObjectGlobes{};
	Assets::RenderObjects::StarTriangleObject ObjectStar{};
	Assets::RenderObjects::StarGlowPointCloudObject ObjectStarGlow{};
	Assets::RenderObjects::StringLightsPointCloudObject ObjectStringLights{};
	Assets::RenderObjects::ChristmasPresentTriangleObject<0> ObjectPresent1{};
	Assets::RenderObjects::ChristmasPresentTriangleObject<1> ObjectPresent2{};
	Assets::RenderObjects::ChristmasPresentTriangleObject<2> ObjectPresent3{};
	Assets::RenderObjects::ChristmasPresentTriangleObject<3> ObjectPresent4{};
	Assets::RenderObjects::ChristmasPresentTriangleObject<4> ObjectPresent5{};

	// Fragment shaders.
	IntegerWorld::RenderObjects::Mesh::FragmentShaders::TriangleShade::FillShader TriangleShader{};
	IntegerWorld::RenderObjects::Mesh::FragmentShaders::VertexShade::ColorInterpolateShader<> VertexShader{};
	Assets::FragmentShaders::TemplateGlowFragmentShader<Assets::Styles::Dimensions::StringLightsGlowRadius> StringLightsShader{};
	Assets::FragmentShaders::TemplateGlowFragmentShader<Assets::Styles::Dimensions::StarLightGlowRadius> StarGlowShader{};
	Assets::FragmentShaders::CarpetTexturedTriangleUnlitShader CarpetShader{};

	// Pointer to engine camera controls.
	camera_state_t* CameraControls = nullptr;

public:
	ChristmasScene(IEngineRenderer& engineRenderer)
		: IFrameListener()
	{
	}

	bool Start(IEngineRenderer& engineRenderer, const int16_t width, const int16_t height)
	{
		// Clear all objects from the renderer.
		engineRenderer.ClearObjects();

		// Add all scene objects to the pipeline.
		if (!engineRenderer.AddObject(&ObjectCarpet)
			|| !engineRenderer.AddObject(&ObjectTrunk)
			|| !engineRenderer.AddObject(&ObjectLeaves)
			|| !engineRenderer.AddObject(&ObjectGlobes)
			|| !engineRenderer.AddObject(&ObjectStar)
			|| !engineRenderer.AddObject(&ObjectStarGlow)
			|| !engineRenderer.AddObject(&ObjectStringLights)
			|| !engineRenderer.AddObject(&ObjectPresent1)
			|| !engineRenderer.AddObject(&ObjectPresent2)
			|| !engineRenderer.AddObject(&ObjectPresent3)
			|| !engineRenderer.AddObject(&ObjectPresent4)
			|| !engineRenderer.AddObject(&ObjectPresent5)
			)
		{
			engineRenderer.ClearObjects();
			return false;
		}

		// Set camera controls pointer.
		CameraControls = engineRenderer.GetCameraControls();

		// Set frame listener to update scene each frame.
		engineRenderer.SetFrameListener(this);

		// Set scene field of view.
		engineRenderer.SetFov((uint32_t(UFRACTION16_1X) * Assets::Styles::Scene::SceneFoVPercent) / 100);

		// Configure scene lights.
		SceneLights[uint8_t(LightsEnum::GlobalDirectionalLight)] = SceneShaders::LightSource::DirectionalLightSource(
			Assets::Styles::Colors::GlobalLight,
			Assets::Styles::Scene::AmbientLightDirection);
		SceneLights[uint8_t(LightsEnum::StarLight)] = SceneShaders::LightSource::PointLightSource(
			Assets::Styles::Colors::StarGlow, GetStarGlowPositionAverage(),
			0, Assets::Styles::Dimensions::GlowLightMaxDistance);
		SceneLights[uint8_t(LightsEnum::TrunkLight)] = SceneShaders::LightSource::PointLightSource(
			Assets::Styles::Colors::YellowishBright, { 0, 1, 0 },
			0, Assets::Styles::Dimensions::TrunkLightMaxDistance);

		// Configure string lights.
		for (size_t i = 0; i < Assets::ChristmasLights::LightsCount; i++)
		{
			SceneLights[i] = SceneShaders::LightSource::PointLightSource(
				Rgb8::BLACK, Assets::ChristmasLights::Vertices[i],
				0, Assets::Styles::Dimensions::StringLightsMaxDistance);
		}

		// Configure light's shader.
		LightSourceShader.CameraPosition = &engineRenderer.GetCameraControls()->Position;
		LightSourceShader.SetLights(SceneLights, uint8_t(LightsEnum::EnumCount));
		LightSourceShader.AmbientLight = Assets::Styles::Colors::AmbientLight;

		// Configure objects and their shaders.
		ObjectCarpet.FragmentShader = &CarpetShader;
		ObjectLeaves.FragmentShader = &VertexShader;
		ObjectLeaves.SceneShader = &LightSourceShader;
		ObjectTrunk.FragmentShader = &TriangleShader;
		ObjectTrunk.SceneShader = &LightSourceShader;
		ObjectStar.FragmentShader = &TriangleShader;
		ObjectStar.SceneShader = &LightSourceShader;
		ObjectStarGlow.FragmentShader = &StarGlowShader;
		ObjectStarGlow.AlbedoSource = &SceneLights[uint8_t(LightsEnum::StarLight)].Color;
		ObjectStarGlow.SetPosition(GetStarGlowPositionAverage());
		ObjectStringLights.FragmentShader = &StringLightsShader;
		ObjectStringLights.SetLights(SceneLights);
		ObjectGlobes.FragmentShader = &TriangleShader;
		ObjectGlobes.SceneShader = &LightSourceShader;
		ObjectPresent1.FragmentShader = &TriangleShader;
		ObjectPresent1.SceneShader = &LightSourceShader;
		PlacePresent(ObjectPresent1, 0);
		ObjectPresent2.FragmentShader = &TriangleShader;
		ObjectPresent2.SceneShader = &LightSourceShader;
		PlacePresent(ObjectPresent2, 1);
		ObjectPresent3.FragmentShader = &TriangleShader;
		ObjectPresent3.SceneShader = &LightSourceShader;
		PlacePresent(ObjectPresent3, 2);
		ObjectPresent4.FragmentShader = &TriangleShader;
		ObjectPresent4.SceneShader = &LightSourceShader;
		PlacePresent(ObjectPresent4, 3);
		ObjectPresent5.FragmentShader = &TriangleShader;
		ObjectPresent5.SceneShader = &LightSourceShader;
		PlacePresent(ObjectPresent5, 4);

		// Initial update.
		OnFrameStart();

		return true;
	}

	void OnFrameStart(const uint32_t predictedDisplayPeriod) final
	{

	}

	void OnFrameStart() final
	{
		const uint32_t timestamp = micros();

		if (CameraControls != nullptr)
		{
			// Orbit rotation around Y axis.
			const ufraction32_t rotateFraction = UFraction32::GetScalar<uint32_t>((timestamp % (Assets::Styles::Scene::OrbitPeriod + 1)), Assets::Styles::Scene::OrbitPeriod);
			CameraControls->Rotation.y = Fraction(rotateFraction, (uint16_t)ANGLE_RANGE);

			// Orbit camera position to match rotation.
			CameraControls->Position = { 0, 0, -Assets::Styles::Scene::OrbitRadius };
			transform16_rotate_t orbitTransform{};
			CalculateTransformRotation(orbitTransform, Assets::Styles::Scene::OrbitAngle, CameraControls->Rotation.y, 0);
			ApplyTransform(orbitTransform, CameraControls->Position);
			CameraControls->Position.y += Assets::Styles::Scene::OrbitHeight;
		}

		// Animate star light twinkle.
		const ufraction16_t twinkleFraction = CandleTwinkleFraction(timestamp);
		SceneLights[uint8_t(LightsEnum::StarLight)].Color = Rgb8::Color(
			Fraction(twinkleFraction, Rgb8::Red(Assets::Styles::Colors::StarGlow)),
			Fraction(twinkleFraction, Rgb8::Green(Assets::Styles::Colors::StarGlow)),
			Fraction(twinkleFraction, Rgb8::Blue(Assets::Styles::Colors::StarGlow)));

		// Animate string lights color and intensity.
		const ufraction16_t colorFraction = UFraction16::GetScalar(
			(uint32_t)(timestamp % (Assets::Styles::Scene::StringLightsColorPeriod + 1)),
			Assets::Styles::Scene::StringLightsColorPeriod);
		const ufraction16_t intensityFraction = UFraction16::GetScalar(
			(uint32_t)(timestamp % (Assets::Styles::Scene::StringLightsIntensityPeriod + 1)),
			Assets::Styles::Scene::StringLightsIntensityPeriod);
		static constexpr ufraction16_t colorOffset = UFRACTION16_1X / 7;
		static constexpr ufraction16_t intensityOffset = UFRACTION16_1X / 4;
		for (size_t i = 0; i < Assets::ChristmasLights::LightsCount; i++)
		{
			const ufraction16_t lightColorFraction = (colorFraction - (colorOffset * i)) % UFRACTION16_1X;
			ufraction16_t lightIntensity = (intensityFraction + (intensityOffset * i)) % UFRACTION16_1X;

			if (lightIntensity > (UFRACTION16_1X / 2))
			{
				lightIntensity = UFRACTION16_1X - ((lightIntensity - (UFRACTION16_1X / 2)) << 1);
			}
			else
			{
				lightIntensity = lightIntensity << 1;
			}

			SceneLights[i].Color = Rgb8::ColorHsvFraction(lightColorFraction, 
				Assets::Styles::Colors::StringLightsColorSaturation, lightIntensity);
		}
	}

public:
	/// <summary>
	/// Extract and print the center positions of the Christmas lights and light sources.
	/// Lights positions are set by averaging the triangle centers of each group, each group being an object that represents a light.
	/// </summary>
	static void PrintLightsPositions()
	{
		Serial.println(F("Christsmas Lights:"));
		PrintTriangleCenters(Assets::Shapes::ChristmasLights::Group, Assets::Shapes::ChristmasLights::TriangleCount,
			Assets::Shapes::ChristmasLights::Vertices, Assets::Shapes::ChristmasLights::VertexCount,
			Assets::Shapes::ChristmasLights::Triangles, Assets::Shapes::ChristmasLights::TriangleCount);

		Serial.println(F("Light Sources:"));
		PrintTriangleCenters(Assets::Shapes::LightSources::Group, Assets::Shapes::LightSources::TriangleCount,
			Assets::Shapes::LightSources::Vertices, Assets::Shapes::LightSources::VertexCount,
			Assets::Shapes::LightSources::Triangles, Assets::Shapes::LightSources::TriangleCount);
	}

private:
	template<typename PresentType>
	void PlacePresent(PresentType& present, const uint8_t presentIndex)
	{
		const auto presentBottomY = GetPresentsBottomY();

		const angle_t theta = Assets::Styles::Dimensions::PresentPlacementAngleOffset + static_cast<angle_t>((uint32_t(ANGLE_RANGE) * presentIndex) / Assets::ChristmasPresent::PresentCount);

		present.Resize = Assets::ChristmasPresent::Scales[presentIndex];
		present.Rotation = Assets::ChristmasPresent::Rotations[presentIndex];

		// Base point on the -Z axis, then rotate around Y by theta
		vertex16_t p = { 0,  0, -Assets::Styles::Dimensions::PresentPlacementRadius };
		transform16_rotate_t rot{};
		CalculateTransformRotation(rot, 0, theta, 0);
		ApplyTransform(rot, p);

		p.y = -Scale(present.Resize, presentBottomY);
		present.Translation = p;
	}

	vertex16_t GetStarGlowPositionTop() const
	{
		vertex16_t top(Assets::Shapes::ChristmasStar::Vertices[0]);
		for (uint16_t i = 1; i < Assets::Shapes::ChristmasStar::VertexCount; i++)
		{
			if (Assets::Shapes::ChristmasStar::Vertices[i].y > top.y)
			{
				top = Assets::Shapes::ChristmasStar::Vertices[i];
			}
		}
		return top;
	}

	vertex16_t GetStarGlowPositionAverage() const
	{
		int64_t xSum = 0;
		int64_t ySum = 0;
		int64_t zSum = 0;
		for (uint16_t i = 0; i < Assets::Shapes::ChristmasStar::VertexCount; i++)
		{
			xSum += Assets::Shapes::ChristmasStar::Vertices[i].x;
			ySum += Assets::Shapes::ChristmasStar::Vertices[i].y;
			zSum += Assets::Shapes::ChristmasStar::Vertices[i].z;
		}
		return vertex16_t{
			static_cast<int16_t>(xSum / Assets::Shapes::ChristmasStar::VertexCount),
			static_cast<int16_t>(ySum / Assets::Shapes::ChristmasStar::VertexCount),
			static_cast<int16_t>(zSum / Assets::Shapes::ChristmasStar::VertexCount)
		};
	}

	int16_t GetPresentsBottomY() const
	{
		int16_t bottomY = Assets::Shapes::ChristmasPresent::Vertices[0].y;
		for (uint16_t i = 1; i < Assets::Shapes::ChristmasPresent::VertexCount; i++)
		{
			if (Assets::Shapes::ChristmasPresent::Vertices[i].y < bottomY)
			{
				bottomY = Assets::Shapes::ChristmasPresent::Vertices[i].y;
			}
		}
		return bottomY;
	}

	vertex16_t GetStarGlowMiddle() const
	{
		vertex16_t top(Assets::Shapes::ChristmasStar::Vertices[0]);
		vertex16_t bottom(Assets::Shapes::ChristmasStar::Vertices[0]);

		for (uint16_t i = 1; i < Assets::Shapes::ChristmasStar::VertexCount; i++)
		{
			const auto& v = Assets::Shapes::ChristmasStar::Vertices[i];
			if (v.y > top.y)     top = v;
			if (v.y < bottom.y)  bottom = v;
		}
		return vertex16_t{
			static_cast<int16_t>((static_cast<int32_t>(top.x) + bottom.x) / 2),
			static_cast<int16_t>((static_cast<int32_t>(top.y) + bottom.y) / 2),
			static_cast<int16_t>((static_cast<int32_t>(top.z) + bottom.z) / 2)
		};
	}

	// Fast integer triangle wave in [0..UFRACTION16_1X] using a period in microseconds.
	static ufraction16_t TriangleWave16(const uint32_t t, const uint32_t periodMicros)
	{
		const uint32_t p = periodMicros ? periodMicros : 1;
		const uint32_t phase = t % p;                // [0..period-1]
		const uint32_t half = p >> 1;                // half period
		const uint32_t up = (phase <= half) ? phase : (p - phase);
		// Scale up to unit range: up in [0..half] -> [0..1]
		return UFraction16::GetScalar(up, half ? half : 1);
	}

	// Tiny hash-based jitter in [0..UFRACTION16_1X/16], varies quickly but remains subtle.
	static ufraction16_t CandleJitter16(const uint32_t t)
	{
		// Xorshift32-like hash; mix microseconds to avoid resonance
		uint32_t x = t ^ 0xA3C59AC3u;
		x ^= x << 13;
		x ^= x >> 17;
		x ^= x << 5;
		// Map to [0..1/16] range
		const uint16_t jitter8 = static_cast<uint16_t>((x >> 24) & 0xFF); // 0..255
		const ufraction16_t jitter = Fraction(static_cast<ufraction8_t>(jitter8), static_cast<uint16_t>(UFraction16::FRACTION_1X / 16));
		return jitter;
	}

	// Candle-like twinkle in [0..UFRACTION16_1X]
	static ufraction16_t CandleTwinkleFraction(const uint32_t timestampMicros)
	{
		// Base slow waveform.
		static constexpr uint32_t basePeriod = 3400000;
		static constexpr uint32_t harm1Period = 180000;
		static constexpr uint32_t harm2Period = 97000;

		// Compose: floor + effective range scaled by base + small variations
		// Floor prevents full black dips; effective lifts the range above floor.
		static constexpr ufraction16_t floor = (UFRACTION16_1X * 10) / 100;        // 0.10
		static constexpr ufraction16_t effective = UFRACTION16_1X - floor;         // 0.90

		const ufraction16_t base = TriangleWave16(timestampMicros, basePeriod); // [0..1]

		// Harmonics: faster, lower amplitude components
		const ufraction16_t harm1 = TriangleWave16(timestampMicros, harm1Period);
		const ufraction16_t harm2 = TriangleWave16(timestampMicros, harm2Period);

		// Weight harmonics (soft contribution)
		const ufraction16_t harm1W = Fraction(harm1, static_cast<ufraction16_t>(UFRACTION16_1X / 6)); // ~0.166
		const ufraction16_t harm2W = Fraction(harm2, static_cast<ufraction16_t>(UFRACTION16_1X / 10)); // ~0.1

		// Jitter: very subtle fast noise
		const ufraction16_t jitter = CandleJitter16(timestampMicros); // ~0..1/16

		// baseEffective = floor + effective*base
		const ufraction16_t baseEffective = floor + Fraction(effective, base);

		// Add micro-variations and clamp
		uint32_t mix = baseEffective;
		mix += harm1W;
		mix += harm2W;
		mix += jitter;

		if (mix > UFRACTION16_1X) mix = UFRACTION16_1X;
		return static_cast<ufraction16_t>(mix);
	}

	static void PrintTriangleCenters(
		const uint8_t* groups,
		size_t groupCount,
		const vertex16_t* vertices,
		size_t vertexCount,
		const triangle_face_t* triangles,
		size_t triangleCount)
	{
		uint8_t currentGroup = UINT8_MAX;

		int32_t xSum = 0;
		int32_t ySum = 0;
		int32_t zSum = 0;
		uint16_t count = 0;

		Serial.println(F("static constexpr vertex16_t Vertices[] PROGMEM\n{"));

		for (uint16_t i = 0; i < groupCount; i++)
		{
			if (currentGroup != groups[i])
			{
				if (count > 0)
				{
					xSum = xSum / count;
					ySum = ySum / count;
					zSum = zSum / count;

					Serial.print(F("{ "));
					Serial.print(xSum);
					Serial.print(',');
					Serial.print(ySum);
					Serial.print(',');
					Serial.print(zSum);
					Serial.println(F("},"));
				}

				currentGroup = groups[i];
				xSum = 0;
				ySum = 0;
				zSum = 0;
				count = 0;
			}

			const triangle_face_t& tri = triangles[i];

			xSum += static_cast<int32_t>(vertices[tri.a].x) + static_cast<int32_t>(vertices[tri.b].x) + static_cast<int32_t>(vertices[tri.c].x);
			ySum += static_cast<int32_t>(vertices[tri.a].y) + static_cast<int32_t>(vertices[tri.b].y) + static_cast<int32_t>(vertices[tri.c].y);
			zSum += static_cast<int32_t>(vertices[tri.a].z) + static_cast<int32_t>(vertices[tri.b].z) + static_cast<int32_t>(vertices[tri.c].z);
			count += 3;
		}

		if (count > 0)
		{
			xSum = xSum / count;
			ySum = ySum / count;
			zSum = zSum / count;

			Serial.print(F("{ "));
			Serial.print(xSum);
			Serial.print(',');
			Serial.print(ySum);
			Serial.print(',');
			Serial.print(zSum);
			Serial.println(F("}"));
		}

		Serial.println(F("};"));
	}
};
#endif