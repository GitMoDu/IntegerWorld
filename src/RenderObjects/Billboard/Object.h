#ifndef _INTEGER_WORLD_RENDER_OBJECTS_BILLBOARD_OBJECT_h
#define _INTEGER_WORLD_RENDER_OBJECTS_BILLBOARD_OBJECT_h

#include "../AbstractObject.h"

namespace IntegerWorld
{
	namespace RenderObjects
	{
		namespace Billboard
		{
			/// <summary>
			/// The billboard is always centered on its world position.
			/// The billboard is always axis-aligned (upright in screen space), with no rotation to match the camera’s up vector
			/// </summary>
			template<BillboardScaleModeEnum billboardScaleMode = BillboardScaleModeEnum::WorldSpace>
			class AxisAlignedObject : public AbstractTranslationObject
			{
			public:
				static constexpr BillboardScaleModeEnum BillboardScaleMode = billboardScaleMode;

			public:
				/// <summary>
				/// Billboard fragment shader. If null, no fragments are shaded.
				/// </summary>
				IFragmentShader<billboard_fragment_t>* FragmentShader = nullptr;

				/// <summary>
				/// Scene's lighting shader. If null, no lighting is applied.
				/// </summary>
				ISceneShader* SceneShader = nullptr;

			private:
				int16_t ZOverride = -VERTEX16_UNIT;
				scale16_t Resize = SCALE16_1X;

				scale16_t Proportion = SCALE16_1X;
				uint16_t HeightScaled = 0;
				int16_t Primitive{};
				vertex16_t Top{};
				vertex16_t ScreenPosition{};

				scale16_t ProportionCopy;
				uint16_t Height;

			private:
				int16_t topLeftX;
				int16_t topLeftY;
				int16_t bottomRightX;
				int16_t bottomRightY;

			private:
				Rgb8::color_t Albedo = Rgb8::WHITE;
				Rgb8::color_t PrimitiveColor = Rgb8::WHITE;

			public:
				AxisAlignedObject()
					: AbstractTranslationObject()
					, Proportion(SCALE16_1X)
					, Height(1) {
				}

				AxisAlignedObject(const uint16_t width, const uint16_t height)
					: AbstractTranslationObject()
					, Height(height)
					, Proportion(Scale16::GetFactor(width, height))
				{
				}

				void SetAlbedo(const Rgb8::color_t albedo)
				{
					Albedo = albedo;
				}

				Rgb8::color_t GetAlbedo() const
				{
					return Albedo;
				}

				void SetZOverride(const int16_t zOverride)
				{
					ZOverride = zOverride;
				}

				void ClearZOverride()
				{
					ZOverride = -VERTEX16_UNIT;
				}

				void SetDimensions(const uint16_t width, const uint16_t height)
				{
					Proportion = Scale16::GetFactor(MaxValue(uint16_t(1), width), MaxValue(uint16_t(1), height));
					Height = height;
				}

				virtual void ObjectShade(const frustum_t& frustum)
				{
					AbstractTranslationObject::ObjectShade(frustum);

					// Double buffer to ensure consistent billboard widht/height.
					HeightScaled = Scale(Resize, Height);
					ProportionCopy = Proportion;

					// Simple frustum culling based on the center point.
					if (frustum.IsPointInside(WorldPosition))
					{
						Primitive = 0;
					}
					else
					{
						Primitive = -VERTEX16_UNIT;
					}
				}

				bool WorldTransform(const uint16_t vertexIndex) { return true; }

				bool WorldShade(const frustum_t& frustum, const uint16_t primitiveIndex)
				{
					//TODO: Apply scene lighting.
					PrimitiveColor = Albedo;

					return true;
				}

				bool CameraTransform(const transform16_camera_t& transform, const uint16_t vertexIndex)
				{
					if (Primitive < 0)
						return true;

					ScreenPosition = WorldPosition;
					ApplyCameraTransform(transform, ScreenPosition);

					Top = { ScreenPosition.x,
								   int16_t(ScreenPosition.y + HeightScaled),
								   ScreenPosition.z };

					if (HeightScaled < 1)
						Primitive = -VERTEX16_UNIT;

					return true;
				}

				bool ScreenProject(ViewportProjector& screenProjector, const uint16_t vertexIndex)
				{
					screenProjector.Project(ScreenPosition);
					screenProjector.Project(Top);

					return true;
				}

				bool ScreenShade(const uint16_t primitiveIndex)
				{
					if (Primitive < 0)
						return true;

					uint16_t width = 0;
					uint16_t height = 0;

					switch (BillboardScaleMode)
					{
					case BillboardScaleModeEnum::ScreenSpace: // Use a fixed pixel height regardless of distance
						height = HeightScaled;
						width = Scale(ProportionCopy, height);
						break;
					case BillboardScaleModeEnum::WorldSpace: // Pixel height based on the distance in screen-space.
					default:
						height = Distance16(Top, ScreenPosition);
						width = Scale(ProportionCopy, height);
						break;
					}

					const uint16_t halfWidth = (width >> 1);
					const uint16_t halfHeight = (height >> 1);

					// Cull if the billboard is outside the bounds.
					if ((halfWidth == 0)
						|| (halfHeight == 0)
						|| ScreenPosition.z <= 0)
					{
						Primitive = -VERTEX16_UNIT;
					}
					else
					{
						topLeftX = ScreenPosition.x - halfWidth;
						bottomRightX = ScreenPosition.x + halfWidth;
						topLeftY = ScreenPosition.y - halfHeight;
						bottomRightY = ScreenPosition.y + halfHeight;

						Primitive = ScreenPosition.z;
					}

					return true;
				}

				void FragmentCollect(FragmentCollector& fragmentCollector)
				{
					if (Primitive >= 0)
					{
						if (ZOverride == -VERTEX16_UNIT)
						{
							fragmentCollector.AddFragment(0, Primitive);
						}
						else
						{
							fragmentCollector.AddFragment(0, ZOverride);
						}
					}
				}

				void FragmentShade(WindowRasterizer& rasterizer, const uint16_t primitiveIndex) final
				{
					if (primitiveIndex == 0)
					{
						billboard_fragment_t fragment;
						fragment.red = Rgb8::Red(PrimitiveColor);
						fragment.green = Rgb8::Green(PrimitiveColor);
						fragment.blue = Rgb8::Blue(PrimitiveColor);
						fragment.topLeftX = topLeftX;
						fragment.topLeftY = topLeftY;
						fragment.bottomRightX = bottomRightX;
						fragment.bottomRightY = bottomRightY;
						fragment.z = Primitive;

						if (FragmentShader != nullptr)
						{
							FragmentShader->FragmentShade(rasterizer, fragment);
						}
					}
				}
			};
		}
	}
}
#endif