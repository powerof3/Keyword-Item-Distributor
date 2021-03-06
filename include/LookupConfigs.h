#pragma once

#include "Cache.h"

namespace Lookup::Config
{
	namespace detail
	{
		inline std::vector<std::string> split_sub_string(const std::string& a_str, const std::string& a_delimiter = ",")
		{
			return !a_str.empty() && a_str.find("NONE"sv) == std::string::npos ?
                       string::split(a_str, a_delimiter) :
                       std::vector<std::string>();
		}

		inline FORMID_TYPE get_formID_type(const std::string& a_str)
		{
			if (a_str.find("~"sv) != std::string::npos) {
				return kFormIDMod;
			}
			if (Lookup::detail::is_mod_name(a_str)) {
				return kMod;
			}
			return string::is_only_hex(a_str) ? kFormID : kEditorID;
		}

		inline FormIDPair get_formID(const FORMID_TYPE a_type, const std::string& a_str)
		{
			switch (a_type) {
			case kFormIDMod:
				{
					auto splitID = string::split(a_str, "~");
					return std::make_pair(
						string::lexical_cast<RE::FormID>(splitID.at(CONFIG::kFormID), true),
						splitID.at(CONFIG::kESP));
				}
			case kMod:
			case kEditorID:
				return std::make_pair(
					std::nullopt,
					a_str);
			case kFormID:
				return std::make_pair(
					string::lexical_cast<RE::FormID>(a_str, true),
					std::nullopt);
			default:
				return FormIDPair{};
			}
		}

		inline FormIDPair get_formID(const std::string& a_str)
		{
			return get_formID(
				get_formID_type(a_str),
				a_str);
		}

		template <class T>
		std::pair<T, T> get_minmax_values(std::string& a_str)
		{
			std::pair<T, T> minMax{ std::numeric_limits<T>::max(), std::numeric_limits<T>::max() };

			if (const auto values = string::split(string::remove_non_numeric(a_str), " "); !values.empty()) {
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
			if (const auto values = string::split(string::remove_non_numeric(a_str), " "); !values.empty()) {
				return string::lexical_cast<T>(values.at(0));
			}
			return std::nullopt;
		}
	}

	inline std::pair<INIData, ITEM::TYPE> parse_config(const std::string& a_value)
	{
		INIData data;
		auto& [formIDPair_ini, strings_ini, filterIDs_ini, traits_ini, chance_ini] = data;

		auto sections = string::split(a_value, "|");

		//[FORMID/ESP] / string
		std::variant<FormIDPair, std::string> item_ID;
		try {
			auto& formSection = sections.at(CONFIG::kFormID);
			if (const auto type = detail::get_formID_type(formSection); type != kEditorID) {
				item_ID.emplace<FormIDPair>(detail::get_formID(type, formSection));
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
			const auto& typeStr = sections.at(CONFIG::kType);
			if (!typeStr.empty()) {
			    type = Cache::Item::GetType(typeStr);
			}
		} catch (...) {
		}

		//FILTERS
		try {
			auto& [strings_ALL, strings_NOT, strings_MATCH, strings_ANY] = strings_ini;
			auto& [filterIDs_ALL, filterIDs_NOT, filterIDs_MATCH] = filterIDs_ini;

			auto split_str = detail::split_sub_string(sections.at(CONFIG::kFilters));
			for (auto& str : split_str) {
				if (str.find("+"sv) != std::string::npos) {
					auto strings = detail::split_sub_string(str, "+");

					std::ranges::copy_if(strings, std::back_inserter(strings_ALL), [](const auto& string_str) {
						return detail::get_formID_type(string_str) == kEditorID;
					});
					std::ranges::transform(strings, std::back_inserter(filterIDs_ALL), [](const auto& filter_str) {
						return detail::get_formID(filter_str);
					});
				} else if (str.at(0) == '-') {
					str.erase(0, 1);

					if (detail::get_formID_type(str) == kEditorID) {
						strings_NOT.emplace_back(str);
					}
					filterIDs_NOT.emplace_back(detail::get_formID(str));

				} else if (str.at(0) == '*') {
					str.erase(0, 1);
					strings_ANY.emplace_back(str);

				} else {
					if (detail::get_formID_type(str) == kEditorID) {
						strings_MATCH.emplace_back(str);
					}
					filterIDs_MATCH.emplace_back(detail::get_formID(str));
				}
			}
		} catch (...) {
		}

		//TRAITS
		try {
			for (auto split_str = detail::split_sub_string(sections.at(CONFIG::kTraits)); auto& str : split_str) {
				switch (type) {
				case ITEM::kArmor:
					{
						auto& [enchanted, templated, armorRating, armorType] = std::get<TRAITS::kArmor>(traits_ini);
						if (str.contains("AR")) {
							armorRating = detail::get_minmax_values<float>(str);
						} else if (str == "HEAVY") {
							armorType = RE::BIPED_MODEL::ArmorType::kHeavyArmor;
						} else if (str == "LIGHT") {
							armorType = RE::BIPED_MODEL::ArmorType::kLightArmor;
						} else if (str == "CLOTHING") {
							armorType = RE::BIPED_MODEL::ArmorType::kClothing;
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
						if (str.contains('W')) {
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
						if (str.contains('D')) {
							deliveryType = detail::get_single_value<RE::MagicSystem::Delivery>(str);
						} else if (str.contains("CT")) {
							castingType = detail::get_single_value<RE::MagicSystem::CastingType>(str);
						} else if (str.contains('(')) {
                            if (auto value = string::split(string::remove_non_numeric(str), " "); !value.empty()) {
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
			const auto& chanceStr = sections.at(CONFIG::kChance);
			if (!chanceStr.empty() && chanceStr.find("NONE"sv) == std::string::npos) {
				chance_ini = string::lexical_cast<float>(chanceStr);
			}
		} catch (...) {
		}

		return std::make_pair(data, type);
	}

	bool Read();
}
