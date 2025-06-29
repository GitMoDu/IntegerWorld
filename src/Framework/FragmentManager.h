#ifndef _INTEGER_WORLD_FRAGMENT_MANAGER_h
#define _INTEGER_WORLD_FRAGMENT_MANAGER_h

#include "../Framework/Model.h"
#if __has_include(<algorithm>) && !defined(SKIP_STD)
#include <algorithm>
#endif

namespace IntegerWorld
{
	class FragmentCollector
	{
	protected:
		ordered_fragment_t* Fragments;
		const uint16_t MaxFragments;
		uint16_t FragmentCount = 0;

	protected:
		uint16_t ObjectIndex = 0;

	public:
		FragmentCollector(ordered_fragment_t* fragments, const uint16_t maxFragments)
			: Fragments(fragments)
			, MaxFragments(maxFragments)
		{
		}

		bool AddFragment(const uint16_t fragmentIndex, const uint16_t z)
		{
			if (FragmentCount < MaxFragments)
			{
				Fragments[FragmentCount].ObjectIndex = ObjectIndex;
				Fragments[FragmentCount].FragmentIndex = fragmentIndex;
				Fragments[FragmentCount].Z = z;
				FragmentCount++;

				return true;
			}
			return false;
		}
	};

	template<uint16_t MaxOrderedFragments>
	class OrderedFragmentManager : FragmentCollector
	{
	public:
		OrderedFragmentManager(ordered_fragment_t* fragments)
			: FragmentCollector(fragments, MaxOrderedFragments)
		{
		}

		void Clear()
		{
			FragmentCount = 0;
			ObjectIndex = 0;
		}

		void PrepareForObject(const uint16_t objectIndex)
		{
			ObjectIndex = objectIndex;
		}

		uint16_t Count() const
		{
			return FragmentCount;
		}

		void Sort()
		{
#if __has_include(<algorithm>) && !defined(SKIP_STD)
			std::sort(Fragments, Fragments + FragmentCount,
				[](const ordered_fragment_t& a, const ordered_fragment_t& b)
				{
					return a.Z > b.Z;
				});
#else
			ordered_fragment_t temp{};
			for (uint_fast16_t gap = FragmentCount >> 1; gap > 0; gap >>= 1)
			{
				for (uint_fast16_t i = gap; i < FragmentCount; ++i)
				{
					temp = Fragments[i];
					uint_fast16_t j = i;
					while (j >= gap && Fragments[j - gap].Z < temp.Z)
					{
						Fragments[j] = Fragments[j - gap];
						j -= gap;
					}
					Fragments[j] = temp;
				}
			}
#endif
		}
	};
}
#endif