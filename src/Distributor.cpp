#include "Distributor.h"

static std::map<ITEM::TYPE, INIDataVec> INIs;
static std::map<ITEM::TYPE, KeywordDataVec> Keywords;

bool INI::Read()
{
	logger::info("{:*^30}", "INI");

	std::vector<std::string> configs;

	auto constexpr folder = R"(Data\)";
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
		Keywords[type] = KeywordDataVec{};
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
				auto [data, type] = parse_ini(entry);
				INIs[type].emplace_back(data);
			}
		}
	}

	return true;
}

bool Lookup::GetForms()
{
	bool result = false;

	if (const auto dataHandler = RE::TESDataHandler::GetSingleton(); dataHandler) {
		logger::info("{:*^30}", "LOOKUP");

		for (size_t i = 0; i < ITEM::TYPE::kTotal; i++) {
			auto type = static_cast<ITEM::TYPE>(i);
			get_forms(dataHandler, INIs[type], Keywords[type]);

			if (!Keywords[type].empty()) {
				result = true;
			}
		}
	}

	if (result) {
		logger::info("{:*^30}", "PROCESSING");

		logger::info("	Adding {}/{} keywords to armors", INIs[ITEM::TYPE::kArmor].size(), Keywords[ITEM::TYPE::kArmor].size());
		logger::info("	Adding {}/{} keywords to weapons", INIs[ITEM::TYPE::kWeapon].size(), Keywords[ITEM::TYPE::kWeapon].size());
		logger::info("	Adding {}/{} keywords to ammo", INIs[ITEM::TYPE::kAmmo].size(), Keywords[ITEM::TYPE::kAmmo].size());
	}
	return result;
}

void Distribute::AddKeywords()
{
	if (const auto dataHandler = RE::TESDataHandler::GetSingleton(); dataHandler) {
		auto& armorKeywords = Keywords[ITEM::TYPE::kArmor];
		if (!armorKeywords.empty()) {
			for (const auto& armor : dataHandler->GetFormArray<RE::TESObjectARMO>()) {
				if (armor) {
					add_keyword(*armor, armorKeywords);
				}
			}
		}
		auto& weapKeywords = Keywords[ITEM::TYPE::kWeapon];
		if (!weapKeywords.empty()) {
			for (const auto& weapon : dataHandler->GetFormArray<RE::TESObjectWEAP>()) {
				if (weapon) {
					add_keyword(*weapon, weapKeywords);
				}
			}
		}
		auto& ammoKeywords = Keywords[ITEM::TYPE::kAmmo];
		if (!ammoKeywords.empty()) {
			for (const auto& ammo : dataHandler->GetFormArray<RE::TESAmmo>()) {
				if (ammo) {
					add_keyword(*ammo, ammoKeywords);
				}
			}
		}

		logger::info("{:*^30}", "RESULT");

		list_keyword_count("armors", armorKeywords, dataHandler->GetFormArray<RE::TESObjectARMO>().size());
		list_keyword_count("weapons", weapKeywords, dataHandler->GetFormArray<RE::TESObjectWEAP>().size());
		list_keyword_count("ammo", ammoKeywords, dataHandler->GetFormArray<RE::TESAmmo>().size());
	}
}
