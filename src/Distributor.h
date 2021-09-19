#pragma once

#include "Defs.h"

namespace INI
{
	namespace detail
	{
		inline std::vector<std::string> split_sub_string(const std::string& a_str, const std::string& a_delimiter = ",")
		{
			if (!a_str.empty() && a_str.find("NONE"sv) == std::string::npos) {
				return string::split(a_str, a_delimiter);
			}
			return std::vector<std::string>();
		}

		inline FormIDPair get_formID(const std::string& a_str)
		{
			if (a_str.find("~"sv) != std::string::npos) {
				auto splitID = string::split(a_str, "~");
				return std::make_pair(
					string::lexical_cast<RE::FormID>(splitID.at(kFormID), true),
					splitID.at(kESP));
			} else if (string::icontains(a_str, ".esp") || string::icontains(a_str, ".esl") || string::icontains(a_str, ".esm")) {
				return std::make_pair(
					std::nullopt,
					a_str);
			} else {
				return std::make_pair(
					string::lexical_cast<RE::FormID>(a_str, true),
					std::nullopt);			
			}
		}

		template <class T>
		std::pair<T, T> get_minmax_values(std::string& a_str)
		{
			std::pair<T, T> minMax{ std::numeric_limits<T>::max(), std::numeric_limits<T>::max() };

			auto sanitizedStr = string::remove_non_numeric(a_str);
			if (auto values = string::split(sanitizedStr, " "); !values.empty()) {
				if (values.size() > 1) {
					minMax.first = string::lexical_cast<T>(values.at(0));
					minMax.second = string::lexical_cast<T>(values.at(1));
				} else {
					minMax.first = string::lexical_cast<T>(values.at(0));
				}
			}

			return minMax;
		}

		template <class T>
		std::optional<T> get_single_value(std::string& a_str)
		{
			auto sanitizedStr = string::remove_non_numeric(a_str);
			if (auto values = string::split(sanitizedStr, " "); !values.empty()) {
				return string::lexical_cast<T>(values.at(0));
			}
			return std::nullopt;
		}
	}

	inline std::pair<INIData, ITEM::TYPE> parse_ini(const std::string& a_value)
	{
		INIData data;
		auto& [formIDPair_ini, strings_ini, filterIDs_ini, traits_ini, chance_ini] = data;

		auto sections = string::split(a_value, "|");

		//[FORMID/ESP] / string
		std::variant<FormIDPair, std::string> item_ID;
		try {
			if (auto formSection = sections.at(kFormID); formSection.find('~') != std::string::npos || string::is_only_hex(formSection)) {
				FormIDPair pair;
				pair.second = std::nullopt;

				if (string::is_only_hex(formSection)) {
					// formID
					pair.first = string::lexical_cast<RE::FormID>(formSection, true);
				} else {
					// formID~esp
					pair = detail::get_formID(formSection);
				}

				item_ID.emplace<FormIDPair>(pair);
			} else {
				item_ID.emplace<std::string>(formSection);
			}
		} catch (...) {
			FormIDPair pair = { 0, std::nullopt };
			item_ID.emplace<FormIDPair>(pair);
		}
		formIDPair_ini = item_ID;

		//TYPE
		ITEM::TYPE type = ITEM::kArmor;
		try {
			if (const auto typeStr = sections.at(kType); !typeStr.empty() && typeStr.find("NONE"sv) == std::string::npos) {
				if (typeStr == "Armor" || typeStr == "Armour") {
					type = ITEM::kArmor;
				} else if (typeStr == "Weapon") {
					type = ITEM::kWeapon;
				} else if (typeStr == "Ammo") {
					type = ITEM::kAmmo;
				} else if (typeStr == "Magic Effect") {
					type = ITEM::kMagicEffect;
				} else if (typeStr == "Potion") {
					type = ITEM::kPotion;
				} else if (typeStr == "Scroll") {
					type = ITEM::kScroll;
				} else if (typeStr == "Location") {
					type = ITEM::kLocation;
				} else {
					type = string::lexical_cast<ITEM::TYPE>(typeStr);
				}
			}
		} catch (...) {
		}

		//FILTERS
		try {
			auto& [strings_ALL, strings_NOT, strings_MATCH, strings_ANY] = strings_ini;
			auto& [filterIDs_ALL, filterIDs_NOT, filterIDs_MATCH] = filterIDs_ini;

			for (auto split_str = detail::split_sub_string(sections.at(kFilters)); auto& str : split_str) {
				if (str.find('~') != std::string::npos || string::is_only_hex(str) || string::icontains(str, ".esp") || string::icontains(str, ".esl") || string::icontains(str, ".esm")) {
					if (str.find("+"sv) != std::string::npos) {
						auto splitIDs_ALL = detail::split_sub_string(str, "+");
						for (auto& IDs_ALL : splitIDs_ALL) {
							filterIDs_ALL.push_back(detail::get_formID(IDs_ALL));
						}
					} else if (str.at(0) == '-') {
						str.erase(0, 1);
						filterIDs_NOT.push_back(detail::get_formID(str));

					} else {
						filterIDs_MATCH.push_back(detail::get_formID(str));
					}
				} else {
					if (str.find("+"sv) != std::string::npos) {
						auto strings = detail::split_sub_string(str, "+");
						strings_ALL.insert(strings_ALL.end(), strings.begin(), strings.end());

					} else if (str.at(0) == '-') {
						str.erase(0, 1);
						strings_NOT.emplace_back(str);

					} else if (str.at(0) == '*') {
						str.erase(0, 1);
						strings_ANY.emplace_back(str);

					} else {
						strings_MATCH.emplace_back(str);
					}
				}
			}
		} catch (...) {
		}

		//TRAITS
		traits_ini;
		try {
			for (auto split_str = detail::split_sub_string(sections.at(kTraits)); auto& str : split_str) {
				switch (type) {
				case ITEM::kArmor:
					{
						auto& [enchanted, templated, armorRating] = std::get<ITEM::kArmor>(traits_ini);
						if (str.find("AR") != std::string::npos) {
							armorRating = detail::get_minmax_values<float>(str);
						} else if (str == "E") {
							enchanted = true;
						} else if (str == "-E") {
							enchanted = false;
						} else if (str == "T") {
							templated = true;
						} else if (str == "-T") {
							templated = false;
						}
					}
					break;
				case ITEM::kWeapon:
					{
						auto& [enchanted, templated, weight] = std::get<ITEM::kWeapon>(traits_ini);
						if (str.find("W") != std::string::npos) {
							weight = detail::get_minmax_values<float>(str);
						} else if (str == "E") {
							enchanted = true;
						} else if (str == "-E") {
							enchanted = false;
						} else if (str == "T") {
							templated = true;
						} else if (str == "-T") {
							templated = false;
						}
					}
					break;
				case ITEM::kAmmo:
					{
						auto& isBolt = std::get<ITEM::kAmmo>(traits_ini);
						if (str == "B") {
							isBolt = true;
						} else if (str == "-B") {
							isBolt = false;
						}
					}
					break;
				case ITEM::kMagicEffect:
					{
						auto& [isHostile, castingType, deliveryType, skillValue] = std::get<ITEM::kMagicEffect>(traits_ini);
						if (str.find("DT") != std::string::npos) {
							deliveryType = detail::get_single_value<RE::MagicSystem::Delivery>(str);
						} else if (str.find("CT") != std::string::npos) {
							castingType = detail::get_single_value<RE::MagicSystem::CastingType>(str);
						} else if (str.find('(') != std::string::npos) {
							auto sanitizedStr = string::remove_non_numeric(str);
							auto value = string::split(sanitizedStr, " ");
							if (!value.empty()) {
								auto skill = string::lexical_cast<RE::ActorValue>(value.at(0));
								auto min = string::lexical_cast<std::int32_t>(value.at(1));
								if (value.size() > 2) {
									auto max = string::lexical_cast<std::int32_t>(value.at(2));
									skillValue = { skill, { min, max } };
								} else {
									skillValue = { skill, { min, std::numeric_limits<std::int32_t>::max() } };
								}
							}
						} else if (str == "H") {
							isHostile = true;
						} else if (str == "-H") {
							isHostile = false;
						}
					}
					break;
				case ITEM::kPotion:
					{
						auto& [isPoison, isFood] = std::get<ITEM::kPotion>(traits_ini);
						if (str == "P") {
							isPoison = true;
						} else if (str == "-P") {
							isPoison = false;
						} else if (str == "F") {
							isFood = true;
						} else if (str == "-F") {
							isFood = false;
						}
					}
					break;
				default:
					break;
				}
			}
		} catch (...) {
		}

		//CHANCE
		chance_ini = 100;
		try {
			if (const auto chanceStr = sections.at(kChance); !chanceStr.empty() && chanceStr.find("NONE"sv) == std::string::npos) {
				chance_ini = string::lexical_cast<float>(chanceStr);
			}
		} catch (...) {
		}

		return std::make_pair(data, type);
	}

	bool Read();
}

namespace Lookup
{
	namespace detail
	{
		inline std::string_view lookup_form_type(const RE::FormType a_type)
		{
			switch (a_type) {
			case RE::FormType::Armor:
				return "Armor"sv;
			case RE::FormType::Weapon:
				return "Weapon"sv;
			case RE::FormType::Ammo:
				return "Ammo"sv;
			case RE::FormType::MagicEffect:
				return "Magic Effect"sv;
			case RE::FormType::AlchemyItem:
				return "Potion"sv;
			case RE::FormType::Scroll:
				return "Scroll"sv;
			case RE::FormType::Location:
				return "Location"sv;
			case RE::FormType::EffectShader:
				return "Effect Shader"sv;
			case RE::FormType::ReferenceEffect:
				return "Visual Effect"sv;
			case RE::FormType::ArtObject:
				return "Art Object"sv;
			case RE::FormType::MusicType:
				return "Music Type"sv;
			case RE::FormType::Faction:
				return "Faction"sv;
			default:
				return ""sv;
			}
		};

		inline void formID_to_form(RE::TESDataHandler* a_dataHandler, const FormIDPairVec& a_formIDVec, FormVec& a_formVec)
		{
			if (!a_formIDVec.empty()) {
				for (auto& [optFormID, modName] : a_formIDVec) {
					if (modName.has_value() && !optFormID.has_value()) {
						if (const RE::TESFile* filterMod = a_dataHandler->LookupModByName(modName.value()); filterMod) {
							logger::info("			Filter ({}) INFO - mod found", filterMod->fileName);
							a_formVec.push_back(filterMod);
						} else {
							logger::error("			Filter ({}) SKIP - mod cannot be found", modName.value());
						}						
					} else {
						auto formID = optFormID.value();
						RE::TESForm* filterForm = nullptr;
						if (modName.has_value()) {
							filterForm = a_dataHandler->LookupForm(formID, modName.value());
						} else {
							filterForm = RE::TESForm::LookupByID(formID);
						}
						if (filterForm) {
							const auto formType = filterForm->GetFormType();
							if (const auto type = lookup_form_type(formType); !type.empty()) {
								a_formVec.push_back(filterForm);
							} else {
								logger::error("			Filter [0x{:X}] ({}) SKIP - invalid formtype ({})", formID, modName.has_value() ? modName.value() : "", formType);
							}
						} else {
							logger::error("			Filter [0x{:X}] ({}) SKIP - form doesn't exist", formID, modName.has_value() ? modName.value() : "");
						}					
					}					
				}
			}
		}
	}

	inline void get_forms(RE::TESDataHandler* a_dataHandler, const INIDataVec& a_INIDataVec, KeywordDataVec& a_keywordDataVec)
	{
		if (a_INIDataVec.empty()) {
			return;
		}

		logger::info("	Starting lookup");

		for (auto& [formIDPair_ini, strings_ini, filterIDs_ini, traits_ini, chance_ini] : a_INIDataVec) {
			RE::BGSKeyword* keyword = nullptr;

			if (std::holds_alternative<FormIDPair>(formIDPair_ini)) {
				auto [optFormID, modName] = std::get<FormIDPair>(formIDPair_ini);
				if (optFormID.has_value()) {
					auto formID = optFormID.value();
					if (modName.has_value()) {
						keyword = a_dataHandler->LookupForm<RE::BGSKeyword>(formID, modName.value());
					} else {
						keyword = RE::TESForm::LookupByID<RE::BGSKeyword>(formID);
					}
					if (!keyword) {
						logger::error("		Keyword [0x{:X}] ({}) doesn't exist", formID, modName.has_value() ? modName.value() : "");
						continue;
					}
				}
			} else if (std::holds_alternative<std::string>(formIDPair_ini)) {
				if (auto keywordName = std::get<std::string>(formIDPair_ini); !keywordName.empty()) {
					auto& keywordArray = a_dataHandler->GetFormArray<RE::BGSKeyword>();

					auto result = std::ranges::find_if(keywordArray, [&](const auto& kywd) {
						return kywd && string::iequals(kywd->formEditorID, keywordName);
					});

					if (result != keywordArray.end()) {
						if (keyword = *result; keyword) {
							if (!keyword->IsDynamicForm()) {
								logger::info("		{} [0x{:X}] INFO - using existing keyword", keywordName, keyword->GetFormID());
							}
						} else {
							logger::critical("		{} FAIL - couldn't get existing keyword", keywordName);
							continue;
						}
					} else {
						const auto factory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSKeyword>();
						if (keyword = factory ? factory->Create() : nullptr; keyword) {
							keyword->formEditorID = keywordName;
							logger::info("		{} [0x{:X}] INFO - creating keyword", keywordName, keyword->GetFormID());

							keywordArray.push_back(keyword);
						} else {
							logger::critical("		{} FAIL - couldn't create keyword", keywordName);
							continue;
						}
					}
				}
			}

			std::array<FormVec, 3> filterForms;
			for (std::uint32_t i = 0; i < 3; i++) {
				detail::formID_to_form(a_dataHandler, filterIDs_ini[i], filterForms[i]);
			}

			std::uint32_t count = 0;
			KeywordData keywordData = { keyword, strings_ini, filterForms, traits_ini, chance_ini, count };
			a_keywordDataVec.emplace_back(keywordData);
		}
	}

	bool GetForms();
}

namespace Filter
{
	namespace detail
	{
		namespace name
		{
			inline bool contains(const std::string& a_name, const StringVec& a_strings)
			{
				return std::ranges::any_of(a_strings, [&](const auto& str) {
					return string::icontains(a_name, str);
				});
			}

			inline bool matches(const std::string& a_name, const StringVec& a_strings)
			{
				return std::ranges::any_of(a_strings, [&](const auto& str) {
					return string::iequals(a_name, str);
				});
			}
		}

		namespace form
		{
			inline bool has_effect(RE::MagicItem* a_item, RE::EffectSetting* a_mgef)
			{
				return std::ranges::any_of(a_item->effects, [&](const auto& effect) {
					return effect && effect->baseEffect == a_mgef;
				});
			}

			inline bool get_type(RE::TESForm* a_item, RE::TESForm* a_filter)
			{
				switch (a_filter->GetFormType()) {
				case RE::FormType::Armor:
				case RE::FormType::Weapon:
				case RE::FormType::Ammo:
				case RE::FormType::AlchemyItem:
				case RE::FormType::Scroll:
				case RE::FormType::Location:
					{
						return a_item->GetFormID() == a_filter->GetFormID();
					}
					break;
				case RE::FormType::MagicEffect:
					{
						if (const auto spell = a_item->As<RE::MagicItem>(); spell) {
							const auto mgef = static_cast<RE::EffectSetting*>(a_filter);
							return mgef && has_effect(spell, mgef);
						}
						return a_item->GetFormID() == a_filter->GetFormID();
					}
					break;
				case RE::FormType::EffectShader:
					if (const auto mgef = static_cast<RE::EffectSetting*>(a_item); mgef && (mgef->data.effectShader == a_filter || mgef->data.enchantShader == a_filter)) {
						return true;
					}
					break;
				case RE::FormType::ReferenceEffect:
					if (const auto mgef = static_cast<RE::EffectSetting*>(a_item); mgef && (mgef->data.hitVisuals == a_filter || mgef->data.enchantVisuals == a_filter)) {
						return true;
					}
					break;
				case RE::FormType::ArtObject:
					if (const auto mgef = static_cast<RE::EffectSetting*>(a_item); mgef && (mgef->data.castingArt == a_filter || mgef->data.hitEffectArt == a_filter || mgef->data.enchantEffectArt == a_filter)) {
						return true;
					}
					break;
				case RE::FormType::MusicType:
					if (const auto loc = static_cast<RE::BGSLocation*>(a_item); loc && loc->musicType == a_filter) {
						return true;
					}
					break;
				case RE::FormType::Faction:
					if (const auto loc = static_cast<RE::BGSLocation*>(a_item); loc && loc->unreportedCrimeFaction == a_filter) {
						return true;
					}
					break;
				default:
					break;
				}

				return false;
			}

			inline bool matches(RE::TESForm& a_item, const FormVec& a_forms)
			{
				return std::ranges::any_of(a_forms, [&a_item](const auto& a_formFile) {
					if (std::holds_alternative<RE::TESForm*>(a_formFile)) {
						auto form = std::get<RE::TESForm*>(a_formFile);
						return form && get_type(&a_item, form);
					} else if (std::holds_alternative<const RE::TESFile*>(a_formFile)) {
						auto file = std::get<const RE::TESFile*>(a_formFile);
						return file && file->IsFormInMod(a_item.GetFormID());
					}
					return false;
				});
			}

			inline bool matches_ALL(RE::TESForm& a_item, const FormVec& a_forms)
			{
				return std::ranges::all_of(a_forms, [&a_item](const auto& a_formFile) {
					if (std::holds_alternative<RE::TESForm*>(a_formFile)) {
						auto form = std::get<RE::TESForm*>(a_formFile);
						return form && get_type(&a_item, form);
					} else if (std::holds_alternative<const RE::TESFile*>(a_formFile)) {
						auto file = std::get<const RE::TESFile*>(a_formFile);
						return file && file->IsFormInMod(a_item.GetFormID());
					}
					return false;
				});
			}
		}

		namespace keyword
		{
			inline bool contains(RE::TESForm& a_item, const StringVec& a_strings)
			{
				return std::ranges::any_of(a_strings, [&a_item](const auto& str) {
					auto keywordForm = a_item.As<RE::BGSKeywordForm>();
					return keywordForm && keywordForm->ContainsKeywordString(str);
				});
			}

			inline bool matches(RE::TESForm& a_item, const StringVec& a_strings, bool a_matchesAll = false)
			{
				if (a_matchesAll) {
					return std::ranges::all_of(a_strings, [&a_item](const auto& str) {
						auto keywordForm = a_item.As<RE::BGSKeywordForm>();
						return keywordForm && keywordForm->HasKeywordString(str);
					});
				}
				return std::ranges::any_of(a_strings, [&a_item](const auto& str) {
					auto keywordForm = a_item.As<RE::BGSKeywordForm>();
					return keywordForm && keywordForm->HasKeywordString(str);
				});
			}
		}

		namespace archetype
		{
			using Archetype = RE::EffectArchetypes::ArchetypeID;
			inline constexpr frozen::map<Archetype, std::string_view, 47> archetypeMap = {
				{ Archetype::kNone, "None"sv },				
				{ Archetype::kValueModifier, "ValueMod"sv },
				{ Archetype::kScript, "Script"sv },
				{ Archetype::kDispel, "Dispel"sv },
				{ Archetype::kCureDisease, "CureDisease"sv },
				{ Archetype::kAbsorb, "Absorb"sv },
				{ Archetype::kDualValueModifier, "DualValueMod"sv },
				{ Archetype::kCalm, "Calm"sv },
				{ Archetype::kDemoralize, "Demoralize"sv },
				{ Archetype::kFrenzy, "Frenzy"sv },
				{ Archetype::kDisarm, "Disarm"sv },
				{ Archetype::kCommandSummoned, "CommandSummoned"sv },
				{ Archetype::kInvisibility, "Invisibility"sv },
				{ Archetype::kLight, "Light"sv },
				{ Archetype::kDarkness, "Darkness"sv },
				{ Archetype::kNightEye, "NightEye"sv },
				{ Archetype::kLock, "Lock"sv },
				{ Archetype::kOpen, "Open"sv },
				{ Archetype::kBoundWeapon, "BoundWeapon"sv },
				{ Archetype::kSummonCreature, "SummonCreature"sv },
				{ Archetype::kDetectLife, "DetectLife"sv },
				{ Archetype::kTelekinesis, "Telekinesis"sv },
				{ Archetype::kParalysis, "Paralysis"sv },
				{ Archetype::kReanimate, "Reanimate"sv },
				{ Archetype::kSoulTrap, "SoulTrap"sv },
				{ Archetype::kTurnUndead, "TurnUndead"sv },
				{ Archetype::kGuide, "Guide"sv },
				{ Archetype::kWerewolfFeed, "WerewolfFeed"sv },
				{ Archetype::kCureParalysis, "CureParalysis"sv },
				{ Archetype::kCureAddiction, "CureAddiction"sv },
				{ Archetype::kCurePoison, "CurePoison"sv },
				{ Archetype::kConcussion, "Concussion"sv },
				{ Archetype::kValueAndParts, "ValueAndParts"sv },
				{ Archetype::kAccumulateMagnitude, "AccumulateMagnitude"sv },
				{ Archetype::kStagger, "Stagger"sv },
				{ Archetype::kPeakValueModifier, "PeakValueMod"sv },
				{ Archetype::kCloak, "Cloak"sv },
				{ Archetype::kWerewolf, "Werewolf"sv },
				{ Archetype::kSlowTime, "SlowTime"sv },
				{ Archetype::kRally, "Rally"sv },
				{ Archetype::kEnhanceWeapon, "EnhanceWeapon"sv },
				{ Archetype::kSpawnHazard, "SpawnHazard"sv },
				{ Archetype::kEtherealize, "Etherealize"sv },
				{ Archetype::kBanish, "Banish"sv },
				{ Archetype::kSpawnScriptedRef, "SpawnScriptedRef"sv },
				{ Archetype::kDisguise, "Disguise"sv },
				{ Archetype::kGrabActor, "GrabActor"sv },
				{ Archetype::kVampireLord, "VampireLord"sv },
			};

			inline bool matches(Archetype a_archetype, const StringVec& a_strings)
			{
				auto archetypeStr = archetypeMap.at(a_archetype);
				return std::ranges::any_of(a_strings, [&](const auto& str) {
					return string::iequals(archetypeStr, str);
				});
			}
		}
	}

	template <class T>
	bool strings(T& a_item, const KeywordData& a_keywordData)
	{
		auto& [strings_ALL, strings_NOT, strings_MATCH, strings_ANY] = std::get<DATA_TYPE::kStrings>(a_keywordData);

		if (!strings_ALL.empty() && !detail::keyword::matches(a_item, strings_ALL, true)) {
			return false;
		}

		const std::string name = a_item.GetName();

		if (!strings_NOT.empty()) {
			bool result = false;
			if (!name.empty() && detail::name::matches(name, strings_NOT)) {
				result = true;
			}
			if (!result && detail::keyword::matches(a_item, strings_NOT)) {
				result = true;
			}
			if constexpr (std::is_same_v<T, RE::EffectSetting>) {
				if (!result && detail::archetype::matches(a_item.GetArchetype(), strings_NOT)) {
					result = true;
				}
			}
			if (result) {
				return false;
			}
		}
		if (!strings_MATCH.empty()) {
			bool result = false;
			if (!name.empty() && detail::name::matches(name, strings_MATCH)) {
				result = true;
			}
			if (!result && detail::keyword::matches(a_item, strings_MATCH)) {
				result = true;
			}
			if constexpr (std::is_same_v<T, RE::EffectSetting>) {
				if (!result && detail::archetype::matches(a_item.GetArchetype(), strings_MATCH)) {
					result = true;
				}
			}
			if (!result) {
				return false;
			}
		}
		if (!strings_ANY.empty()) {
			bool result = false;
			if (!name.empty() && detail::name::contains(name, strings_ANY)) {
				result = true;
			}
			if (!result && detail::keyword::contains(a_item, strings_ANY)) {
				result = true;
			}
			if (!result) {
				return false;
			}
		}

		return true;
	}

	inline bool forms(RE::TESForm& a_item, const KeywordData& a_keywordData)
	{
		auto& [filterForms_ALL, filterForms_NOT, filterForms_MATCH] = std::get<DATA_TYPE::kFilters>(a_keywordData);

		if (!filterForms_ALL.empty() && !detail::form::matches_ALL(a_item, filterForms_ALL)) {
			return false;
		}

		if (!filterForms_NOT.empty() && detail::form::matches(a_item, filterForms_NOT)) {
			return false;
		}

		if (!filterForms_MATCH.empty() && !detail::form::matches(a_item, filterForms_MATCH)) {
			return false;
		}

		auto chance = std::get<DATA_TYPE::kChance>(a_keywordData);
		if (!numeric::essentially_equal(chance, 100.0)) {
			const auto rng = RNG::GetSingleton()->Generate<float>(0.0, 100.0);
			if (rng > chance) {
				return false;
			}
		}

		return true;
	}

	template <class T>
	bool secondary(T& a_item, const KeywordData& a_keywordData)
	{
		const auto traits = std::get<DATA_TYPE::kTraits>(a_keywordData);

		if constexpr (std::is_same_v<T, RE::TESObjectARMO>) {
			const auto& [enchanted, templated, ARValue] = std::get<ITEM::kArmor>(traits);
			if (enchanted.has_value() && (a_item.formEnchanting != nullptr) != enchanted.value()) {
				return false;
			}
			if (templated.has_value() && (a_item.templateArmor != nullptr) != templated.value()) {
				return false;
			}
			if (ARValue.has_value()) {
				auto [min, max] = ARValue.value();
				auto AR = a_item.GetArmorRating();

				if (min < RE::NI_INFINITY && max < RE::NI_INFINITY) {
					if (AR < min || AR > max) {
						return false;
					}
				} else if (min < RE::NI_INFINITY && AR < min) {
					return false;
				} else if (max < RE::NI_INFINITY && AR > max) {
					return false;
				}
			}
		} else if constexpr (std::is_same_v<T, RE::TESObjectWEAP>) {
			const auto& [enchanted, templated, weightValue] = std::get<ITEM::kWeapon>(traits);
			if (enchanted.has_value() && (a_item.formEnchanting != nullptr) != enchanted.value()) {
				return false;
			}
			if (templated.has_value() && (a_item.templateWeapon != nullptr) != templated.value()) {
				return false;
			}
			if (weightValue.has_value()) {
				auto [min, max] = weightValue.value();
				auto weight = a_item.weight;

				if (min < RE::NI_INFINITY && max < RE::NI_INFINITY) {
					if (weight < min || weight > max) {
						return false;
					}
				} else if (min < RE::NI_INFINITY && weight < min) {
					return false;
				} else if (max < RE::NI_INFINITY && weight > max) {
					return false;
				}
			}
		} else if constexpr (std::is_same_v<T, RE::TESAmmo>) {
			const auto isBolt = std::get<ITEM::kAmmo>(traits);
			if (isBolt.has_value() && a_item.IsBolt() != isBolt.value()) {
				return false;
			}
		} else if constexpr (std::is_same_v<T, RE::EffectSetting>) {
			const auto& [isHostile, castingType, deliveryType, skillValue] = std::get<ITEM::kMagicEffect>(traits);
			if (isHostile.has_value() && a_item.IsHostile() != isHostile.value()) {
				return false;
			}
			if (castingType.has_value() && a_item.data.castingType != castingType.value()) {
				return false;
			}
			if (deliveryType.has_value() && a_item.data.delivery != deliveryType.value()) {
				return false;
			}
			if (skillValue.has_value()) {
				auto [skill, minMax] = skillValue.value();
				auto [min, max] = minMax;

				if (skill != a_item.GetMagickSkill()) {
					return false;
				}

				auto minSkill = a_item.GetMinimumSkillLevel();

				if (min < INT_MAX && max < INT_MAX) {
					if (minSkill < min || minSkill > max) {
						return false;
					}
				} else if (min < INT_MAX && minSkill < min) {
					return false;
				} else if (max < INT_MAX && minSkill > max) {
					return false;
				}
			}
		} else if constexpr (std::is_same_v<T, RE::AlchemyItem>) {
			const auto& [isPoison, isFood] = std::get<ITEM::kPotion>(traits);
			if (isPoison.has_value() && a_item.IsPoison() != isPoison.value()) {
				return false;
			}
			if (isFood.has_value() && a_item.IsFood() != isFood.value()) {
				return false;
			}
		}

		auto chance = std::get<DATA_TYPE::kChance>(a_keywordData);
		if (!numeric::essentially_equal(chance, 100.0)) {
			const auto rng = RNG::GetSingleton()->Generate<float>(0.0, 100.0);
			if (rng > chance) {
				return false;
			}
		}

		return true;
	}
}

namespace Distribute
{
	template <class T>
	inline void add_keyword(T& a_item, KeywordDataVec& a_keywordDataVec)
	{
		for (auto& keywordData : a_keywordDataVec) {
			if (!Filter::strings(a_item, keywordData) || !Filter::forms(a_item, keywordData) || !Filter::secondary(a_item, keywordData)) {
				continue;
			}
			logger::info("{}", a_item.GetName());
			if (const auto keyword = std::get<DATA_TYPE::kForm>(keywordData); keyword) {
				if (const auto keywordForm = a_item.As<RE::BGSKeywordForm>(); keywordForm && keywordForm->AddKeyword(keyword)) {
					++std::get<DATA_TYPE::kCount>(keywordData);
				}
			}
		}
	}

	void AddKeywords();
}
