#include "LookupConfigs.h"

namespace INI
{
	namespace detail
	{
		inline std::pair<Data, ITEM::TYPE> parse_config(std::string& a_value, const std::string& a_path)
		{
#ifdef SKYRIMVR
			// swap dawnguard and dragonborn forms
			// VR apparently does not load masters in order so the lookup fails
			static const srell::regex re_dawnguard(R"((0x0*2)([0-9a-f]{6}))", srell::regex_constants::optimize | srell::regex::icase);
			a_value = regex_replace(a_value, re_dawnguard, "0x$2~Dawnguard.esm");

			static const srell::regex re_dragonborn(R"((0x0*4)([0-9a-f]{6}))", srell::regex_constants::optimize | srell::regex::icase);
			a_value = regex_replace(a_value, re_dragonborn, "0x$2~Dragonborn.esm");
#endif

			Data       data{};
			ITEM::TYPE type{ ITEM::kNone };

			const auto sections = string::split(a_value, "|");
			const auto size = sections.size();

			//[FORMID/ESP] / string
			if (kFormID < size) {
				data.rawForm = distribution::get_record(sections[kFormID]);
			}

			//TYPE
			if (kType < size) {
				if (const auto& typeStr = sections[kType]; !typeStr.empty()) {
					type = ITEM::GetType(typeStr);
				}
			}

			//FILTERS
			if (kFilters < size) {
				auto filterStr = distribution::split_entry(sections[kFilters]);

				for (auto& filter : filterStr) {
					if (filter.contains('+')) {
						auto ALL = distribution::split_entry(filter, "+");
						std::ranges::transform(ALL, std::back_inserter(data.rawFilters.ALL), [](const auto& filter_str) {
							return distribution::get_record(filter_str);
						});
					} else if (filter.at(0) == '-') {
						filter.erase(0, 1);

					    data.rawFilters.NOT.emplace_back(distribution::get_record(filter));

					} else if (filter.at(0) == '*') {
						filter.erase(0, 1);
						data.rawFilters.ANY.emplace_back(filter);

					} else {
						data.rawFilters.MATCH.emplace_back(distribution::get_record(filter));
					}
				}
			}

			//TRAITS
			if (kTraits < size) {
				auto& traitsStr = sections[kTraits];
				switch (type) {
				case ITEM::kArmor:
					data.traits = std::make_unique<TRAITS::ArmorTraits>(traitsStr);
					break;
				case ITEM::kWeapon:
					data.traits = std::make_unique<TRAITS::WeaponTraits>(traitsStr);
					break;
				case ITEM::kAmmo:
					data.traits = std::make_unique<TRAITS::AmmoTraits>(traitsStr);
					break;
				case ITEM::kMagicEffect:
					data.traits = std::make_unique<TRAITS::MagicEffectTraits>(traitsStr);
					break;
				case ITEM::kPotion:
					data.traits = std::make_unique<TRAITS::PotionTraits>(traitsStr);
					break;
				case ITEM::kIngredient:
					data.traits = std::make_unique<TRAITS::IngredientTraits>(traitsStr);
					break;
				case ITEM::kBook:
					data.traits = std::make_unique<TRAITS::BookTraits>(traitsStr);
					break;
				case ITEM::kSoulGem:
					data.traits = std::make_unique<TRAITS::SoulGemTraits>(traitsStr);
					break;
				case ITEM::kSpell:
					data.traits = std::make_unique<TRAITS::SpellTraits>(traitsStr);
					break;
				default:
					break;
				}
			}

			//CHANCE
			if (INI::kChance < size) {
				const auto& chanceStr = sections[kChance];
				if (distribution::is_valid_entry(chanceStr)) {
					data.chance = string::to_num<Chance>(chanceStr);
				}
			}

			//PATH
			data.path = a_path;

			return std::make_pair(std::move(data), type);
		}
	}

	std::pair<bool, bool> GetConfigs()
	{
		logger::info("{:*^50}", "INI");

		std::vector<std::string> configs = distribution::get_configs(R"(Data\)", "_KID"sv);

		if (configs.empty()) {
			logger::warn("No .ini files with _KID suffix were found within the Data folder, aborting...");
			return { false, false };
		}

		logger::info("{} matching inis found", configs.size());

		std::ranges::sort(configs);

		bool shouldLogErrors{ false };

		for (auto& path : configs) {
			logger::info("\tINI : {}", path);

			CSimpleIniA ini;
			ini.SetUnicode();
			ini.SetMultiKey();

			if (const auto rc = ini.LoadFile(path.c_str()); rc < 0) {
				logger::error("\t\tcouldn't read INI");
				continue;
			}

			string::replace_first_instance(path, "Data\\", "");

			if (const auto values = ini.GetSection(""); values) {
				for (const auto& entry : *values | std::views::values) {
					try {
						std::string entryStr{ entry };
						auto [data, type] = detail::parse_config(entryStr, path);
						INIs[type].emplace_back(std::move(data));
					} catch (...) {
						logger::error("\t\tFailed to parse entry [Keyword = {}]", entry);
						shouldLogErrors = true;
					}
				}
			}
		}

		return { true, shouldLogErrors };
	}
}
