#pragma once

namespace ITEM
{
	enum TYPE : std::uint32_t
	{
		kNone = static_cast<std::underlying_type_t<TYPE>>(-1),
		kArmor = 0,
		kWeapon,
		kAmmo,
		kMagicEffect,
		kPotion,
		kScroll,
		kLocation,
		kIngredient,
		kBook,
		kMiscItem,
		kKey,
		kSoulGem,

		kTotal
	};

	inline constexpr frozen::map<TYPE, std::string_view, kTotal> map = {
		{ kArmor, "armors"sv },
		{ kWeapon, "weapons"sv },
		{ kAmmo, "ammo"sv },
		{ kMagicEffect, "magic effects"sv },
		{ kPotion, "potions"sv },
		{ kScroll, "scrolls"sv },
		{ kLocation, "locations"sv },
		{ kIngredient, "ingredients"sv },
		{ kBook, "books"sv },
		{ kMiscItem, "misc items"sv },
		{ kKey, "keys"sv },
		{ kSoulGem, "soul gems"sv },
	};
}

namespace TRAITS
{
	template <class T>
	using min_max = std::pair<T, T>;

	enum TYPE : std::uint32_t
	{
		kArmor = 0,
		kWeapon,
		kAmmo,
		kMagicEffect,
		kPotion,
		kIngredient,
		kBook,
		kSoulGem,

		kTotal
	};

	namespace ARMOR
	{
		enum : std::uint32_t
		{
			kEnchanted,
			kTemplate,
			kArmorRating,
			kArmorType
		};

		using Traits = std::tuple<
			std::optional<bool>,
			std::optional<bool>,
			std::optional<min_max<float>>,
			std::optional<RE::BIPED_MODEL::ArmorType>>;
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
			std::optional<min_max<float>>>;
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
			std::optional<std::pair<RE::ActorValue, min_max<std::int32_t>>>>;
	}

	namespace POTION
	{
		enum : std::uint32_t
		{
			kPoison,
			kFood
		};

		using Traits = std::tuple<
			std::optional<bool>,
			std::optional<bool>>;
	}

	namespace INGREDIENT
	{
		enum : std::uint32_t
		{
			kFood
		};

		using Traits = std::optional<bool>;
	}

	namespace BOOK
	{
		enum : std::uint32_t
		{
			kTeachesSpell,
			kTeachesSkill,
			kActorValue
		};

		using Traits = std::tuple<
			std::optional<bool>,
			std::optional<bool>,
			std::optional<RE::ActorValue>>;
	}

	namespace SOULGEM
	{
		enum : std::uint32_t
		{
			kBlack,
			kSoulSize,
			kGemSize
		};

		using Traits = std::tuple<
			std::optional<bool>,
			std::optional<RE::SOUL_LEVEL>,
			std::optional<RE::SOUL_LEVEL>>;
	}

	using Traits = std::tuple<
		ARMOR::Traits,
		WEAP::Traits,
		AMMO::Traits,
		MGEF::Traits,
		POTION::Traits,
		INGREDIENT::Traits,
		BOOK::Traits,
		SOULGEM::Traits>;
}

namespace CONFIG
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

namespace Lookup
{
	enum FORMID_TYPE : std::uint32_t
	{
		kNone = 0,
		kFormIDMod,
		kMod,
		kFormID,
		kEditorID
	};

	namespace detail
	{
		inline bool is_mod_name(const std::string& a_str)
		{
			return a_str.contains(".esp") || a_str.contains(".esl") || a_str.contains(".esm");
		}
	}
}

using FormIDPair = std::pair<
	std::optional<RE::FormID>,
	std::optional<std::string>>;
using FormOrEditorID = std::variant<FormIDPair, std::string>;

using FormIDPairVec = std::vector<FormIDPair>;
using StringVec = std::vector<std::string>;
using Chance = float;
using Path = std::string;

using INIData = std::tuple<
	FormOrEditorID,
	std::array<StringVec, 4>,
	std::array<FormIDPairVec, 3>,
	TRAITS::Traits,
	Chance,
	Path>;
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

inline std::map<ITEM::TYPE, INIDataVec> INIs;
inline std::map<ITEM::TYPE, KeywordDataVec> Keywords;
