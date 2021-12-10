#include "LookupForms.h"

std::map<ITEM::TYPE, KeywordDataVec> Keywords;

bool Lookup::Forms::GetForms()
{
	bool result = false;

	if (const auto dataHandler = RE::TESDataHandler::GetSingleton(); dataHandler) {
		logger::info("{:*^30}", "LOOKUP");

		//initialize map
		for (size_t i = 0; i < ITEM::TYPE::kTotal; i++) {
			auto type = static_cast<ITEM::TYPE>(i);
			Keywords[type] = KeywordDataVec{};
		}
		
		Cache::EditorID::GetSingleton()->FillMap();

	    for (auto& [type, record] : ITEM::map) {
			get_forms(dataHandler, INIs[type], Keywords[type]);

			if (!Keywords[type].empty()) {
				result = true;
			}
		}
	}

	if (result) {
		logger::info("{:*^30}", "PROCESSING");


		for (auto& [type, record] : ITEM::map) {
			if (!INIs[type].empty()) {
				logger::info("	Adding {}/{} keywords to {}", INIs[type].size(), Keywords[type].size(), record);
			}
		}
	}
	
	return result;
}
