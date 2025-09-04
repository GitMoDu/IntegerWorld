#ifndef _INTEGER_WORLD_BILLBOARD_OBJECT_h
#define _INTEGER_WORLD_BILLBOARD_OBJECT_h

#include "PointObject.h"

namespace IntegerWorld
{
	enum class BillboardScaleModeEnum : uint8_t
	{
		ScreenSpace,	// Fixed pixel size
		WorldSpace		// Scales with distance.
	};

	/// <summary>
	/// The billboard is always centered on its world position.
	/// The billboard is always axis-aligned (upright in screen space), with no rotation to match the camera’s up vector
	/// </summary>
	template<BillboardScaleModeEnum billboardScaleMode = BillboardScaleModeEnum::WorldSpace>
	class BillboardObject : public TranslationObject
	{
	public:
		int16_t ZOverride = -VERTEX16_UNIT;
		resize16_t Resize = RESIZE16_1X;
		static constexpr BillboardScaleModeEnum BillboardScaleMode = billboardScaleMode;

	public:
		IFragmentShader<billboard_fragment_t>* FragmentShader = nullptr;

	protected:
		resize16_t Proportion = RESIZE16_1X;
		uint16_t HeightScaled = 0;
		billboard_primitive_t Primitive{};
		vertex16_t Top{};

		resize16_t ProportionCopy;
		uint16_t Height;

	private:
		vertex16_t ScreenPosition{}; //TODO: Remove?

	public:
		BillboardObject()
			: TranslationObject()
			, Proportion(RESIZE16_1X)
			, Height(1)
		{
		}

		BillboardObject(const uint16_t width, const uint16_t height)
			: TranslationObject()
			, Height(height)
			, Proportion(Resize::GetResize16(width, height))
		{
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
			Proportion = Resize::GetResize16(MaxValue(uint16_t(1), width), MaxValue(uint16_t(1), height));
			Height = height;
		}

		virtual void ObjectShade(const frustum_t& frustum)
		{
			TranslationObject::ObjectShade(frustum);

			// Copy world position before camera transform.
			ScreenPosition = WorldPosition;

			// Double buffer to ensure consistent billboard widht/height.
			HeightScaled = Resize::Scale(Resize, Height);
			ProportionCopy = Proportion;

			// Simple frustum culling based on the center point.
			if (frustum.IsPointInside(WorldPosition))
			{
				Primitive.z = 0;
			}
			else
			{
				Primitive.z = -VERTEX16_UNIT;
			}
		}

		bool WorldTransform(const uint16_t vertexIndex) final
		{
			return true;
		}

		bool WorldShade(const frustum_t& frustum, const uint16_t primitiveIndex) final
		{
			return true;
		}

		bool CameraTransform(const transform16_camera_t& transform, const uint16_t vertexIndex) final
		{
			if (Primitive.z < 0)
				return true;

			ApplyCameraTransform(transform, ScreenPosition);

			Top = { ScreenPosition.x,
						   int16_t(ScreenPosition.y + HeightScaled),
						   ScreenPosition.z };

			if (HeightScaled < 1)
				Primitive.z = -VERTEX16_UNIT;

			return true;
		}

		bool ScreenProject(ViewportProjector& screenProjector, const uint16_t vertexIndex) final
		{
			screenProjector.Project(ScreenPosition);
			screenProjector.Project(Top);

			return true;
		}

		bool ScreenShade(const uint16_t primitiveIndex) final
		{
			if (Primitive.z < 0)
				return true;

			uint16_t width = 0;
			uint16_t height = 0;

			switch (BillboardScaleMode)
			{
			case BillboardScaleModeEnum::ScreenSpace: // Use a fixed pixel height regardless of distance
				height = HeightScaled;
				width = Resize::Scale(ProportionCopy, height);
				break;
			case BillboardScaleModeEnum::WorldSpace: // Pixel height based on the distance in screen-space.
			default:
				height = Distance16(Top, ScreenPosition);
				width = Resize::Scale(ProportionCopy, height);
				break;
			}

			const uint16_t halfWidth = (width >> 1);
			const uint16_t halfHeight = (height >> 1);

			// Cull if the billboard is outside the bounds.
			if ((halfWidth == 0)
				|| (halfHeight == 0)
				|| ScreenPosition.z <= 0)
			{
				Primitive.z = -VERTEX16_UNIT;
			}
			else
			{
				Primitive.topLeftX = ScreenPosition.x - halfWidth;
				Primitive.bottomRightX = ScreenPosition.x + halfWidth;
				Primitive.topLeftY = ScreenPosition.y - halfHeight;
				Primitive.bottomRightY = ScreenPosition.y + halfHeight;

				Primitive.z = ScreenPosition.z;
			}

			return true;
		}

		void FragmentCollect(FragmentCollector& fragmentCollector) final
		{
			if (Primitive.z >= 0)
			{
				if (ZOverride == -VERTEX16_UNIT)
				{
					fragmentCollector.AddFragment(0, Primitive.z);
				}
				else
				{
					fragmentCollector.AddFragment(0, ZOverride);
				}
			}
		}

		void FragmentShade(WindowRasterizer& rasterizer, const uint16_t primitiveIndex)
		{
			billboard_fragment_t fragment;
			fragment.world = ScreenPosition;
			fragment.topLeftX = Primitive.topLeftX;
			fragment.topLeftY = Primitive.topLeftY;
			fragment.bottomRightX = Primitive.bottomRightX;
			fragment.bottomRightY = Primitive.bottomRightY;

			if (FragmentShader != nullptr)
			{
				if (SceneShader != nullptr)
				{
					FragmentShader->FragmentShade(rasterizer, fragment, SceneShader);
				}
				else
				{
					FragmentShader->FragmentShade(rasterizer, fragment);

				}
			}
		}
	};
}
#endif