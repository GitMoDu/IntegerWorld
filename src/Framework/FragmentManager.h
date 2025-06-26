#ifndef _INTEGER_WORLD_FRAGMENT_MANAGER_h
#define _INTEGER_WORLD_FRAGMENT_MANAGER_h

#include "../Framework/Model.h"

namespace IntegerWorld
{
	class FragmentCollector
	{
	protected:
		ordered_fragment_t* Fragments;
		const uint16_t MaxFragments;
		uint16_t FragmentCount = 0;
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

	class OrderedFragmentManager : FragmentCollector
	{
	public:
		OrderedFragmentManager(ordered_fragment_t* fragments, const uint16_t maxFragments)
			: FragmentCollector(fragments, maxFragments)
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
			// Insertion sort using the Compare method
			for (uint16_t i = 1; i < FragmentCount; ++i)
			{
				ordered_fragment_t item = Fragments[i];
				uint16_t j = i;

				// Move elements of Items[0..i-1] that are greater than key
				// to one position ahead of their current position
				for (; j > 0; --j)
				{
					if (!(Fragments[j - 1].Z < item.Z))
					{
						break;
					}
					Fragments[j] = Fragments[j - 1];
				}
				Fragments[j] = item;
			}
		}
	};


}
#endif