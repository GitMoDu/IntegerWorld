#ifndef _INTEGER_WORLD_PRIMITIVE_SOURCES_NORMAL_h
#define _INTEGER_WORLD_PRIMITIVE_SOURCES_NORMAL_h

#include "../Framework/Model.h"

namespace IntegerWorld
{
	namespace PrimitiveSources
	{
		namespace Normal
		{
			namespace Static
			{
				struct NoSource
				{
					static constexpr bool HasNormals()
					{
						return false;
					}

					/// <summary>
					/// Mock normal source that always returns a zero vector.
					/// </summary>
					/// <param name="index"></param>
					/// <returns></returns>
					static constexpr vertex16_t GetNormal(const uint16_t index)
					{
						return vertex16_t{ 0, 0, 0 };
					}
				};

				template<int16_t x, int16_t y, int16_t z>
				struct FixedSource
				{
					static constexpr bool HasNormals()
					{
						return true;
					}

					static constexpr vertex16_t Normal{ x, y, z };

					/// <summary>
					/// Returns a copy of the fixed normal vector.
					/// 
					static constexpr vertex16_t GetNormal(const uint16_t index)
					{
						return Normal;
					}
				};

				class Source
				{
				private:
					const vertex16_t* NormalsSource;

				public:
					Source(const vertex16_t* normals) : NormalsSource(normals) {}

					static constexpr bool HasNormals()
					{
						return true;
					}

#if defined(ARDUINO_ARCH_AVR)
					/// <summary>
					/// Returns a copy of a precomputed triangle normal from ROM.
					/// </summary>
					const vertex16_t GetNormal(const uint16_t index) const
					{
						return vertex16_t{
							static_cast<int16_t>(pgm_read_word(&NormalsSource[index].x)),
							static_cast<int16_t>(pgm_read_word(&NormalsSource[index].y)),
							static_cast<int16_t>(pgm_read_word(&NormalsSource[index].z))
						};
					}
#else
					/// <summary>
					/// Returns a reference to a precomputed triangle normal from ROM.
					/// </summary>
					const vertex16_t& GetNormal(const uint16_t index) const
					{
						return NormalsSource[index];
					}
#endif
				};
			}

			namespace Dynamic
			{
				class Source
				{
				public:
					vertex16_t* NormalsSource;

				public:
					Source(vertex16_t* normals) : NormalsSource(normals) {}

					static constexpr bool HasNormals()
					{
						return true;
					}

					/// <summary>
					/// Returns a reference to a precomputed triangle normal from RAM.
					/// 
					const vertex16_t& GetNormal(const uint16_t index) const
					{
						return NormalsSource[index];
					}
				};

				struct SingleSource
				{
					vertex16_t Normal;

					static constexpr bool HasNormals()
					{
						return true;
					}

					SingleSource(const vertex16_t& normal) : Normal(normal) {}

					SingleSource()
						: Normal{ 0, VERTEX16_UNIT, 0 }
					{
					}

					const vertex16_t& GetNormal(const uint16_t index) const
					{
						return Normal;
					}
				};
			}

			static constexpr Static::NoSource NormalNoSourceInstance{};
		}
	}
}
#endif