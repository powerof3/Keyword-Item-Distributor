#pragma once

using FormIDPair = std::pair<RE::FormID, std::optional<std::string>>;
using FormIDPairVec = std::vector<FormIDPair>;
using StringVec = std::vector<std::string>;
using FormVec = std::vector<RE::TESForm*>;

using Chance = float;
using Count = std::uint32_t;

using INIData = std::tuple<
	std::variant<FormIDPair, std::string>,
	std::array<StringVec, 4>,
	std::array<FormIDPairVec, 3>,
	Chance>;
using INIDataVec = std::vector<INIData>;

using KeywordData = std::tuple<
	RE::BGSKeyword*,
	std::array<StringVec, 4>,
	std::array<FormVec, 3>,
	Chance,
	Count>;
using KeywordDataVec = std::vector<KeywordData>;

namespace INI
{
	enum TYPE : std::uint32_t
	{
		kFormIDPair = 0,
		kFormID = kFormIDPair,
		kType,
		kESP = kType,
		kFilters,
		kChance
	};

	using Values = std::vector<std::pair<std::string, std::string>>;
}
using INI_TYPE = INI::TYPE;

namespace DATA
{
	enum TYPE : std::uint32_t
	{
		kForm = 0,
		kStrings,
		kFilters,
		kChance,
		kCount
	};
}
using DATA_TYPE = DATA::TYPE;

namespace ITEM
{
	enum TYPE : std::uint32_t
	{
		kArmor = 0,
		kWeapon,
		kAmmo,

		kTotal
	};
}
