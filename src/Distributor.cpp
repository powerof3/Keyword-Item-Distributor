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

		const auto log_addition = [](ITEM::TYPE a_type, const std::string& a_records) {
			if (!INIs[a_type].empty()) {
				logger::info("	Adding {}/{} keywords to {}", INIs[a_type].size(), Keywords[a_type].size(), a_records);
			}
		};
		
		log_addition(ITEM::kArmor, "armors");
		log_addition(ITEM::kWeapon, "weapons");
		log_addition(ITEM::kAmmo, "ammo");
		log_addition(ITEM::kMagicEffect, "magic effects");
		log_addition(ITEM::kPotion, "potions");
		log_addition(ITEM::kScroll, "scrolls");
		log_addition(ITEM::kLocation, "locations");
		log_addition(ITEM::kIngredient, "ingredients");
	}
	return result;
}

void Distribute::AddKeywords()
{
	if (const auto dataHandler = RE::TESDataHandler::GetSingleton(); dataHandler) {
		
		const auto distribute = [&]<typename T>(ITEM::TYPE a_type, const std::string& a_record, RE::BSTArray<T*>& formArray) {
			auto& keywords = Keywords[a_type];
			if (!keywords.empty()) {
				for (const auto& item : formArray) {
					if (item) {
						add_keyword(*item, keywords);
					}
				}
				for (auto& formData : keywords) {
					auto keyword = std::get<DATA_TYPE::kForm>(formData);
					auto count = std::get<DATA_TYPE::kCount>(formData);

					if (keyword) {
						logger::info("{} [0x{:X}] added to {}/{} {}", keyword->GetFormEditorID(), keyword->GetFormID(), count, formArray.size(), a_record);
					}
				}
			}
		};

		logger::info("{:*^30}", "RESULT");

		distribute(ITEM::kArmor, "armors", dataHandler->GetFormArray<RE::TESObjectARMO>());
		distribute(ITEM::kWeapon, "weapons", dataHandler->GetFormArray<RE::TESObjectWEAP>());
		distribute(ITEM::kAmmo, "ammo", dataHandler->GetFormArray<RE::TESAmmo>());
		distribute(ITEM::kMagicEffect, "magic effects", dataHandler->GetFormArray<RE::EffectSetting>());
		distribute(ITEM::kPotion, "potions", dataHandler->GetFormArray<RE::AlchemyItem>());
		distribute(ITEM::kScroll, "scrolls", dataHandler->GetFormArray<RE::ScrollItem>());
		distribute(ITEM::kLocation, "locations", dataHandler->GetFormArray<RE::BGSLocation>());		
		distribute(ITEM::kIngredient, "ingredients", dataHandler->GetFormArray<RE::IngredientItem>());	
	}
}
