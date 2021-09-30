#pragma once

namespace ITEM
{
	enum TYPE : std::uint32_t
	{
		kArmor = 0,
		kWeapon,
		kAmmo,
		kMagicEffect,
		kPotion,
		kScroll,
		kLocation,
		kIngredient,

		kTotal
	};
}

namespace TRAITS
{
	enum TYPE : std::uint32_t
	{
		kArmor = 0,
		kWeapon,
		kAmmo,
		kMagicEffect,
		kPotion,
		kIngredient,

		kTotal
	};
	
	namespace ARMOR
	{
		enum : std::uint32_t
		{
			kEnchanted,
			kTemplate,
			kArmorRating,
		};

		using Traits = std::tuple<
			std::optional<bool>,
			std::optional<bool>,
			std::optional<
				std::pair<float, float>>>;
	}

	namespace WEAP
	{
		enum : std::uint32_t
		{
			kEnchanted,
			kTemplate,
			kWeight,
		};

		using Traits = std::tuple<
			std::optional<bool>,
			std::optional<bool>,
			std::optional<
				std::pair<float, float>>>;
	}

	namespace AMMO
	{
		enum : std::uint32_t
		{
			kIsBolt
		};

		using Traits = std::optional<bool>;
	}

	namespace MGEF
	{
		enum : std::uint32_t
		{
			kHostile,
			kCastingType,
			kDelivery,
			kSkill
		};

		using Traits = std::tuple<
			std::optional<bool>,
			std::optional<RE::MagicSystem::CastingType>,
			std::optional<RE::MagicSystem::Delivery>,
			std::optional<
				std::pair<RE::ActorValue,
					std::pair<std::int32_t, std::int32_t>>>>;
	}

	namespace POTION
	{
		enum : std::uint32_t
		{
			kPoison,
			kFood
		};

		using Traits = std::tuple<std::optional<bool>, std::optional<bool>>;
	}

	namespace INGREDIENT
	{
		enum : std::uint32_t
		{
			kFood
		};

		using Traits = std::optional<bool>;
	}

	using Traits = std::tuple<
		ARMOR::Traits,
		WEAP::Traits,
		AMMO::Traits,
		MGEF::Traits,
		POTION::Traits,
		INGREDIENT::Traits>;
}

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
}
using INI_TYPE = INI::TYPE;

namespace DATA
{
	enum TYPE : std::uint32_t
	{
		kForm = 0,
		kStrings,
		kFilters,
		kTraits,
		kChance,
		kCount
	};
}
using DATA_TYPE = DATA::TYPE;

using FormIDPair = std::pair<
	std::optional<RE::FormID>,
	std::optional<std::string>>;
using FormIDPairVec = std::vector<FormIDPair>;
using StringVec = std::vector<std::string>;
using Chance = float;

using INIData = std::tuple<
	std::variant<FormIDPair, std::string>,
	std::array<StringVec, 4>,
	std::array<FormIDPairVec, 3>,
	TRAITS::Traits,
	Chance>;
using INIDataVec = std::vector<INIData>;

using FormVec = std::vector<
	std::variant<RE::TESForm*, const RE::TESFile*>>;
using Count = std::uint32_t;

using KeywordData = std::tuple<
	RE::BGSKeyword*,
	std::array<StringVec, 4>,
	std::array<FormVec, 3>,
	TRAITS::Traits,
	Chance,
	Count>;
using KeywordDataVec = std::vector<KeywordData>;
