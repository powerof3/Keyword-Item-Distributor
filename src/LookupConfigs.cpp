#include "LookupConfigs.h"
#include "ExclusiveGroups.h"

namespace INI
{
	Data::Data(std::string& a_value, const std::string& a_path) :
		path(a_path),
		resolvedKeyword(nullptr)
	{
#ifdef SKYRIMVR
		// swap dawnguard and dragonborn forms
		// VR apparently does not load masters in order so the lookup fails
		static const srell::regex re_dawnguard(R"((0x0*2)([0-9a-f]{6}))", srell::regex_constants::optimize | srell::regex::icase);
		a_value = regex_replace(a_value, re_dawnguard, "0x$2~Dawnguard.esm");

		static const srell::regex re_dragonborn(R"((0x0*4)([0-9a-f]{6}))", srell::regex_constants::optimize | srell::regex::icase);
		a_value = regex_replace(a_value, re_dragonborn, "0x$2~Dragonborn.esm");
#endif

		const auto sections = string::split(a_value, "|");
		const auto size = sections.size();

		//TYPE
		if (kType < size) {
			if (const auto& typeStr = sections[kType]; !typeStr.empty()) {
				type = DISTRIBUTION::GetType(typeStr);
			}
		}

		if (type == DISTRIBUTION::TYPE::kNone) {
			return;
		}

		//[FORMID/ESP] / string
		if (kFormID < size) {
			rawForm = RawForm(sections[kFormID]);
		}

		//FILTERS
		if (kFilters < size) {
			if (const auto& filters = sections[kFilters]; distribution::is_valid_entry(filters)) {
				criteria.filters = ConfigFilterSet(sections[kFilters]);
			}
		}

		//TRAITS
		if (kTraits < size) {
			if (const auto& traits = sections[kTraits]; distribution::is_valid_entry(traits)) {
				criteria.traits = Traits::Create(type, traits);
			}
		}

		//CHANCE
		if (INI::kChance < size) {
			if (const auto& chanceStr = sections[kChance]; distribution::is_valid_entry(chanceStr)) {
				criteria.chance.value = string::to_num<float>(chanceStr);
			}
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
				for (auto& [key, entry] : *values) {
					try {
						std::string entryStr{ entry };

						if (ExclusiveGroups::INI::TryParse(key.pItem, entryStr, path)) {
							continue;
						}

						Data data(entryStr, path);
						if (data.type == DISTRIBUTION::TYPE::kNone) {
							continue;
						}

						INIs[data.type].emplace_back(std::move(data));

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
