#pragma once

#include "Cache.h"

namespace Lookup::Config
{
	namespace detail
	{
		template <class T>
		std::pair<T, T> get_minmax_values(std::string& a_str)
		{
			std::pair<T, T> minMax{ std::numeric_limits<T>::max(), std::numeric_limits<T>::max() };

			if (const auto values = string::split(string::remove_non_numeric(a_str), " "); !values.empty()) {
				if (values.size() > 1) {
					minMax.first = string::to_num<T>(values.at(0));
					minMax.second = string::to_num<T>(values.at(1));
				} else {
					minMax.first = string::to_num<T>(values.at(0));
				}
			}

			return minMax;
		}

		template <class T>
		std::optional<T> get_single_value(std::string& a_str)
		{
			if (const auto values = string::split(string::remove_non_numeric(a_str), " "); !values.empty()) {
				return string::to_num<T>(values.at(0));
			}
			return std::nullopt;
		}
	}

	inline std::pair<INIData, ITEM::TYPE> parse_config(std::string& a_value, const std::string& a_path)
	{
#ifdef SKYRIMVR
		// swap dawnguard and dragonborn forms
		// VR apparently does not load masters in order so the lookup fails
		static const srell::regex re_dawnguard(R"((0x0*2)([0-9a-f]{6}))", srell::regex_constants::optimize | srell::regex::icase);
		a_value = regex_replace(a_value, re_dawnguard, "0x$2~Dawnguard.esm");

		static const srell::regex re_dragonborn(R"((0x0*4)([0-9a-f]{6}))", srell::regex_constants::optimize | srell::regex::icase);
		a_value = regex_replace(a_value, re_dragonborn, "0x$2~Dragonborn.esm");
#endif

		INIData data;
		auto& [keywordID_ini, strings_ini, filterIDs_ini, traits_ini, chance_ini, path] = data;
		path = a_path;

		const auto sections = string::split(a_value, "|");
		const auto size = sections.size();

		//[FORMID/ESP] / string
		if (CONFIG::kFormID < size) {
			keywordID_ini = distribution::get_record(sections[CONFIG::kFormID]);
		}

		//TYPE
		ITEM::TYPE type = ITEM::kNone;
		if (CONFIG::kType < size) {
			if (const auto& typeStr = sections[CONFIG::kType]; !typeStr.empty()) {
				type = Cache::Item::GetType(typeStr);
			}
		}

		//FILTERS
		if (CONFIG::kFilters < size) {
			auto& [strings_ALL, strings_NOT, strings_MATCH, strings_ANY] = strings_ini;
			auto& [filterIDs_ALL, filterIDs_NOT, filterIDs_MATCH] = filterIDs_ini;

			auto split_str = distribution::split_entry(sections[CONFIG::kFilters]);
			for (auto& str : split_str) {
				if (str.find("+"sv) != std::string::npos) {
					auto strings = distribution::split_entry(str, "+");

					std::ranges::copy_if(strings, std::back_inserter(strings_ALL), [](const auto& string_str) {
						return distribution::get_record_type(string_str) == FORMID_TYPE::kEditorID;
					});
					std::ranges::transform(strings, std::back_inserter(filterIDs_ALL), [](const auto& filter_str) {
						return distribution::get_record(filter_str);
					});
				} else if (str.at(0) == '-') {
					str.erase(0, 1);

					if (distribution::get_record_type(str) == FORMID_TYPE::kEditorID) {
						strings_NOT.emplace_back(str);
					}
					filterIDs_NOT.emplace_back(distribution::get_record(str));

				} else if (str.at(0) == '*') {
					str.erase(0, 1);
					strings_ANY.emplace_back(str);

				} else {
					if (distribution::get_record_type(str) == FORMID_TYPE::kEditorID) {
						strings_MATCH.emplace_back(str);
					}
					filterIDs_MATCH.emplace_back(distribution::get_record(str));
				}
			}
		}

		//TRAITS
		if (CONFIG::kTraits < size) {
			auto split_str = distribution::split_entry(sections[CONFIG::kTraits]);
			for (auto& str : split_str) {
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
								auto skill = string::to_num<RE::ActorValue>(value.at(0));
								auto min = string::to_num<std::int32_t>(value.at(1));
								if (value.size() > 2) {
									auto max = string::to_num<std::int32_t>(value.at(2));
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
							av = string::to_num<RE::ActorValue>(str);
						}
					}
					break;
				case ITEM::kSoulGem:
					{
						auto& [black, soulSize, gemSize] = std::get<TRAITS::kSoulGem>(traits_ini);
						if (str == "BLACK") {
							black = true;
						} else if (str.contains("SOUL")) {
							soulSize = detail::get_single_value<RE::SOUL_LEVEL>(str);
						} else {  // GEM
							gemSize = detail::get_single_value<RE::SOUL_LEVEL>(str);
						}
					}
					break;
				default:
					break;
				}
			}
		}

		//CHANCE
		chance_ini = 100.0f;
		if (CONFIG::kChance < size) {
			const auto& chanceStr = sections[CONFIG::kChance];
			if (distribution::is_valid_entry(chanceStr)) {
				chance_ini = string::to_num<float>(chanceStr);
			}
		}

		return std::make_pair(data, type);
	}

	std::pair<bool, bool> Read();
}
