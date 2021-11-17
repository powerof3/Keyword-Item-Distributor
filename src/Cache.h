#pragma once

#include "Defs.h"

//implements https://github.com/Ryan-rsm-McKenzie/CCExtender/blob/master/src/EditorIDCache.h
namespace Cache
{
	class EditorID
	{
	public:
		static EditorID* GetSingleton();

		void FillMap();

		std::string GetEditorID(RE::FormID a_formID);
		RE::FormID GetFormID(const std::string& a_editorID);

	private:
		using Lock = std::mutex;
		using Locker = std::scoped_lock<Lock>;

		EditorID() = default;
		EditorID(const EditorID&) = delete;
		EditorID(EditorID&&) = delete;
		~EditorID() = default;

		EditorID& operator=(const EditorID&) = delete;
		EditorID& operator=(EditorID&&) = delete;

		mutable Lock _lock;
		robin_hood::unordered_flat_map<RE::FormID, std::string> _formIDToEditorIDMap;
		robin_hood::unordered_flat_map<std::string, RE::FormID> _editorIDToFormIDMap;
	};

	namespace FormType
	{
		inline constexpr frozen::map<RE::FormType, std::string_view, 13> map = {
			{ RE::FormType::Armor, "Armor"sv },
			{ RE::FormType::Weapon, "Weapon"sv },
			{ RE::FormType::Ammo, "Ammo"sv },
			{ RE::FormType::MagicEffect, "Magic Effect"sv },
			{ RE::FormType::AlchemyItem, "Potion"sv },
			{ RE::FormType::Scroll, "Scroll"sv },
			{ RE::FormType::Location, "Location"sv },
			{ RE::FormType::Ingredient, "Ingredient"sv },
			{ RE::FormType::EffectShader, "Effect Shader"sv },
			{ RE::FormType::ReferenceEffect, "Visual Effect"sv },
			{ RE::FormType::ArtObject, "Art Object"sv },
			{ RE::FormType::MusicType, "MusicType"sv },
			{ RE::FormType::Faction, "Faction"sv }
		};

		std::string GetString(const RE::FormType a_type);
	}

	namespace Item
	{
		inline constexpr frozen::map<std::string_view, ITEM::TYPE, 9> map = {
			{ "Armor"sv, ITEM::kArmor },
			{ "Weapon"sv, ITEM::kWeapon },
			{ "Ammo"sv, ITEM::kAmmo },
			{ "Magic Effect"sv, ITEM::kMagicEffect },
			{ "Potion"sv, ITEM::kPotion },
			{ "Scroll"sv, ITEM::kScroll },
			{ "Location"sv, ITEM::kLocation },
			{ "Ingredient"sv, ITEM::kIngredient },
			{ "Book"sv, ITEM::kBook }
		};
		
		ITEM::TYPE GetType(const std::string& a_type);
	}
}
