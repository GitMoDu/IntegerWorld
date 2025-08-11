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
	/// The billboard is always centered on ObjectPosition.
	/// The billboard is always axis-aligned (upright in screen space), with no rotation to match the camera’s up vector
	/// </summary>
	template<BillboardScaleModeEnum billboardScaleMode = BillboardScaleModeEnum::WorldSpace>
	class BillboardObject : public TranslationObject
	{
	public:
		int16_t ZOverride = -VERTEX16_RANGE;
		resize16_t Resize = RESIZE16_1X;
		static constexpr BillboardScaleModeEnum BillboardScaleMode = billboardScaleMode;

	public:
		IFragmentShader<billboard_fragment_t>* FragmentShader = nullptr;

	protected:
		resize16_t Proportion = RESIZE16_1X;
		uint16_t HeightScaled = 0;
		billboard_primitive_t Primitive{};
		vertex16_t Top{};

		uint16_t Height;
		resize16_t ProportionCopy;

	public:
		BillboardObject()
			: TranslationObject()
			, Height(1)
			, Proportion(RESIZE16_1X)
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
			ZOverride = -VERTEX16_RANGE;
		}

		void SetDimensions(const uint16_t width, const uint16_t height)
		{
			Proportion = Resize::GetResize16(MaxValue(uint16_t(1), width), MaxValue(uint16_t(1), height));
			Height = height;
		}

		bool VertexShade(const uint16_t index) final
		{
			// Apply world transform to object.
			WorldPosition = Translation;

			// Copy world position before camera transform.
			ObjectPosition = WorldPosition;

			// Double buffer to ensure consistent billboard widht/height.
			HeightScaled = Resize::Scale(Resize, Height);
			ProportionCopy = Proportion;

			return true;
		}

		bool PrimitiveWorldShade(const uint16_t index) final
		{
			Primitive.z = 0;
			//TODO: Frustum culling.

			return true;
		}

		bool CameraTransform(const camera_transform_t& transform, const uint16_t index) final
		{
			ApplyCameraTransform(transform, ObjectPosition);

			Top = { ObjectPosition.x,
						   int16_t(ObjectPosition.y + HeightScaled),
						   ObjectPosition.z };

			if (HeightScaled < 1)
				Primitive.z = -VERTEX16_RANGE;

			return true;
		}

		bool ScreenProject(ViewportProjector& screenProjector, const uint16_t index) final
		{
			screenProjector.Project(ObjectPosition);
			screenProjector.Project(Top);

			return true;
		}

		bool PrimitiveScreenShade(const uint16_t index, const uint16_t boundsWidth, const uint16_t boundsHeight) final
		{
			if (Primitive.z != -VERTEX16_RANGE)
			{
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
					height = Distance16(Top, ObjectPosition);
					width = Resize::Scale(ProportionCopy, height);
					break;
				}

				const uint16_t halfWidth = (width >> 1);
				const uint16_t halfHeight = (height >> 1);

				// Cull if the billboard is outside the bounds.
				if ((halfWidth == 0)
					|| (halfHeight == 0)
					|| ObjectPosition.z <= 0
					|| (ObjectPosition.x + halfWidth < 0)
					|| (ObjectPosition.x - halfWidth >= boundsWidth)
					|| (ObjectPosition.y + halfHeight < 0)
					|| (ObjectPosition.y - halfHeight >= boundsHeight))
				{
					Primitive.z = -VERTEX16_RANGE;
				}
				else
				{
					Primitive.topLeftX = ObjectPosition.x - halfWidth;
					Primitive.bottomRightX = ObjectPosition.x + halfWidth;
					Primitive.topLeftY = ObjectPosition.y - halfHeight;
					Primitive.bottomRightY = ObjectPosition.y + halfHeight;

					Primitive.z = ObjectPosition.z;
				}
			}

			return true;
		}

		void FragmentCollect(FragmentCollector& fragmentCollector) final
		{
			if (Primitive.z != -VERTEX16_RANGE)
			{
				if (ZOverride == -VERTEX16_RANGE)
				{
					fragmentCollector.AddFragment(0, Primitive.z);
				}
				else
				{
					fragmentCollector.AddFragment(0, ZOverride);
				}
			}
		}

		void FragmentShade(WindowRasterizer& rasterizer, const uint16_t index)
		{
			billboard_fragment_t fragment;
			fragment.world = ObjectPosition;
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