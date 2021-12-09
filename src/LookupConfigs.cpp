#include "LookupConfigs.h"

std::map<ITEM::TYPE, INIDataVec> INIs;

bool Lookup::Config::Read()
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
		return false;
	}

	logger::info("	{} matching inis found", configs.size());

	//initialize map
	for (size_t i = 0; i < ITEM::TYPE::kTotal; i++) {
		auto type = static_cast<ITEM::TYPE>(i);
		INIs[type] = INIDataVec{};
	}

	for (auto& path : configs) {
		logger::info("	INI : {}", path);

		CSimpleIniA ini;
		ini.SetUnicode();
		ini.SetMultiKey();

		if (const auto rc = ini.LoadFile(path.c_str()); rc < 0) {
			logger::error("	couldn't read INI");
			continue;
		}

		if (auto values = ini.GetSection(""); values) {
			for (auto& [key, entry] : *values) {
				auto [data, type] = parse_config(entry);
				INIs[type].emplace_back(data);
			}
		}
	}

	return true;
}
