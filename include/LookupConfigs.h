#pragma once

#include "Defs.h"

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
		Data() = default;
		Data(std::string& a_value, const std::string& a_path);

		// members
		DISTRIBUTION::TYPE type{ DISTRIBUTION::TYPE::kNone };
		RawForm            rawForm{};
		ConfigCriteria     criteria{};
		std::string        path{};
		RE::BGSKeyword*    resolvedKeyword;
	};
	using DataVec = std::vector<Data>;

	inline std::map<DISTRIBUTION::TYPE, DataVec> INIs{};

	std::pair<bool, bool> GetConfigs();
}
using INIDataVec = INI::DataVec;
