#pragma once

#include "Defs.h"

#include <array>
#include <iosfwd>
#include <vector>

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
			}
			return std::make_pair(
				string::lexical_cast<RE::FormID>(a_str, true),
				std::nullopt);
		}
	}

	inline std::pair<INIData, ITEM::TYPE> parse_ini(const std::string& a_value)
	{
		INIData data;
		auto& [formIDPair_ini, strings_ini, filterIDs_ini, chance_ini] = data;

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
				if (str.find('~') != std::string::npos || string::is_only_hex(str)) {
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
			default:
				return ""sv;
			}
		};

		inline bool formID_to_form(RE::TESDataHandler* a_dataHandler, const FormIDPairVec& a_formIDVec, FormVec& a_formVec)
		{
			if (!a_formIDVec.empty()) {
				for (auto& [formID, modName] : a_formIDVec) {
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
							logger::error("			Filter [0x{:X}] ({}) FAIL - invalid formtype ({})", formID, modName.has_value() ? modName.value() : "", formType);
							return false;
						}
					} else {
						logger::error("			Filter [0x{:X}] ({}) FAIL - form doesn't exist", formID, modName.has_value() ? modName.value() : "");
						return false;
					}
				}
			}
			return true;
		}
	}

	inline void get_forms(RE::TESDataHandler* a_dataHandler, const INIDataVec& a_INIDataVec, KeywordDataVec& a_keywordDataVec)
	{
		if (a_INIDataVec.empty()) {
			return;
		}

		logger::info("	Starting lookup");

		for (auto& [formIDPair_ini, strings_ini, filterIDs_ini, chance_ini] : a_INIDataVec) {
			RE::BGSKeyword* keyword = nullptr;

			if (std::holds_alternative<FormIDPair>(formIDPair_ini)) {
				auto [formID, modName] = std::get<FormIDPair>(formIDPair_ini);
				if (modName.has_value()) {
					keyword = a_dataHandler->LookupForm<RE::BGSKeyword>(formID, modName.value());
				} else {
					keyword = RE::TESForm::LookupByID<RE::BGSKeyword>(formID);
				}
				if (!keyword) {
					logger::error("		Keyword [0x{:X}] ({}) doesn't exist", formID, modName.has_value() ? modName.value() : "");
					continue;
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
			bool result = true;
			for (std::uint32_t i = 0; i < 3; i++) {
				if (!detail::formID_to_form(a_dataHandler, filterIDs_ini[i], filterForms[i])) {
					result = false;
					break;
				}
			}
			if (!result) {
				continue;
			}

			std::uint32_t count = 0;
			KeywordData keywordData = { keyword, strings_ini, filterForms, chance_ini, count };
			a_keywordDataVec.emplace_back(keywordData);
		}
	}

	bool GetForms();
}

namespace Distribute
{
	namespace filter
	{
		namespace name
		{
			inline bool contains(const std::string& a_name, const StringVec& a_strings)
			{
				return std::ranges::any_of(a_strings, [&](const auto& str) {
					return string::icontains(str, a_name);
				});
			}

			inline bool matches(const std::string& a_name, const StringVec& a_strings)
			{
				return std::ranges::any_of(a_strings, [&](const auto& str) {
					return string::iequals(str, a_name);
				});
			}
		}

		namespace form
		{
			inline bool matches(RE::TESForm& a_item, const FormVec& a_forms)
			{
				return std::ranges::any_of(a_forms, [&a_item](const auto& a_form) {
					return a_form && a_form->GetFormID() == a_item.GetFormID();
				});
			}

			inline bool matches_ALL(RE::TESForm& a_item, const FormVec& a_forms)
			{
				return std::ranges::all_of(a_forms, [&a_item](const auto& a_form) {
					return a_form && a_form->GetFormID() == a_item.GetFormID();
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

		inline bool strings(RE::TESForm& a_item, const KeywordData& a_keywordData)
		{
			auto& [strings_ALL, strings_NOT, strings_MATCH, strings_ANY] = std::get<DATA_TYPE::kStrings>(a_keywordData);

			if (!strings_ALL.empty() && !keyword::matches(a_item, strings_ALL, true)) {
				return false;
			}

			const std::string name = a_item.GetName();

			if (!strings_NOT.empty()) {
				bool result = false;
				if (!name.empty() && name::matches(name, strings_NOT)) {
					result = true;
				}
				if (!result && keyword::matches(a_item, strings_NOT)) {
					result = true;
				}
				if (result) {
					return false;
				}
			}
			if (!strings_MATCH.empty()) {
				bool result = false;
				if (!name.empty() && name::matches(name, strings_MATCH)) {
					result = true;
				}
				if (!result && keyword::matches(a_item, strings_MATCH)) {
					result = true;
				}
				if (!result) {
					return false;
				}
			}
			if (!strings_ANY.empty()) {
				bool result = false;
				if (!name.empty() && name::contains(name, strings_ANY)) {
					result = true;
				}
				if (!result && keyword::contains(a_item, strings_ANY)) {
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

			if (!filterForms_ALL.empty() && !form::matches_ALL(a_item, filterForms_ALL)) {
				return false;
			}

			if (!filterForms_NOT.empty() && form::matches(a_item, filterForms_NOT)) {
				return false;
			}

			if (!filterForms_MATCH.empty() && !form::matches(a_item, filterForms_MATCH)) {
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
	}

	inline void add_keyword(RE::TESForm& a_item, KeywordDataVec& a_keywordDataVec)
	{
		for (auto& keywordData : a_keywordDataVec) {
			if (!filter::strings(a_item, keywordData) || !filter::forms(a_item, keywordData)) {
				continue;
			}
			if (const auto keyword = std::get<DATA_TYPE::kForm>(keywordData); keyword) {
				if (const auto keywordForm = a_item.As<RE::BGSKeywordForm>(); keywordForm && keywordForm->AddKeyword(keyword)) {
					++std::get<DATA_TYPE::kCount>(keywordData);
				}
			}
		}
	}

	inline void list_keyword_count(const std::string& a_type, const KeywordDataVec& a_formDataVec, const size_t a_totalCount)
	{
		for (auto& formData : a_formDataVec) {
			auto keyword = std::get<DATA_TYPE::kForm>(formData);
			auto count = std::get<DATA_TYPE::kCount>(formData);

			if (keyword) {
				logger::info("{} [0x{:X}] added to {}/{} {}", keyword->GetFormEditorID(), keyword->GetFormID(), count, a_totalCount, a_type);
			}
		}
	}

	void AddKeywords();
}
