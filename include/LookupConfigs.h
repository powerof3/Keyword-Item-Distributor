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
		FormIDOrString rawForm{};
		RawFilters     rawFilters{};
		TraitsPtr      traits{};
		Chance         chance{ 100 };
		std::string    path{};
	};
	using DataVec = std::vector<Data>;

	inline std::map<ITEM::TYPE, DataVec> INIs{};

	std::pair<bool, bool> GetConfigs();
}
using INIDataVec = INI::DataVec;
