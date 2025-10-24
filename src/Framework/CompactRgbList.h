#ifndef _INTEGER_WORLD_COMPACT_RGB_LIST_h
#define _INTEGER_WORLD_COMPACT_RGB_LIST_h

#include "Model.h"

namespace IntegerWorld
{
	template<uint16_t Count>
	struct CompactRgb8List
	{
	private:
		static constexpr size_t ByteSize = static_cast<size_t>(Count) * 3;

	private:
		uint8_t Colors[ByteSize]{};

	public:
		void SetColor(const Rgb8::color_t color, const uint16_t colorIndex)
		{
			const size_t baseIndex = colorIndex * 3;
			Colors[baseIndex + 0] = Rgb8::Red(color);
			Colors[baseIndex + 1] = Rgb8::Green(color);
			Colors[baseIndex + 2] = Rgb8::Blue(color);
		}

		Rgb8::color_t GetColor(const uint16_t colorIndex) const
		{
			const size_t baseIndex = static_cast<size_t>(colorIndex) * 3;
			return Rgb8::Color(Colors[baseIndex + 0], Colors[baseIndex + 1], Colors[baseIndex + 2]);
		}
	};
}
#endif