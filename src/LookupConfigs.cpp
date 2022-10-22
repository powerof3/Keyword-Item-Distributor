#include "LookupConfigs.h"

std::pair<bool, bool> Lookup::Config::Read()
{
	logger::info("{:*^30}", "INI");

	std::vector<std::string> configs;

	constexpr auto folder = R"(Data\)";
	for (const auto& entry : std::filesystem::directory_iterator(folder)) {
		if (entry.exists() && !entry.path().empty() && entry.path().extension() == ".ini"sv) {
			if (const auto path = entry.path().string(); path.rfind("_KID") != std::string::npos) {
				configs.push_back(path);
			}
		}
	}

	if (configs.empty()) {
		logger::warn("	No .ini files with _KID suffix were found within the Data folder, aborting...");
		return { false, false };
	}

	logger::info("	{} matching inis found", configs.size());

	std::ranges::sort(configs);

	//initialize map
	for (size_t i = 0; i < ITEM::TYPE::kTotal; i++) {
		auto type = static_cast<ITEM::TYPE>(i);
		INIs[type] = INIDataVec{};
	}

	bool shouldLogErrors{ false };

	for (auto& path : configs) {
		logger::info("	INI : {}", path);

		CSimpleIniA ini;
		ini.SetUnicode();
		ini.SetMultiKey();

		if (const auto rc = ini.LoadFile(path.c_str()); rc < 0) {
			logger::error("		couldn't read INI");
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
					logger::error("		Failed to parse entry [Keyword = {}]", entry);
					shouldLogErrors = true;
				}
			}
		}
	}

	return { true, shouldLogErrors };
}
