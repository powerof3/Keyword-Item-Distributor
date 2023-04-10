#pragma once

namespace Cache
{
	namespace EditorID
	{
		using _GetFormEditorID = const char* (*)(std::uint32_t);

		std::string GetEditorID(const RE::TESForm* a_form);
	}

    namespace Item
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
			kSpell,

			kTotal
		};

		inline constexpr std::array itemTypes{ "Armor"sv, "Weapon"sv, "Ammo"sv, "Magic Effect"sv, "Potion"sv, "Scroll"sv, "Location"sv, "Ingredient"sv, "Book"sv, "Misc Item"sv, "Key"sv, "Soul Gem"sv, "Spell"sv };
		inline constexpr std::array logTypes{ "armors"sv, "weapons"sv, "ammo"sv, "magic effects"sv, "potions"sv, "scrolls"sv, "location"sv, "ingredients"sv, "books"sv, "misc items"sv, "keys"sv, "soul gems"sv, "spells"sv };

		TYPE  GetType(const std::string& a_type);
		std::string_view GetType(TYPE a_type);
	}

    namespace FormType
	{
		inline constexpr std::array set{
			//types
			RE::FormType::Armor,
			RE::FormType::Weapon,
			RE::FormType::Ammo,
			RE::FormType::MagicEffect,
			RE::FormType::AlchemyItem,
			RE::FormType::Scroll,
			RE::FormType::Location,
			RE::FormType::Ingredient,
			RE::FormType::Book,
			RE::FormType::Misc,
			RE::FormType::KeyMaster,
			RE::FormType::SoulGem,
			//filters
			RE::FormType::EffectShader,
			RE::FormType::ReferenceEffect,
			RE::FormType::ArtObject,
			RE::FormType::MusicType,
			RE::FormType::Faction,
			RE::FormType::Spell,	// also type
			RE::FormType::Projectile,
			RE::FormType::FormList
		};

		bool IsFilter(RE::FormType a_type);
	}
}
namespace ITEM = Cache::Item;
