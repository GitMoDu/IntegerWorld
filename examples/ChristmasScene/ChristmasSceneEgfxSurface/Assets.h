#ifndef _CHRISTMAS_SCENE_ASSETS_h
#define _CHRISTMAS_SCENE_ASSETS_h

#include <IntegerWorld.h>

#include "GeneratedAssets.h"

// Scene style definitions.
namespace Assets
{
	using namespace IntegerWorld;

	namespace Styles
	{
		namespace Colors
		{
			static constexpr Rgb8::color_t Red = 0xFF0000;
			static constexpr Rgb8::color_t White = 0xFFFFFF;
			static constexpr Rgb8::color_t Green = 0x00FF00;
			static constexpr Rgb8::color_t Blue = 0x0000FF;
			static constexpr Rgb8::color_t Yellow = 0xFFFF00;
			static constexpr Rgb8::color_t Cyan = 0x00FFFF;
			static constexpr Rgb8::color_t Magenta = 0xFF00FF;
			static constexpr Rgb8::color_t Orange = 0xFFA500;
			static constexpr Rgb8::color_t Purple = 0x800080;
			static constexpr Rgb8::color_t GreenLeaf = 0x85B575;
			static constexpr Rgb8::color_t BrownWood = 0xAB3000;
			static constexpr Rgb8::color_t YellowGold = 0xFFD705;
			static constexpr Rgb8::color_t StarGlow = 0xFFE787;
			static constexpr Rgb8::color_t YellowishBright = 0xFFF1BA;
			static constexpr Rgb8::color_t LightYellow = 0xFFFFE0;
			static constexpr Rgb8::color_t LightGreen = 0x90EE90;
			static constexpr Rgb8::color_t LightBlue = 0xADD8E6;
			static constexpr Rgb8::color_t Silver = 0xC0C0C0;
			static constexpr Rgb8::color_t Pink = 0xFFC0CB;
			static constexpr Rgb8::color_t GlobalLight = 0x5D5C68;
			static constexpr Rgb8::color_t AmbientLight = 0x260C10;

			static constexpr ufraction16_t StringLightsColorSaturation = (static_cast<uint32_t>(UFRACTION16_1X) * 85) / 100;
		}

		namespace Materials
		{
			static constexpr material_t PresentMatte = { 0, UFRACTION8_1X, 0, 0, 0, fraction16_t(-FRACTION8_1X / 2) };
			static constexpr material_t PresentShiny = { UFRACTION8_1X / 16, UFRACTION8_1X, UFRACTION8_1X, UFRACTION8_1X / 2, UFRACTION8_1X / 8, fraction16_t(FRACTION8_1X / 3) };
			static constexpr material_t Star = { UFRACTION8_1X / 8, UFRACTION8_1X / 4, UFRACTION8_1X, UFRACTION8_1X / 3, UFRACTION8_1X, FRACTION8_1X / 2 };
			static constexpr material_t Globe = { UFRACTION8_1X / 8, UFRACTION8_1X, UFRACTION8_1X, UFRACTION8_1X / 9, UFRACTION8_1X, FRACTION8_1X / 4 };
			static constexpr material_t Carpet = { 0, UFRACTION8_1X, 0, 0, 0, 0 };
		}

		namespace Dimensions
		{
			static constexpr uint16_t StringLightsMaxDistance = 1100;
			static constexpr uint16_t GlowLightMaxDistance = 850;
			static constexpr uint16_t TrunkLightMaxDistance = 1200;

			static constexpr int16_t StringLightsGlowRadius = 7;
			static constexpr int16_t StarLightGlowRadius = 23;

			static constexpr int16_t PresentPlacementRadius = 1100;
			static constexpr angle_t PresentPlacementAngleOffset = ANGLE_90 / 4;

			// Offset the floor render Z position to avoid Z-fighting. This only works on a camera above the floor.
			static constexpr int16_t CarpetZOffset = 1000;

			// Offset the lSights render Z position to avoid Z-fighting with the tree.
			static constexpr int16_t StringLightsZOffset = -200;
			static constexpr int16_t GlowLightZOffset = -400;
		}

		namespace Scene
		{
			static constexpr vertex16_t AmbientLightDirection{ -VERTEX16_UNIT, -VERTEX16_UNIT, VERTEX16_UNIT };

			static constexpr uint8_t SceneFoVPercent = 30; // Tall screens.
			//static constexpr uint8_t SceneFoVPercent = 50;

			static constexpr int16_t OrbitRadius = 2900;
			static constexpr int16_t OrbitHeight = 1200;
			static constexpr angle_t OrbitAngle = ANGLE_90 / 22;
			static constexpr uint32_t OrbitPeriod = 30000000 * 3;

			static constexpr uint32_t StringLightsColorPeriod = 7500000;
			static constexpr uint32_t StringLightsIntensityPeriod = 1750000;
		}
	}
}

// Additions to generated assets, based on assets and styles.
namespace Assets
{
	namespace ChristmasLights
	{
		static constexpr uint8_t LightsCount = Shapes::ChristmasLights::Group[Shapes::ChristmasLights::TriangleCount - 1] + 1;
		static constexpr uint8_t VertexCount = LightsCount;

		// Generated using the triangle centers of each light object group.
		static constexpr vertex16_t Vertices[] PROGMEM
		{
			{ -432,976,-1296},
			{ -928,1296,-544},
			{ -1136,1648,272},
			{ -544,1936,944},
			{ 400,2176,896},
			{ 736,2400,160},
			{ 288,2640,-768},
			{ -288,2848,-640},
			{ -528,3200,-112},
			{ -240,3360,352},
			{ 288,832,1272},
			{ 920,1296,488},
			{ 1016,1568,-288},
			{ 656,1936,-832},
			{ -168,2176,-840},
			{ -728,2400,-312},
			{ -808,2608,368},
			{ -240,2848,624},
			{ 488,3040,408},
			{ 344,3296,-376},
			{ -1240,832,480},
			{ -144,1232,1064},
			{ 1128,848,-776},
			{ 32,1504,-872}
		};
	}

	namespace ChristmasTree
	{
		static constexpr Rgb8::color_t Pallete[2] PROGMEM
		{
			Styles::Colors::BrownWood,
			Styles::Colors::GreenLeaf
		};
	}

	namespace ChristmasPresent
	{
		static constexpr auto PresentCount = 5;

		static constexpr auto MaterialCount = 2;

		static constexpr material_t Materials[MaterialCount] PROGMEM
		{
			Styles::Materials::PresentShiny,
			Styles::Materials::PresentMatte
		};

		static constexpr Rgb8::color_t Pallete1[MaterialCount] PROGMEM
		{
			Styles::Colors::Red,
			Styles::Colors::White
		};
		static constexpr Rgb8::color_t Pallete2[MaterialCount] PROGMEM
		{
			Styles::Colors::Green,
			Styles::Colors::Yellow
		};
		static constexpr Rgb8::color_t Pallete3[MaterialCount] PROGMEM
		{
			Styles::Colors::Blue,
			Styles::Colors::Magenta
		};
		static constexpr Rgb8::color_t Pallete4[MaterialCount] PROGMEM
		{
			Styles::Colors::Yellow,
			Styles::Colors::Cyan
		};
		static constexpr Rgb8::color_t Pallete5[MaterialCount] PROGMEM
		{
			Styles::Colors::Orange,
			Styles::Colors::Purple
		};

		static const Rgb8::color_t* GetAlbedos(uint8_t presentIndex)
		{
			switch (presentIndex % PresentCount)
			{
			case 0:
				return Pallete1;
			case 1:
				return Pallete2;
			case 2:
				return Pallete3;
			case 3:
				return Pallete4;
			case 4:
			default:
				break;
			}
			return Pallete5;
		}

		static constexpr rotation_angle_t Rotations[PresentCount]
		{
			{ 0, static_cast<angle_t>(uint32_t(ANGLE_RANGE) * 22 / 360) , 0},
			{ 0, static_cast<angle_t>(uint32_t(ANGLE_RANGE) * 74 / 360) , 0},
			{ 0, static_cast<angle_t>(uint32_t(ANGLE_RANGE) * 128 / 360) , 0},
			{ 0, static_cast<angle_t>(uint32_t(ANGLE_RANGE) * 200 / 360) , 0},
			{ 0, static_cast<angle_t>(uint32_t(ANGLE_RANGE) * 230 / 360) , 0}
		};

		static constexpr scale16_t Scales[PresentCount]
		{
			Scale16::SCALE_1_256X * 180,
			Scale16::SCALE_1_256X * 115,
			Scale16::SCALE_1_256X * 191,
			Scale16::SCALE_1_256X * 161,
			Scale16::SCALE_1_256X * 137,
		};
	}

	namespace ChristmasGlobes
	{
		static constexpr auto GlobeCount = Shapes::ChristmasGlobes::Group[Shapes::ChristmasGlobes::TriangleCount - 1] + 1;
		static constexpr Rgb8::color_t Pallete[GlobeCount] PROGMEM
		{
			Styles::Colors::Red,
			Styles::Colors::LightYellow,
			Styles::Colors::Blue,
			Styles::Colors::Yellow,
			Styles::Colors::Magenta,
			Styles::Colors::Cyan,
			Styles::Colors::Orange,
			Styles::Colors::Green,
			Styles::Colors::Purple,
			Styles::Colors::LightGreen,
			Styles::Colors::YellowGold,
			Styles::Colors::LightBlue,
			Styles::Colors::Silver,
			Styles::Colors::Green,
			Styles::Colors::Pink,
			Styles::Colors::Cyan,
			Styles::Colors::Red,
			Styles::Colors::Yellow,
			Styles::Colors::Magenta,
			Styles::Colors::Orange,
			Styles::Colors::Blue,
			Styles::Colors::Purple,
			Styles::Colors::YellowGold,
			Styles::Colors::Magenta
		};
	}

	namespace LightSources
	{
		// Generated using the triangle centers of each light object group.
		static constexpr vertex16_t Positions[]
		{
			{ 0,640,-2304},
			{ -1920,1648,1408},
			{ 2176,1120,1408}
		};

		constexpr auto LightsCount = sizeof(Positions) / sizeof(vertex16_t);
	}

	namespace ObjectSources
	{
		using StarAlbedoSource = PrimitiveSources::Albedo::Static::FixedSource<Styles::Colors::YellowishBright>;
		using StarTemplateMaterial = PrimitiveSources::Material::Static::TemplateSource<
			Styles::Materials::Star.Emit, Styles::Materials::Star.Rough, Styles::Materials::Star.Shine,
			Styles::Materials::Star.Gloss, Styles::Materials::Star.SpecularTint, Styles::Materials::Star.Fresnel>;

		using TrunkAlbedoSource = PrimitiveSources::Albedo::Static::FixedSource<Styles::Colors::BrownWood>;

		using TreeLeavesAlbedoSource = PrimitiveSources::Albedo::Static::FixedSource<Styles::Colors::GreenLeaf>;

		using CarpetTemplateMaterial = PrimitiveSources::Material::Static::TemplateSource<
			Styles::Materials::Carpet.Emit, Styles::Materials::Carpet.Rough, Styles::Materials::Carpet.Shine,
			Styles::Materials::Carpet.Gloss, Styles::Materials::Carpet.SpecularTint, Styles::Materials::Carpet.Fresnel>;
		using CarpetNormalSourceType = PrimitiveSources::Normal::Static::FixedSource<0, VERTEX16_UNIT, 0>;

		using GlobeTemplateMaterial = PrimitiveSources::Material::Static::TemplateSource<
			Styles::Materials::Globe.Emit, Styles::Materials::Globe.Rough, Styles::Materials::Globe.Shine,
			Styles::Materials::Globe.Gloss, Styles::Materials::Globe.SpecularTint, Styles::Materials::Globe.Fresnel>;

		class StringLightsAlbedoSource
		{
		private:
			const SceneShaders::LightSource::light_source_t* Lights;

		public:
			StringLightsAlbedoSource(const SceneShaders::LightSource::light_source_t* lights = nullptr)
				: Lights(lights)
			{

			}

			void SetLights(const SceneShaders::LightSource::light_source_t* lights)
			{
				Lights = lights;
			}

			static constexpr bool HasAlbedos()
			{
				return true;
			}

			Rgb8::color_t GetAlbedo(const uint16_t groupIndex) const
			{
				return Lights[groupIndex].Color;
			}
		};
	}

	namespace RenderObjects
	{
		class StarTriangleObject
			: public Mesh::TriangleShadeObject<
			Shapes::ChristmasStar::VertexCount, Shapes::ChristmasStar::TriangleCount,
			PrimitiveSources::Vertex::Static::Source,
			PrimitiveSources::Triangle::Static::Source,
			FrustumCullingEnum::PrimitiveCulling,
			FaceCullingEnum::BackfaceCulling,
			ObjectSources::StarAlbedoSource,
			ObjectSources::StarTemplateMaterial>
		{
		private:
			using Base = Mesh::TriangleShadeObject<
				Shapes::ChristmasStar::VertexCount, Shapes::ChristmasStar::TriangleCount,
				PrimitiveSources::Vertex::Static::Source,
				PrimitiveSources::Triangle::Static::Source,
				FrustumCullingEnum::PrimitiveCulling,
				FaceCullingEnum::BackfaceCulling,
				ObjectSources::StarAlbedoSource,
				ObjectSources::StarTemplateMaterial>;

		private:
			PrimitiveSources::Vertex::Static::Source VerticesSource;
			PrimitiveSources::Triangle::Static::Source TrianglesSource;
			ObjectSources::StarAlbedoSource AlbedoSource;
			ObjectSources::StarTemplateMaterial StarMaterial{};

		public:
			StarTriangleObject()
				: Base(VerticesSource, TrianglesSource, AlbedoSource, StarMaterial)
				, VerticesSource(Shapes::ChristmasStar::Vertices)
				, TrianglesSource(Shapes::ChristmasStar::Triangles)
				, AlbedoSource()
			{
			}
		};

		class GlobesTriangleObject
			: public Mesh::TriangleShadeObject<
			Shapes::ChristmasGlobes::VertexCount, Shapes::ChristmasGlobes::TriangleCount,
			PrimitiveSources::Vertex::Static::Source,
			PrimitiveSources::Triangle::Static::Source,
			FrustumCullingEnum::PrimitiveCulling,
			FaceCullingEnum::BackfaceCulling,
			PrimitiveSources::Albedo::Static::PalletedSource,
			ObjectSources::GlobeTemplateMaterial>
		{
		private:
			using Base = Mesh::TriangleShadeObject<
				Shapes::ChristmasGlobes::VertexCount, Shapes::ChristmasGlobes::TriangleCount,
				PrimitiveSources::Vertex::Static::Source,
				PrimitiveSources::Triangle::Static::Source,
				FrustumCullingEnum::PrimitiveCulling,
				FaceCullingEnum::BackfaceCulling,
				PrimitiveSources::Albedo::Static::PalletedSource,
				ObjectSources::GlobeTemplateMaterial>;

		private:
			PrimitiveSources::Vertex::Static::Source VerticesSource;
			PrimitiveSources::Triangle::Static::Source TrianglesSource;
			PrimitiveSources::Albedo::Static::PalletedSource AlbedoSource;
			ObjectSources::GlobeTemplateMaterial GlobesMaterial{};

		public:
			GlobesTriangleObject()
				: Base(VerticesSource, TrianglesSource, AlbedoSource, GlobesMaterial)
				, VerticesSource(Shapes::ChristmasGlobes::Vertices)
				, TrianglesSource(Shapes::ChristmasGlobes::Triangles)
				, AlbedoSource(Assets::ChristmasGlobes::Pallete, Shapes::ChristmasGlobes::Group)
			{
			}
		};

		class TreeLeavesVertexObject
			: public Mesh::VertexShadeObject<
			Shapes::ChristmasLeaves::VertexCount,
			Shapes::ChristmasLeaves::TriangleCount,
			PrimitiveSources::Vertex::Static::Source,
			PrimitiveSources::Triangle::Static::Source,
			FrustumCullingEnum::PrimitiveCulling,
			FaceCullingEnum::BackfaceCulling,
			ObjectSources::TreeLeavesAlbedoSource,
			PrimitiveSources::Material::DiffuseMaterialSource,
			PrimitiveSources::Normal::Static::Source>
		{
		private:
			using Base = Mesh::VertexShadeObject<
				Shapes::ChristmasLeaves::VertexCount,
				Shapes::ChristmasLeaves::TriangleCount,
				PrimitiveSources::Vertex::Static::Source,
				PrimitiveSources::Triangle::Static::Source,
				FrustumCullingEnum::PrimitiveCulling,
				FaceCullingEnum::BackfaceCulling,
				ObjectSources::TreeLeavesAlbedoSource,
				PrimitiveSources::Material::DiffuseMaterialSource,
				PrimitiveSources::Normal::Static::Source>;

			ObjectSources::TreeLeavesAlbedoSource AlbedoSource{};
			PrimitiveSources::Vertex::Static::Source VerticesSource;
			PrimitiveSources::Triangle::Static::Source TrianglesSource;
			PrimitiveSources::Normal::Static::Source NormalSource;

		public:
			TreeLeavesVertexObject()
				: Base(VerticesSource, TrianglesSource,
					AlbedoSource,
					const_cast<PrimitiveSources::Material::DiffuseMaterialSource&>(PrimitiveSources::Material::DiffuseMaterialSourceInstance),
					NormalSource)
				, VerticesSource(Shapes::ChristmasLeaves::Vertices)
				, TrianglesSource(Shapes::ChristmasLeaves::Triangles)
				, NormalSource(Shapes::ChristmasLeaves::VertexNormals)
			{
			}
		};

		class TreeTrunkTriangleObject
			: public Mesh::TriangleShadeObject<
			Shapes::ChristmasTrunk::VertexCount,
			Shapes::ChristmasTrunk::TriangleCount,
			PrimitiveSources::Vertex::Static::Source,
			PrimitiveSources::Triangle::Static::Source,
			FrustumCullingEnum::PrimitiveCulling,
			FaceCullingEnum::BackfaceCulling,
			ObjectSources::TrunkAlbedoSource,
			PrimitiveSources::Material::DiffuseMaterialSource,
			PrimitiveSources::Normal::Static::Source>
		{
		private:
			using Base = Mesh::TriangleShadeObject<
				Shapes::ChristmasTrunk::VertexCount,
				Shapes::ChristmasTrunk::TriangleCount,
				PrimitiveSources::Vertex::Static::Source,
				PrimitiveSources::Triangle::Static::Source,
				FrustumCullingEnum::PrimitiveCulling,
				FaceCullingEnum::BackfaceCulling,
				ObjectSources::TrunkAlbedoSource,
				PrimitiveSources::Material::DiffuseMaterialSource,
				PrimitiveSources::Normal::Static::Source>;
			ObjectSources::TrunkAlbedoSource AlbedoSource{};
			PrimitiveSources::Vertex::Static::Source VerticesSource;
			PrimitiveSources::Triangle::Static::Source TrianglesSource;
			PrimitiveSources::Normal::Static::Source NormalSource;

		public:
			TreeTrunkTriangleObject()
				: Base(VerticesSource, TrianglesSource,
					AlbedoSource,
					const_cast<PrimitiveSources::Material::DiffuseMaterialSource&>(PrimitiveSources::Material::DiffuseMaterialSourceInstance),
					NormalSource)
				, VerticesSource(Shapes::ChristmasTrunk::Vertices)
				, TrianglesSource(Shapes::ChristmasTrunk::Triangles)
				, NormalSource(Shapes::ChristmasTrunk::VertexNormals)
			{
			}
		};

		template<uint8_t palleteIndex>
		class ChristmasPresentTriangleObject
			: public Mesh::TriangleShadeObject<
			Shapes::ChristmasPresent::VertexCount, Shapes::ChristmasPresent::TriangleCount,
			PrimitiveSources::Vertex::Static::Source,
			PrimitiveSources::Triangle::Static::Source,
			FrustumCullingEnum::PrimitiveCulling,
			FaceCullingEnum::BackfaceCulling,
			PrimitiveSources::Albedo::Static::PalletedSource,
			PrimitiveSources::Material::Static::PalletedSource>
		{
		private:
			using Base = Mesh::TriangleShadeObject<
				Shapes::ChristmasPresent::VertexCount, Shapes::ChristmasPresent::TriangleCount,
				PrimitiveSources::Vertex::Static::Source,
				PrimitiveSources::Triangle::Static::Source,
				FrustumCullingEnum::PrimitiveCulling,
				FaceCullingEnum::BackfaceCulling,
				PrimitiveSources::Albedo::Static::PalletedSource,
				PrimitiveSources::Material::Static::PalletedSource>;

		private:
			PrimitiveSources::Vertex::Static::Source VerticesSource;
			PrimitiveSources::Triangle::Static::Source TrianglesSource;
			PrimitiveSources::Albedo::Static::PalletedSource AlbedoSource;
			PrimitiveSources::Material::Static::PalletedSource MaterialSource;

		public:
			ChristmasPresentTriangleObject()
				: Base(VerticesSource, TrianglesSource, AlbedoSource, MaterialSource)
				, VerticesSource(Shapes::ChristmasPresent::Vertices)
				, TrianglesSource(Shapes::ChristmasPresent::Triangles)
				, AlbedoSource(
					Assets::ChristmasPresent::GetAlbedos(palleteIndex), Shapes::ChristmasPresent::Group)
				, MaterialSource(Assets::ChristmasPresent::Materials, Shapes::ChristmasPresent::Group)
			{
			}
		};

		class TreeVertexObject
			: public Mesh::VertexShadeObject<
			Shapes::ChristmasTree::VertexCount,
			Shapes::ChristmasTree::TriangleCount,
			PrimitiveSources::Vertex::Static::Source,
			PrimitiveSources::Triangle::Static::Source,
			FrustumCullingEnum::PrimitiveCulling,
			FaceCullingEnum::BackfaceCulling,
			PrimitiveSources::Albedo::Dynamic::SingleSource
			>
		{
		private:
			using Base = Mesh::VertexShadeObject<
				Shapes::ChristmasTree::VertexCount,
				Shapes::ChristmasTree::TriangleCount,
				PrimitiveSources::Vertex::Static::Source,
				PrimitiveSources::Triangle::Static::Source,
				FrustumCullingEnum::PrimitiveCulling,
				FaceCullingEnum::BackfaceCulling,
				PrimitiveSources::Albedo::Dynamic::SingleSource>;

			PrimitiveSources::Albedo::Dynamic::SingleSource AlbedoSource{};
			PrimitiveSources::Vertex::Static::Source VerticesSource;
			PrimitiveSources::Triangle::Static::Source TrianglesSource;

		public:
			TreeVertexObject()
				: Base(VerticesSource,
					TrianglesSource,
					AlbedoSource)
				, VerticesSource(Shapes::ChristmasTree::Vertices)
				, TrianglesSource(Shapes::ChristmasTree::Triangles)
			{
			}

			void SetAlbedo(const Rgb8::color_t color)
			{
				AlbedoSource.Albedo = color;
			}
		};

		class StringLightsPointCloudObject
			: public PointCloud::ShadeObject<
			Assets::ChristmasLights::VertexCount,
			PrimitiveSources::Vertex::Static::Source,
			FrustumCullingEnum::PrimitiveCulling,
			FaceCullingEnum::BackfaceCulling,
			ObjectSources::StringLightsAlbedoSource,
			PrimitiveSources::Material::DiffuseMaterialSource>
		{
		private:
			using Base = PointCloud::ShadeObject<
				Assets::ChristmasLights::VertexCount,
				PrimitiveSources::Vertex::Static::Source,
				FrustumCullingEnum::PrimitiveCulling,
				FaceCullingEnum::BackfaceCulling,
				ObjectSources::StringLightsAlbedoSource,
				PrimitiveSources::Material::DiffuseMaterialSource>;

		private:
			PrimitiveSources::Vertex::Static::Source VertexSource;
			ObjectSources::StringLightsAlbedoSource AlbedoSource;

		public:
			StringLightsPointCloudObject(const SceneShaders::LightSource::light_source_t* lights = nullptr)
				: Base(VertexSource,
					AlbedoSource,
					const_cast<PrimitiveSources::Material::DiffuseMaterialSource&>(PrimitiveSources::Material::DiffuseMaterialSourceInstance))
				, VertexSource(Assets::ChristmasLights::Vertices)
				, AlbedoSource(lights)
			{
			}

			void SetLights(const SceneShaders::LightSource::light_source_t* lights)
			{
				AlbedoSource.SetLights(lights);
			}

			virtual void FragmentCollect(FragmentCollector& fragmentCollector) override
			{
				for (uint_fast16_t i = 0; i < VertexCount; i++)
				{
					if (Primitives[i] >= 0)
					{
						Primitives[i] += Styles::Dimensions::StringLightsZOffset;
					}
				}
				Base::FragmentCollect(fragmentCollector);
			}
		};

		class StarGlowPointCloudObject : public RenderObjects::TemplateTransformObject<1, 1>
		{
		private:
			using Base = RenderObjects::TemplateTransformObject<1, 1>;

		public:
			IFragmentShader<point_cloud_fragment_t>* FragmentShader = nullptr;

			const Rgb8::color_t* AlbedoSource = nullptr;

		private:
			point_cloud_fragment_t Fragment{};
			vertex16_t Position{};

		public:
			StarGlowPointCloudObject() : Base()
			{
			}

			void SetPosition(const vertex16_t& positionSource)
			{
				Position = positionSource;
			}

			virtual void ObjectShade(const frustum_t& frustum)
			{
				Base::ObjectShade(frustum);

				const int16_t zFlag = -VERTEX16_UNIT * !frustum.IsPointInside(WorldPosition);

				for (uint_fast16_t i = 0; i < VertexCount; i++)
				{
					auto vertex = Position;
					Vertices[i].x = vertex.x;
					Vertices[i].y = vertex.y;
					Vertices[i].z = vertex.z;
					Primitives[i] = zFlag;
				}
			}

			virtual bool WorldShade(const frustum_t& frustum, const uint16_t primitiveIndex)
			{
				if (primitiveIndex >= VertexCount)
					return true;

				if (Primitives[primitiveIndex] < 0)
					return false;

				if (!frustum.IsPointInside(Vertices[primitiveIndex]))
				{
					Primitives[primitiveIndex] = -VERTEX16_UNIT;
					return false;
				}

				return false;
			}

			virtual bool ScreenShade(const uint16_t primitiveIndex)
			{
				if (primitiveIndex >= VertexCount)
					return true;

				if (Primitives[primitiveIndex] >= 0)
				{
					Primitives[primitiveIndex] = Vertices[primitiveIndex].z + Styles::Dimensions::GlowLightZOffset;
				}

				return false;
			}

			virtual void FragmentCollect(FragmentCollector& fragmentCollector)
			{
				if (Base::Primitives[0] >= 0)
				{
					fragmentCollector.AddFragment(0, Base::Primitives[0]);
				}
			}

			virtual void FragmentShade(WindowRasterizer& rasterizer, const uint16_t primitiveIndex)
			{
				if (FragmentShader == nullptr || AlbedoSource == nullptr)
					return;

				Fragment.index = primitiveIndex;
				Fragment.x = Vertices[primitiveIndex].x;
				Fragment.y = Vertices[primitiveIndex].y;
				Fragment.z = Primitives[primitiveIndex];
				{
					Fragment.red = Rgb8::Red(*AlbedoSource);
					Fragment.green = Rgb8::Green(*AlbedoSource);
					Fragment.blue = Rgb8::Blue(*AlbedoSource);
				}

				FragmentShader->FragmentShade(rasterizer, Fragment);
			}
		};

		class CarpetTextured32x32TriangleObject
			: public Mesh::TriangleShadeObject<
			Shapes::ChristmasCarpet::VertexCount, Shapes::ChristmasCarpet::TriangleCount,
			PrimitiveSources::Vertex::Static::Source,
			PrimitiveSources::Triangle::Static::Source,
			FrustumCullingEnum::NoCulling,
			FaceCullingEnum::NoCulling,
			PrimitiveSources::Albedo::Static::FullSource,
			ObjectSources::CarpetTemplateMaterial,
			ObjectSources::CarpetNormalSourceType,
			PrimitiveSources::Uv::Static::Source>
		{
		public:
			using TextureSourceType = PrimitiveSources::Texture::Static::Source<TextureSize32x32>;

		private:
			using Base = Mesh::TriangleShadeObject <
				Shapes::ChristmasCarpet::VertexCount, Shapes::ChristmasCarpet::TriangleCount,
				PrimitiveSources::Vertex::Static::Source,
				PrimitiveSources::Triangle::Static::Source,
				FrustumCullingEnum::NoCulling,
				FaceCullingEnum::NoCulling,
				PrimitiveSources::Albedo::Static::FullSource,
				ObjectSources::CarpetTemplateMaterial,
				ObjectSources::CarpetNormalSourceType,
				PrimitiveSources::Uv::Static::Source>;

		private:
			ObjectSources::CarpetNormalSourceType NormalSource{};
			PrimitiveSources::Vertex::Static::Source VerticesSource;
			PrimitiveSources::Triangle::Static::Source TrianglesSource;
			ObjectSources::CarpetTemplateMaterial MaterialSource{};
			PrimitiveSources::Uv::Static::Source UvsSource;

		public:
			CarpetTextured32x32TriangleObject()
				: VerticesSource(Shapes::ChristmasCarpet::Vertices)
				, TrianglesSource(Shapes::ChristmasCarpet::Triangles)
				, UvsSource(Shapes::ChristmasCarpet::UVs32x32)
				, Base(VerticesSource, TrianglesSource,
					const_cast<PrimitiveSources::Albedo::Static::FullSource&>(PrimitiveSources::Albedo::FullAlbedoSourceInstance),
					MaterialSource,
					NormalSource,
					UvsSource)
			{
			}

			virtual void FragmentCollect(FragmentCollector& fragmentCollector) override
			{
				for (uint_fast16_t i = 0; i < TriangleCount; i++)
				{
					if (Base::Primitives[i] >= 0)
					{
						fragmentCollector.AddFragment(i, Base::Primitives[i] + Styles::Dimensions::CarpetZOffset);
					}
				}
			}
		};
	}


	namespace FragmentShaders
	{
		template<uint8_t GlowRadius>
		struct TemplateGlowFragmentShader : IFragmentShader<point_cloud_fragment_t>
		{
			void FragmentShade(WindowRasterizer& rasterizer, const point_cloud_fragment_t& fragment) final
			{
				for (int_fast8_t y = 0; y < GlowRadius; y++)
				{
					for (int_fast8_t x = 0; x < GlowRadius; x++)
					{
						const uint8_t distance = SquareRoot16((uint_fast16_t(x) * x) + (uint_fast16_t(y) * y));
						if (distance > GlowRadius)
							continue;

						const ufraction8_t distanceFraction = UFraction8::GetScalar<uint8_t>(distance, GlowRadius);
						ufraction8_t proximityFraction = (UFRACTION8_1X - distanceFraction) >> 1;

						proximityFraction = (static_cast<uint16_t>(proximityFraction) * proximityFraction) >> 6;

						const Rgb8::color_t blendedGlowColor = Rgb8::Color(
							Fraction(proximityFraction, static_cast<uint16_t>(fragment.red)),
							Fraction(proximityFraction, static_cast<uint16_t>(fragment.green)),
							Fraction(proximityFraction, static_cast<uint16_t>(fragment.blue)));

						rasterizer.BlendPixel<pixel_blend_mode_t::Add>(blendedGlowColor, fragment.x + x, fragment.y + y);
						rasterizer.BlendPixel<pixel_blend_mode_t::Add>(blendedGlowColor, fragment.x - x, fragment.y + y);
						rasterizer.BlendPixel<pixel_blend_mode_t::Add>(blendedGlowColor, fragment.x + x, fragment.y - y);
						rasterizer.BlendPixel<pixel_blend_mode_t::Add>(blendedGlowColor, fragment.x - x, fragment.y - y);
					}
				}
			}
		};

		using CarpetTriangleUnlitShaderType = Mesh::PixelShaders::TextureUnlit<
			typename RenderObjects::CarpetTextured32x32TriangleObject::TextureSourceType,
			PrimitiveShaders::TrianglePerspectiveCorrectSampler,
			PrimitiveShaders::UvInterpolationModeEnum::Fast>;

		class CarpetTexturedTriangleUnlitShader
			: public Mesh::FragmentShaders::TriangleShade::TemplateTextureShader<
			typename RenderObjects::CarpetTextured32x32TriangleObject::TextureSourceType,
			CarpetTriangleUnlitShaderType>
		{
		private:
			RenderObjects::CarpetTextured32x32TriangleObject::TextureSourceType TextureSource;
		public:
			CarpetTexturedTriangleUnlitShader()
				: Mesh::FragmentShaders::TriangleShade::TemplateTextureShader<
				typename RenderObjects::CarpetTextured32x32TriangleObject::TextureSourceType,
				CarpetTriangleUnlitShaderType>(TextureSource)
				, TextureSource(Textures::Carpet32x32)
			{
			}
		};
	}
}
#endif