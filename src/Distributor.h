#pragma once

#include "Cache.h"

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

		inline bool is_mod_name(const std::string& a_str)
		{
			return a_str.rfind(".esp") != std::string::npos || a_str.rfind(".esl") != std::string::npos || a_str.rfind(".esm ") != std::string::npos;
		}

		inline FormIDPair get_formID(const std::string& a_str)
		{
			if (a_str.find("~"sv) != std::string::npos) {
				auto splitID = string::split(a_str, "~");
				return std::make_pair(
					string::lexical_cast<RE::FormID>(splitID.at(kFormID), true),
					splitID.at(kESP));
			} else if (is_mod_name(a_str) || !string::is_only_hex(a_str)) {
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
			auto& formSection = sections.at(kFormID);
			if (formSection.find('~') != std::string::npos || string::is_only_hex(formSection)) {
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
		ITEM::TYPE type = ITEM::kNone;
		try {
			const auto& typeStr = sections.at(kType);
			if (!typeStr.empty()) {
				type = Cache::Item::GetType(typeStr);
			}
		} catch (...) {
		}

		//FILTERS
		try {
			auto& [strings_ALL, strings_NOT, strings_MATCH, strings_ANY] = strings_ini;
			auto& [filterIDs_ALL, filterIDs_NOT, filterIDs_MATCH] = filterIDs_ini;

			auto split_str = detail::split_sub_string(sections.at(kFilters));
			for (auto& str : split_str) {
				if (str.find("+"sv) != std::string::npos) {
					auto strings = detail::split_sub_string(str, "+");

					strings_ALL.insert(strings_ALL.end(), strings.begin(), strings.end());

					std::transform(strings.begin(), strings.end(), std::back_inserter(filterIDs_ALL), [](auto const& str) {
						return detail::get_formID(str);
					});

				} else if (str.at(0) == '-') {
					str.erase(0, 1);

					strings_NOT.emplace_back(str);
					filterIDs_NOT.emplace_back(detail::get_formID(str));

				} else if (str.at(0) == '*') {
					str.erase(0, 1);
					strings_ANY.emplace_back(str);

				} else {
					strings_MATCH.emplace_back(str);
					filterIDs_MATCH.emplace_back(detail::get_formID(str));
				}
			}
		} catch (...) {
		}

		//TRAITS
		traits_ini;
		try {
			auto split_str = detail::split_sub_string(sections.at(kTraits));
			for (auto& str : split_str) {
				switch (type) {
				case ITEM::kArmor:
					{
						auto& [enchanted, templated, armorRating] = std::get<TRAITS::kArmor>(traits_ini);
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
						auto& [enchanted, templated, weight] = std::get<TRAITS::kWeapon>(traits_ini);
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
						auto& isBolt = std::get<TRAITS::kAmmo>(traits_ini);
						if (str == "B") {
							isBolt = true;
						} else if (str == "-B") {
							isBolt = false;
						}
					}
					break;
				case ITEM::kMagicEffect:
					{
						auto& [isHostile, castingType, deliveryType, skillValue] = std::get<TRAITS::kMagicEffect>(traits_ini);
						if (str.find("D") != std::string::npos) {
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
						auto& [isPoison, isFood] = std::get<TRAITS::kPotion>(traits_ini);
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
				case ITEM::kIngredient:
					{
						auto& isFood = std::get<TRAITS::kIngredient>(traits_ini);
						if (str == "F") {
							isFood = true;
						} else if (str == "-F") {
							isFood = false;
						}
					}
					break;
				case ITEM::kBook:
					{
						auto& [spell, skill, av] = std::get<TRAITS::kBook>(traits_ini);
						if (str == "S") {
							spell = true;
						} else if (str == "-S") {
							spell = false;
						} else if (str == "AV") {
							skill = true;
						} else if (str == "-AV") {
							skill = false;
						} else {
							av = string::lexical_cast<RE::ActorValue>(str);
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
			const auto& chanceStr = sections.at(kChance);
			if (!chanceStr.empty() && chanceStr.find("NONE"sv) == std::string::npos) {
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
		inline bool formID_to_form(RE::TESDataHandler* a_dataHandler, const FormIDPairVec& a_formIDVec, FormVec& a_formVec)
		{
			if (a_formIDVec.empty()) {
				return true;
			}
			for (auto& [formID, modName] : a_formIDVec) {
				if (modName && !formID) {
					if (INI::detail::is_mod_name(*modName)) {
						if (const RE::TESFile* filterMod = a_dataHandler->LookupModByName(*modName); filterMod) {
							logger::info("			Filter ({}) INFO - mod found", filterMod->fileName);
							a_formVec.push_back(filterMod);
						} else {
							logger::error("			Filter ({}) SKIP - mod cannot be found", *modName);
						}
					} else {
						auto filterForm = RE::TESForm::LookupByEditorID(*modName);
						if (filterForm) {
							const auto formType = filterForm->GetFormType();
							if (const auto type = Cache::FormType::GetString(formType); !type.empty()) {
								a_formVec.push_back(filterForm);
							} else {
								logger::error("			Filter ({}) SKIP - invalid formtype ({})", *modName, formType);
							}
						} else {
							logger::error("			Filter ({}) SKIP - form doesn't exist", *modName);
						}
					}
				} else if (formID) {
					auto filterForm = modName ?
                                          a_dataHandler->LookupForm(*formID, *modName) :
                                          RE::TESForm::LookupByID(*formID);
					if (filterForm) {
						const auto formType = filterForm->GetFormType();
						if (const auto type = Cache::FormType::GetString(formType); !type.empty()) {
							a_formVec.push_back(filterForm);
						} else {
							logger::error("			Filter [0x{:X}] ({}) SKIP - invalid formtype ({})", *formID, modName.value_or(""), formType);
						}
					} else {
						logger::error("			Filter [0x{:X}] ({}) SKIP - form doesn't exist", *formID, modName.value_or(""));
					}
				}
			}
			return !a_formVec.empty();
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
				if (auto [formID, modName] = std::get<FormIDPair>(formIDPair_ini); formID) {
					keyword = modName ?
                                  a_dataHandler->LookupForm<RE::BGSKeyword>(*formID, *modName) :
                                  RE::TESForm::LookupByID<RE::BGSKeyword>(*formID);
					if (!keyword) {
						logger::error("		Keyword [0x{:X}] ({}) doesn't exist", *formID, modName.value_or(""));
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

			bool invalidEntry = false;

			std::array<FormVec, 3> filterForms;
			for (std::uint32_t i = 0; i < filterForms.size(); i++) {
				if (!detail::formID_to_form(a_dataHandler, filterIDs_ini[i], filterForms[i])) {
					invalidEntry = true;
					break;
				}
			}

			if (invalidEntry) {
				continue;
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
				case RE::FormType::Ingredient:
				case RE::FormType::Book:
					return a_item == a_filter;
				case RE::FormType::Location:
					{
						const auto loc = a_item->As<RE::BGSLocation>();
						const auto filterLoc = a_filter->As<RE::BGSLocation>();
						return loc && filterLoc && (loc == filterLoc || loc->IsChild(filterLoc));
					}
				case RE::FormType::MagicEffect:
					if (const auto spell = a_item->As<RE::MagicItem>(); spell) {
						const auto mgef = static_cast<RE::EffectSetting*>(a_filter);
						return mgef && has_effect(spell, mgef);
					}
					return a_item == a_filter;
				case RE::FormType::EffectShader:
					{
						const auto mgef = a_item->As<RE::EffectSetting>();
						return mgef && (mgef->data.effectShader == a_filter || mgef->data.enchantShader == a_filter);
					}
				case RE::FormType::ReferenceEffect:
					{
						const auto mgef = a_item->As<RE::EffectSetting>();
						return mgef && (mgef->data.hitVisuals == a_filter || mgef->data.enchantVisuals == a_filter);
					}
				case RE::FormType::ArtObject:
					{
						const auto mgef = a_item->As<RE::EffectSetting>();
						return mgef && (mgef->data.castingArt == a_filter || mgef->data.hitEffectArt == a_filter || mgef->data.enchantEffectArt == a_filter);
					}
				case RE::FormType::MusicType:
					{
						const auto loc = a_item->As<RE::BGSLocation>();
						return loc && loc->musicType == a_filter;
					}
				case RE::FormType::Faction:
					{
						const auto loc = a_item->As<RE::BGSLocation>();
						return loc && loc->unreportedCrimeFaction == a_filter;
					}
				case RE::FormType::Spell:
					{
						const auto book = a_item->As<RE::TESObjectBOOK>();
						return book && book->GetSpell() == a_filter;
					}
				default:
					return false;
				}
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

		namespace actorvalue
		{
			inline bool matches(const RE::ActorValue a_skill, const StringVec& a_strings)
			{
				if (a_skill == RE::ActorValue::kNone) {
					return false;
				}

				const auto avInfo = RE::ActorValueList::GetSingleton()->GetActorValue(a_skill);
				if (!avInfo) {
					return false;
				}

				return std::ranges::any_of(a_strings, [&](const auto& str) {
					return string::iequals(avInfo->enumName, str);
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
		const std::string editorID = Cache::EditorID::GetSingleton()->GetEditorID(a_item.GetFormID());

		if (!strings_NOT.empty()) {
			bool result = false;
			if (!name.empty() && detail::name::matches(name, strings_NOT)) {
				result = true;
			}
			if (!result && !editorID.empty() && detail::name::matches(editorID, strings_NOT)) {
				result = true;
			}
			if (!result && detail::keyword::matches(a_item, strings_NOT)) {
				result = true;
			}
			if constexpr (std::is_same_v<T, RE::EffectSetting>) {
				if (!result && Cache::Archetype::Matches(a_item.data.archetype, strings_NOT)) {
					result = true;
				}
			} else if constexpr (std::is_same_v<T, RE::TESObjectBOOK>) {
				if (!result && detail::actorvalue::matches(a_item.GetSkill(), strings_NOT)) {
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
			if (!result && !editorID.empty() && detail::name::matches(editorID, strings_MATCH)) {
				result = true;
			}
			if (!result && detail::keyword::matches(a_item, strings_MATCH)) {
				result = true;
			}
			if constexpr (std::is_same_v<T, RE::EffectSetting>) {
				if (!result && Cache::Archetype::Matches(a_item.data.archetype, strings_MATCH)) {
					result = true;
				}
			} else if constexpr (std::is_same_v<T, RE::TESObjectBOOK>) {
				if (!result && detail::actorvalue::matches(a_item.GetSkill(), strings_MATCH)) {
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
			if (!result && !editorID.empty() && detail::name::contains(editorID, strings_ANY)) {
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

		return true;
	}

	template <class T>
	bool secondary(T& a_item, const KeywordData& a_keywordData)
	{
		const auto& traits = std::get<DATA_TYPE::kTraits>(a_keywordData);

		if constexpr (std::is_same_v<T, RE::TESObjectARMO>) {
			const auto& [enchanted, templated, ARValue] = std::get<TRAITS::kArmor>(traits);
			if (enchanted && (a_item.formEnchanting != nullptr) != *enchanted) {
				return false;
			}
			if (templated && (a_item.templateArmor != nullptr) != *templated) {
				return false;
			}
			if (ARValue) {
				auto& [min, max] = *ARValue;
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
			const auto& [enchanted, templated, weightValue] = std::get<TRAITS::kWeapon>(traits);
			if (enchanted && (a_item.formEnchanting != nullptr) != *enchanted) {
				return false;
			}
			if (templated && (a_item.templateWeapon != nullptr) != *templated) {
				return false;
			}
			if (weightValue) {
				auto& [min, max] = weightValue.value();
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
			const auto& isBolt = std::get<TRAITS::kAmmo>(traits);
			if (isBolt && a_item.IsBolt() != *isBolt) {
				return false;
			}
		} else if constexpr (std::is_same_v<T, RE::EffectSetting>) {
			const auto& [isHostile, castingType, deliveryType, skillValue] = std::get<TRAITS::kMagicEffect>(traits);
			if (isHostile && a_item.IsHostile() != *isHostile) {
				return false;
			}
			if (castingType && a_item.data.castingType != *castingType) {
				return false;
			}
			if (deliveryType && a_item.data.delivery != *deliveryType) {
				return false;
			}
			if (skillValue) {
				auto& [skill, minMax] = *skillValue;
				auto& [min, max] = minMax;

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
			const auto& [isPoison, isFood] = std::get<TRAITS::kPotion>(traits);
			if (isPoison && a_item.IsPoison() != *isPoison) {
				return false;
			}
			if (isFood && a_item.IsFood() != *isFood) {
				return false;
			}
		} else if constexpr (std::is_same_v<T, RE::IngredientItem>) {
			const auto& isFood = std::get<TRAITS::kIngredient>(traits);
			if (isFood && a_item.IsFood() != *isFood) {
				return false;
			}
		} else if constexpr (std::is_same_v<T, RE::TESObjectBOOK>) {
			const auto& [spell, skill, av] = std::get<TRAITS::kBook>(traits);
			if (spell && a_item.TeachesSpell() != *spell) {
				return false;
			}
			if (skill && a_item.TeachesSkill() != *skill) {
				return false;
			}
			if (av && a_item.GetSkill() != *av) {
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
			if (const auto keyword = std::get<DATA_TYPE::kForm>(keywordData); keyword) {
				if (const auto keywordForm = a_item.As<RE::BGSKeywordForm>(); keywordForm && keywordForm->AddKeyword(keyword)) {
					++std::get<DATA_TYPE::kCount>(keywordData);
				}
			}
		}
	}

	void AddKeywords();
}
