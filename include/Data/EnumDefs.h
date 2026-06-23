#pragma once

namespace DISTRIBUTION
{
	// distribution item types
	struct TYPES
	{
		enum TYPE
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
			kActivator,
			kFlora,
			kFurniture,
			kRace,
			kTalkingActivator,
			kEnchantmentItem,

			kTotal
		};
	};
	using TYPE = TYPES::TYPE;

	inline constexpr std::array header_strings{
		"Armor"sv,
		"Weapon"sv,
		"Ammo"sv,
		"Magic Effect"sv,
		"Potion"sv,
		"Scroll"sv,
		"Location"sv,
		"Ingredient"sv,
		"Book"sv,
		"Misc Item"sv,
		"Key"sv,
		"Soul Gem"sv,
		"Spell"sv,
		"Activator"sv,
		"Flora"sv,
		"Furniture"sv,
		"Race"sv,
		"Talking Activator"sv,
		"Enchantment"sv
	};
	static_assert(header_strings.size() == TYPE::kTotal);

	inline constexpr std::array form_strings{
		"armors"sv,
		"weapons"sv,
		"ammo"sv,
		"magic effects"sv,
		"potions"sv,
		"scrolls"sv,
		"location"sv,
		"ingredients"sv,
		"books"sv,
		"misc items"sv,
		"keys"sv,
		"soul gems"sv,
		"spells"sv,
		"activators"sv,
		"flora"sv,
		"furniture"sv,
		"races"sv,
		"talking activators"sv,
		"enchantments"sv
	};
	static_assert(form_strings.size() == TYPE::kTotal);

	inline TYPE GetType(std::string_view a_type)
	{
		const auto it = std::ranges::find(header_strings, a_type);
		return it != header_strings.end() ? static_cast<TYPE>(std::distance(header_strings.begin(), it)) : TYPE::kNone;
	}

	inline std::string_view GetType(TYPE a_type)
	{
		return header_strings[a_type];
	}
}
