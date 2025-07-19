#ifndef _DEMO_SCENE_ASSETS_h
#define _DEMO_SCENE_ASSETS_h

#include <IntegerWorld.h>
#include <ArduinoGraphicsDrawer.h>

namespace Assets
{
	using namespace Egfx;
	using namespace IntegerWorld;

	namespace Shapes
	{
		namespace Cube
		{
			static constexpr color_fraction16_t Pallete[TriangleCount]
			{
				ColorFraction::RgbToColorFraction(uint32_t(0xFF0000)),
				ColorFraction::RgbToColorFraction(uint32_t(0x00FF00)),
				ColorFraction::RgbToColorFraction(uint32_t(0x0000FF)),
				ColorFraction::RgbToColorFraction(uint32_t(0xFFFF00)),
				ColorFraction::RgbToColorFraction(uint32_t(0x00FFFF)),
				ColorFraction::RgbToColorFraction(uint32_t(0xFF00FF))
			};
			constexpr uint8_t PalleteSize = sizeof(Pallete) / sizeof(Pallete[0]);
		}

		namespace Star
		{
			static constexpr int16_t scaleFactor = SHAPE_UNIT / 128;

			static constexpr vertex16_t Vertices[] PROGMEM
			{
				{ (int16_t)((int32_t)32 * scaleFactor), (int16_t)((int32_t)-100 * scaleFactor), (int16_t)((int32_t)-27 * scaleFactor) },
				{ (int16_t)((int32_t)-17 * scaleFactor), (int16_t)((int32_t)-50 * scaleFactor), (int16_t)((int32_t)-27 * scaleFactor) },
				{ (int16_t)((int32_t)42 * scaleFactor), (int16_t)((int32_t)-31 * scaleFactor), (int16_t)((int32_t)-27 * scaleFactor) },
				{ (int16_t)((int32_t)-53 * scaleFactor), (int16_t)((int32_t)0 * scaleFactor), (int16_t)((int32_t)-27 * scaleFactor) },
				{ (int16_t)((int32_t)-17 * scaleFactor), (int16_t)((int32_t)49 * scaleFactor), (int16_t)((int32_t)-27 * scaleFactor) },
				{ (int16_t)((int32_t)-85 * scaleFactor), (int16_t)((int32_t)-62 * scaleFactor), (int16_t)((int32_t)-27 * scaleFactor) },
				{ (int16_t)((int32_t)42 * scaleFactor), (int16_t)((int32_t)30 * scaleFactor), (int16_t)((int32_t)-27 * scaleFactor) },
				{ (int16_t)((int32_t)-85 * scaleFactor), (int16_t)((int32_t)61 * scaleFactor), (int16_t)((int32_t)-27 * scaleFactor) },
				{ (int16_t)((int32_t)32 * scaleFactor), (int16_t)((int32_t)99 * scaleFactor), (int16_t)((int32_t)-27 * scaleFactor) },
				{ (int16_t)((int32_t)104 * scaleFactor), (int16_t)((int32_t)0 * scaleFactor), (int16_t)((int32_t)-27 * scaleFactor) },
				{ (int16_t)((int32_t)32 * scaleFactor), (int16_t)((int32_t)-100 * scaleFactor), (int16_t)((int32_t)26 * scaleFactor) },
				{ (int16_t)((int32_t)42 * scaleFactor), (int16_t)((int32_t)-31 * scaleFactor), (int16_t)((int32_t)26 * scaleFactor) },
				{ (int16_t)((int32_t)-17 * scaleFactor), (int16_t)((int32_t)-50 * scaleFactor), (int16_t)((int32_t)26 * scaleFactor) },
				{ (int16_t)((int32_t)42 * scaleFactor), (int16_t)((int32_t)30 * scaleFactor), (int16_t)((int32_t)26 * scaleFactor) },
				{ (int16_t)((int32_t)104 * scaleFactor), (int16_t)((int32_t)0 * scaleFactor), (int16_t)((int32_t)26 * scaleFactor) },
				{ (int16_t)((int32_t)-17 * scaleFactor), (int16_t)((int32_t)49 * scaleFactor), (int16_t)((int32_t)26 * scaleFactor) },
				{ (int16_t)((int32_t)32 * scaleFactor), (int16_t)((int32_t)99 * scaleFactor), (int16_t)((int32_t)26 * scaleFactor) },
				{ (int16_t)((int32_t)-53 * scaleFactor), (int16_t)((int32_t)0 * scaleFactor), (int16_t)((int32_t)26 * scaleFactor) },
				{ (int16_t)((int32_t)-85 * scaleFactor), (int16_t)((int32_t)61 * scaleFactor), (int16_t)((int32_t)26 * scaleFactor) },
				{ (int16_t)((int32_t)-85 * scaleFactor), (int16_t)((int32_t)-62 * scaleFactor), (int16_t)((int32_t)26 * scaleFactor) }
			};
			constexpr uint16_t VertexCount = sizeof(Vertices) / sizeof(vertex16_t);

			static constexpr triangle_face_t Triangles[] PROGMEM
			{
				{ 0, 1, 2 },
				{ 3, 4, 1 },
				{ 5, 3, 1 },
				{ 2, 1, 6 },
				{ 7, 4, 3 },
				{ 6, 1, 4 },
				{ 8, 6, 4 },
				{ 9, 2, 6 },
				{ 10, 11, 12 },
				{ 11, 13, 12 },
				{ 14, 13, 11 },
				{ 13, 15, 12 },
				{ 16, 15, 13 },
				{ 17, 12, 15 },
				{ 18, 17, 15 },
				{ 19, 12, 17 },
				{ 8, 16, 13 },
				{ 8, 13, 6 },
				{ 14, 9, 6 },
				{ 14, 6, 13 },
				{ 4, 7, 18 },
				{ 4, 18, 15 },
				{ 15, 16, 4 },
				{ 8, 4, 16 },
				{ 3, 5, 19 },
				{ 3, 19, 17 },
				{ 1, 12, 19 },
				{ 1, 19, 5 },
				{ 14, 11, 2 },
				{ 14, 2, 9 },
				{ 2, 11, 10 },
				{ 2, 10, 0 },
				{ 17, 18, 3 },
				{ 7, 3, 18 },
				{ 1, 0, 10 },
				{ 1, 10, 12 },
			};
			constexpr uint16_t TriangleCount = sizeof(Triangles) / sizeof(triangle_face_t);

			static constexpr vertex16_t Normals[TriangleCount]{
				// Normals for each triangle, calculated and normalized, scaled to VERTEX16_UNIT
				{   0, -819, -574 }, // { 0, 1, 2 }
				{ -819, -574,   0 }, // { 3, 4, 1 }
				{ -574, -819,   0 }, // { 5, 3, 1 }
				{   0, -574, -819 }, // { 2, 1, 6 }
				{ -819,   0, -574 }, // { 7, 4, 3 }
				{   0, -819, -574 }, // { 6, 1, 4 }
				{   0, -574, -819 }, // { 8, 6, 4 }
				{  819,   0, -574 }, // { 9, 2, 6 }
				{   0,  819, -574 }, // { 10, 11, 12 }
				{  819, -574,   0 }, // { 11, 13, 12 }
				{  574,  819,   0 }, // { 14, 13, 11 }
				{   0,  574,  819 }, // { 13, 15, 12 }
				{  819,   0,  574 }, // { 16, 15, 13 }
				{   0,  574,  819 }, // { 17, 12, 15 }
				{   0,  819,  574 }, // { 18, 17, 15 }
				{ -819,   0,  574 }, // { 19, 12, 17 }
				{   0,  574,  819 }, // { 8, 16, 13 }
				{   0,  574,  819 }, // { 8, 13, 6 }
				{  574,  819,   0 }, // { 14, 9, 6 }
				{  574,  819,   0 }, // { 14, 6, 13 }
				{ -574,  819,   0 }, // { 4, 7, 18 }
				{ -574,  819,   0 }, // { 4, 18, 15 }
				{   0,  574,  819 }, // { 15, 16, 4 }
				{   0,  574,  819 }, // { 8, 4, 16 }
				{ -819,   0,  574 }, // { 3, 5, 19 }
				{ -819,   0,  574 }, // { 3, 19, 17 }
				{   0, -574,  819 }, // { 1, 12, 19 }
				{   0, -574,  819 }, // { 1, 19, 5 }
				{  574, -819,   0 }, // { 14, 11, 2 }
				{  574, -819,   0 }, // { 14, 2, 9 }
				{   0, -819,  574 }, // { 2, 11, 10 }
				{   0, -819,  574 }, // { 2, 10, 0 }
				{ -574, -819,   0 }, // { 17, 18, 3 }
				{ -574, -819,   0 }, // { 7, 3, 18 }
				{   0, -819,  574 }, // { 1, 0, 10 }
				{   0, -819,  574 }, // { 1, 10, 12 }
			};
		}

		namespace Grid8x8
		{
			static constexpr vertex16_t Vertices[] =
			{
				{-SHAPE_UNIT / 28, -SHAPE_UNIT / 28, 0},
				{ SHAPE_UNIT / 28, -SHAPE_UNIT / 28, 0},
				{ SHAPE_UNIT * 3 / 28, -SHAPE_UNIT / 28, 0},
				{ SHAPE_UNIT * 5 / 28, -SHAPE_UNIT / 28, 0},
				{ SHAPE_UNIT * 7 / 28, -SHAPE_UNIT / 28, 0},
				{ SHAPE_UNIT * 9 / 28, -SHAPE_UNIT / 28, 0},
				{ SHAPE_UNIT * 11 / 28, -SHAPE_UNIT / 28, 0},
				{ SHAPE_UNIT * 13 / 28, -SHAPE_UNIT / 28, 0},
				{-SHAPE_UNIT / 28,  SHAPE_UNIT / 28, 0},
				{ SHAPE_UNIT / 28,  SHAPE_UNIT / 28, 0},
				{ SHAPE_UNIT * 3 / 28,  SHAPE_UNIT / 28, 0},
				{ SHAPE_UNIT * 5 / 28,  SHAPE_UNIT / 28, 0},
				{ SHAPE_UNIT * 7 / 28,  SHAPE_UNIT / 28, 0},
				{ SHAPE_UNIT * 9 / 28,  SHAPE_UNIT / 28, 0},
				{ SHAPE_UNIT * 11 / 28,  SHAPE_UNIT / 28, 0},
				{ SHAPE_UNIT * 13 / 28,  SHAPE_UNIT / 28, 0},
				{-SHAPE_UNIT / 28,  SHAPE_UNIT * 3 / 28, 0},
				{ SHAPE_UNIT / 28,  SHAPE_UNIT * 3 / 28, 0},
				{ SHAPE_UNIT * 3 / 28,  SHAPE_UNIT * 3 / 28, 0},
				{ SHAPE_UNIT * 5 / 28,  SHAPE_UNIT * 3 / 28, 0},
				{ SHAPE_UNIT * 7 / 28,  SHAPE_UNIT * 3 / 28, 0},
				{ SHAPE_UNIT * 9 / 28,  SHAPE_UNIT * 3 / 28, 0},
				{ SHAPE_UNIT * 11 / 28,  SHAPE_UNIT * 3 / 28, 0},
				{ SHAPE_UNIT * 13 / 28,  SHAPE_UNIT * 3 / 28, 0},
				{-SHAPE_UNIT / 28,  SHAPE_UNIT * 5 / 28, 0},
				{ SHAPE_UNIT / 28,  SHAPE_UNIT * 5 / 28, 0},
				{ SHAPE_UNIT * 3 / 28,  SHAPE_UNIT * 5 / 28, 0},
				{ SHAPE_UNIT * 5 / 28,  SHAPE_UNIT * 5 / 28, 0},
				{ SHAPE_UNIT * 7 / 28,  SHAPE_UNIT * 5 / 28, 0},
				{ SHAPE_UNIT * 9 / 28,  SHAPE_UNIT * 5 / 28, 0},
				{ SHAPE_UNIT * 11 / 28,  SHAPE_UNIT * 5 / 28, 0},
				{ SHAPE_UNIT * 13 / 28,  SHAPE_UNIT * 5 / 28, 0},
				{-SHAPE_UNIT / 28,  SHAPE_UNIT * 7 / 28, 0},
				{ SHAPE_UNIT / 28,  SHAPE_UNIT * 7 / 28, 0},
				{ SHAPE_UNIT * 3 / 28,  SHAPE_UNIT * 7 / 28, 0},
				{ SHAPE_UNIT * 5 / 28,  SHAPE_UNIT * 7 / 28, 0},
				{ SHAPE_UNIT * 7 / 28,  SHAPE_UNIT * 7 / 28, 0},
				{ SHAPE_UNIT * 9 / 28,  SHAPE_UNIT * 7 / 28, 0},
				{ SHAPE_UNIT * 11 / 28,  SHAPE_UNIT * 7 / 28, 0},
				{ SHAPE_UNIT * 13 / 28,  SHAPE_UNIT * 7 / 28, 0},
				{-SHAPE_UNIT / 28,  SHAPE_UNIT * 9 / 28, 0},
				{ SHAPE_UNIT / 28,  SHAPE_UNIT * 9 / 28, 0},
				{ SHAPE_UNIT * 3 / 28,  SHAPE_UNIT * 9 / 28, 0},
				{ SHAPE_UNIT * 5 / 28,  SHAPE_UNIT * 9 / 28, 0},
				{ SHAPE_UNIT * 7 / 28,  SHAPE_UNIT * 9 / 28, 0},
				{ SHAPE_UNIT * 9 / 28,  SHAPE_UNIT * 9 / 28, 0},
				{ SHAPE_UNIT * 11 / 28,  SHAPE_UNIT * 9 / 28, 0},
				{ SHAPE_UNIT * 13 / 28,  SHAPE_UNIT * 9 / 28, 0},
				{-SHAPE_UNIT / 28,  SHAPE_UNIT * 11 / 28, 0},
				{ SHAPE_UNIT / 28,  SHAPE_UNIT * 11 / 28, 0},
				{ SHAPE_UNIT * 3 / 28,  SHAPE_UNIT * 11 / 28, 0},
				{ SHAPE_UNIT * 5 / 28,  SHAPE_UNIT * 11 / 28, 0},
				{ SHAPE_UNIT * 7 / 28,  SHAPE_UNIT * 11 / 28, 0},
				{ SHAPE_UNIT * 9 / 28,  SHAPE_UNIT * 11 / 28, 0},
				{ SHAPE_UNIT * 11 / 28,  SHAPE_UNIT * 11 / 28, 0},
				{ SHAPE_UNIT * 13 / 28,  SHAPE_UNIT * 11 / 28, 0},
				{-SHAPE_UNIT / 28,  SHAPE_UNIT * 13 / 28, 0},
				{ SHAPE_UNIT / 28,  SHAPE_UNIT * 13 / 28, 0},
				{ SHAPE_UNIT * 3 / 28,  SHAPE_UNIT * 13 / 28, 0},
				{ SHAPE_UNIT * 5 / 28,  SHAPE_UNIT * 13 / 28, 0},
				{ SHAPE_UNIT * 7 / 28,  SHAPE_UNIT * 13 / 28, 0},
				{ SHAPE_UNIT * 9 / 28,  SHAPE_UNIT * 13 / 28, 0},
				{ SHAPE_UNIT * 11 / 28,  SHAPE_UNIT * 13 / 28, 0},
				{ SHAPE_UNIT * 13 / 28,  SHAPE_UNIT * 13 / 28, 0}
			};

			constexpr uint8_t VertexCount = sizeof(Vertices) / sizeof(vertex16_t);
		}
	}

	namespace Shaders
	{
		struct FloorFragmentShader : IFragmentShader<point_fragment_t>
		{
		private:
			world_position_shade_t Shade{};
			color_fraction16_t FragmentColor{};

		public:
			int16_t Radius = 1;

		public:
			FloorFragmentShader() {}

			virtual void FragmentShade(WindowRasterizer& rasterizer, const point_fragment_t& fragment, ISceneShader* sceneShader) final
			{
				FragmentColor = fragment.color;
				Shade.positionWorld = fragment.world;
				sceneShader->Shade(FragmentColor, fragment.material, Shade);
				rasterizer.DrawLine(FragmentColor, fragment.screen.x - Radius, fragment.screen.y, fragment.screen.x + Radius, fragment.screen.y);
				rasterizer.DrawLine(FragmentColor, fragment.screen.x, fragment.screen.y - Radius, fragment.screen.x, fragment.screen.y + Radius);
			}

			void FragmentShade(WindowRasterizer& rasterizer, const point_fragment_t& fragment) final
			{
				FragmentColor = fragment.color;
				rasterizer.DrawLine(FragmentColor, fragment.screen.x - Radius, fragment.screen.y, fragment.screen.x + Radius, fragment.screen.y);
				rasterizer.DrawLine(FragmentColor, fragment.screen.x, fragment.screen.y - Radius, fragment.screen.x, fragment.screen.y + Radius);
			}
		};

		class LightSourceFragmentShader : public IFragmentShader<point_fragment_t>
		{
		private:
			class RaysShaderFunctor
			{
			private:
				uint16_t rng = 42;

			public:
				color_fraction16_t outerColor{};
				uint32_t radiusPower = 0;
				int16_t centerX = 0;
				int16_t centerY = 0;

				RaysShaderFunctor() {}

				bool operator()(color_fraction16_t& color, const int16_t x, int16_t y)
				{
					// Calculate point properties.
					const int16_t xShifted = x - centerX;
					const int16_t yShifted = y - centerY;
					const uint32_t distancePower = ((int32_t(xShifted) * xShifted) + (int32_t(yShifted) * yShifted));

					// Rng update every pixel, regardles of draw to keep entropy high.
					rng ^= rng << 7;
					rng ^= rng >> 9;
					rng ^= rng << 8;
					const uint8_t seed = rng & UINT8_MAX;

					if (distancePower < radiusPower)
					{
						// Approximate a distance fraction.
						const uint8_t distance = Fraction::GetUFraction8(distancePower, radiusPower);
						const uint8_t proximity = UINT8_MAX - 1 - (distance << 1);

						// Rng determines pixel visibility, to apply a dithered transparency according to distance from center.
						if (seed < proximity)
						{
							// Convert distance to fraction to be used in color interpolation.
							const ufraction8_t distanceFraction = uint16_t(distance) >> 1;
							const ufraction8_t proximityFraction = UFRACTION8_1X - distanceFraction;

							// Interpolate between outer and inner color.
							color.r = ufraction16_t(Fraction::Scale(distanceFraction, outerColor.r) + Fraction::Scale(proximityFraction, ColorFraction::COLOR_WHITE.r));
							color.g = ufraction16_t(Fraction::Scale(distanceFraction, outerColor.g) + Fraction::Scale(proximityFraction, ColorFraction::COLOR_WHITE.g));
							color.b = ufraction16_t(Fraction::Scale(distanceFraction, outerColor.b) + Fraction::Scale(proximityFraction, ColorFraction::COLOR_WHITE.b));

							return true;
						}
					}
					return false;
				}
			} RaysShader{};

		public:
			uint16_t MaxSize = 8;

		public:
			LightSourceFragmentShader() {}

			void FragmentShade(WindowRasterizer& rasterizer, const point_fragment_t& fragment)
			{
			}

			void FragmentShade(WindowRasterizer& rasterizer, const point_fragment_t& fragment, ISceneShader* sceneShader) final
			{
				// Calculate fragment properties.
				const ufraction16_t proximity = AbstractPixelShader::GetZFraction(fragment.screen.z, 1, VERTEX16_RANGE / 2);
				const uint16_t radius = MaxValue(uint16_t(Fraction::Scale(proximity, MaxSize)), uint16_t(2));
				const uint32_t radiusPower = (((uint32_t)radius * radius));

				if (sceneShader != nullptr)
				{
					// Pass along the fragment properties.
					RaysShader.centerX = fragment.screen.x;
					RaysShader.centerY = fragment.screen.y;
					RaysShader.radiusPower = radiusPower;

					// Apply shading to light color. This serves as an intensity dial with emissive the property.
					RaysShader.outerColor = fragment.color;
					sceneShader->Shade(RaysShader.outerColor, fragment.material);

					if (RaysShader.outerColor.r > 0 || RaysShader.outerColor.g > 0 || RaysShader.outerColor.b > 0)
					{
						// Draw light source fragment with RaysShader.
						rasterizer.RasterRectangle(fragment.screen.x - radius, fragment.screen.y - radius, fragment.screen.x + radius, fragment.screen.y + radius, RaysShader);
					}
				}
				else
				{
					rasterizer.DrawRectangle(fragment.color, fragment.screen.x - radius, fragment.screen.y - radius, fragment.screen.x + radius, fragment.screen.y + radius);
				}
			}
		};
	}

	namespace Objects
	{
		using namespace Shapes;

		struct StarMeshObject : public MeshWorldSingleColorSingleMaterialObject<Star::VertexCount, Star::TriangleCount>
		{
			StarMeshObject() : MeshWorldSingleColorSingleMaterialObject<Star::VertexCount, Star::TriangleCount>(
				Star::Vertices,
				Star::Triangles) {
			}
		};

		struct CubeMeshObject : public MeshWorldObject<Cube::VertexCount, Cube::TriangleCount>
		{
			material_t Material{ 0, UFRACTION8_1X, 0, 0 };

			CubeMeshObject() : MeshWorldObject<Cube::VertexCount, Cube::TriangleCount>(
				Cube::Vertices,
				Cube::Triangles,
				Cube::Normals) {
			}

		protected:
			virtual void GetFragment(triangle_fragment_t& fragment, const uint16_t index)
			{
				fragment.color = Cube::Pallete[(index / 2) % Cube::PalleteSize];
				fragment.material = Material;
			}
		};

		struct CubeEdgeObject : public EdgeObject<Cube::VertexCount, Cube::EdgeCount>
		{
			CubeEdgeObject()
				: EdgeObject<Cube::VertexCount, Cube::EdgeCount>(
					Cube::Vertices,
					Cube::Edges) {
			}
		};

		struct OctahedronMeshObject : public MeshWorldSingleColorSingleMaterialObject<Octahedron::VertexCount, Octahedron::TriangleCount>
		{
			OctahedronMeshObject() : MeshWorldSingleColorSingleMaterialObject<Octahedron::VertexCount, Octahedron::TriangleCount>(
				Octahedron::Vertices,
				Octahedron::Triangles,
				Octahedron::Normals) {
			}
		};

		struct OctahedronEdgeObject : public EdgeObject<Octahedron::VertexCount, Octahedron::EdgeCount>
		{
			OctahedronEdgeObject()
				: EdgeObject<Octahedron::VertexCount, Octahedron::EdgeCount>(
					Octahedron::Vertices,
					Octahedron::Edges) {
			}
		};

		struct IcosahedronMeshObject : public MeshWorldSingleColorSingleMaterialObject<Icosahedron::VertexCount, Icosahedron::TriangleCount>
		{
			IcosahedronMeshObject() : MeshWorldSingleColorSingleMaterialObject<Icosahedron::VertexCount, Icosahedron::TriangleCount>(
				Icosahedron::Vertices,
				Icosahedron::Triangles,
				Icosahedron::Normals) {
			}
		};

		struct IcosahedronEdgeObject : public EdgeObject<Icosahedron::VertexCount, Icosahedron::EdgeCount>
		{
			IcosahedronEdgeObject() : EdgeObject<Icosahedron::VertexCount, Icosahedron::EdgeCount>(
				Icosahedron::Vertices,
				Icosahedron::Edges) {
			}
		};

		struct SphereMeshObject : public MeshWorldSingleColorSingleMaterialObject<Sphere::VertexCount, Sphere::TriangleCount>
		{
			SphereMeshObject() : MeshWorldSingleColorSingleMaterialObject<Sphere::VertexCount, Sphere::TriangleCount>(
				Sphere::Vertices,
				Sphere::Triangles,
				Sphere::Normals
			) {
			}
		};

		class SpherePointCloudObject : public LitPointCloudObject<Sphere::VertexCount>
		{
		public:
			SpherePointCloudObject() : LitPointCloudObject<Sphere::VertexCount>(Sphere::Vertices) {}

		protected:
			virtual void GetFragment(point_fragment_t& fragment, const uint16_t index)
			{
				fragment.color.r = UFRACTION16_1X;
				fragment.color.g = UFRACTION16_1X;
				fragment.color.b = UFRACTION16_1X;
				fragment.material.Diffuse = UFRACTION8_1X;
				fragment.material.Emissive = 0;
				fragment.material.Specular = 0;
				fragment.material.Metallic = 0;
			}
		};

		struct FloorPointCloudObject : public FlatPointCloudObject<Grid8x8::VertexCount>
		{
			FloorPointCloudObject() : FlatPointCloudObject<Grid8x8::VertexCount>(Grid8x8::Vertices) {}
		};

		template<typename LightSourceType, typename fragment_t = point_fragment_t>
		struct ShadedLightSourceObject : public LightSourceType
		{
		private:
			static constexpr int16_t WindowTolerance = 3;

		protected:
			using LightSourceType::ObjectPosition;
			using LightSourceType::WorldPosition;

			using LightSourceType::GetProximityFraction;

		public:
			using LightSourceType::Color;
			using LightSourceType::SceneShader;

		private:
			point_fragment_t LightFragment{};

		public:
			IFragmentShader<fragment_t>* FragmentShader = nullptr;

		public:
			ShadedLightSourceObject() : LightSourceType() {}

			bool PrimitiveScreenShade(const uint16_t index, const uint16_t boundsWidth, const uint16_t boundsHeight) final
			{
				if (LightSourceType::PrimitiveScreenShade(index, boundsWidth, boundsHeight))
				{
					if (ObjectPosition.z < 0
						|| ObjectPosition.x < -WindowTolerance
						|| ObjectPosition.y < -WindowTolerance
						|| ObjectPosition.x > boundsWidth + WindowTolerance
						|| ObjectPosition.y > boundsHeight + WindowTolerance
						|| (Color.r == 0 && Color.g == 0 && Color.b == 0)
						)
					{
						ObjectPosition.z = -VERTEX16_RANGE;
					}

					return true;
				}
				else
				{
					return false;
				}
			}

			void FragmentCollect(FragmentCollector& fragmentCollector) final
			{
				if (ObjectPosition.z > 0)
				{
					fragmentCollector.AddFragment(0, ObjectPosition.z);
				}
			}

			void FragmentShade(WindowRasterizer& rasterizer, const uint16_t index) final
			{
				if (FragmentShader != nullptr)
				{
					LightFragment.color = Color;
					LightFragment.material = { UFRACTION8_1X , 0, 0, 0 };
					LightFragment.screen = ObjectPosition;
					LightFragment.world = WorldPosition;

					if (SceneShader != nullptr)
					{
						FragmentShader->FragmentShade(rasterizer, LightFragment, SceneShader);
					}
					else
					{
						
					}
				}
			}
		};
	}

	namespace Lights
	{
		template<typename LightSourceType>
		struct TemplateOnOffLightSource : LightSourceType
		{
		private:
			color_fraction16_t OriginalColor{};

		public:
			void SetEnabled(const bool enabled)
			{
				if (enabled)
				{
					LightSourceType::Color = OriginalColor;
				}
				else
				{
					OriginalColor = LightSourceType::Color;
					LightSourceType::Color = ColorFraction::COLOR_BLACK;
				}
			}
		};
	}
}

#endif