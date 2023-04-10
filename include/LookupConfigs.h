#pragma once

#include "LookupFilters.h"

namespace INI
{
	enum TYPE : std::uint32_t
	{
		kFormIDPair = 0,
		kFormID = kFormIDPair,
		kType,
		kESP = kType,
		kFilters,
		kTraits,
		kChance
	};

	struct Data
	{
		FormOrEditorID          rawForm{};
		StringFilters           stringFilters{};
		Filters<FormOrEditorID> rawFormFilters{};
		TraitsPtr               traits{};
		Chance                  chance{ 100 };
		Path                    path{};
	};
	using DataVec = std::vector<Data>;

	inline std::map<ITEM::TYPE, DataVec> INIs{};

	std::pair<bool, bool> GetConfigs();
}
using INIDataVec = INI::DataVec;
