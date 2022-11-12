#include "LookupConfigs.h"

std::pair<bool, bool> Lookup::Config::Read()
{
	logger::info("{:*^30}", "INI");

	std::vector<std::string> configs = distribution::get_configs(R"(Data\)", "_KID"sv);

	if (configs.empty()) {
		logger::warn("\tNo .ini files with _KID suffix were found within the Data folder, aborting...");
		return { false, false };
	}

	logger::info("\t{} matching inis found", configs.size());

	std::ranges::sort(configs);

	//initialize map
	for (size_t i = 0; i < ITEM::TYPE::kTotal; i++) {
		auto type = static_cast<ITEM::TYPE>(i);
		INIs[type] = INIDataVec{};
	}

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
					auto [data, type] = parse_config(entryStr, path);
					INIs[type].emplace_back(data);
				} catch (...) {
					logger::error("\t\tFailed to parse entry [Keyword = {}]", entry);
					shouldLogErrors = true;
				}
			}
		}
	}

	return { true, shouldLogErrors };
}
