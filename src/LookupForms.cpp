#include "LookupForms.h"

bool Lookup::Forms::GetForms()
{
	bool result = false;

	if (const auto dataHandler = RE::TESDataHandler::GetSingleton(); dataHandler) {
		logger::info("{:*^30}", "LOOKUP");

		for (const auto& type : ITEM::map | std::views::keys) {
			get_forms(dataHandler, type, INIs[type], Keywords[type]);

			if (!Keywords[type].empty()) {
				result = true;
			}
		}
	}

	if (result) {
		logger::info("{:*^30}", "PROCESSING");

		for (auto& [type, record] : ITEM::map) {
			if (!INIs[type].empty()) {
				logger::info("\tAdding {}/{} keywords to {}", Keywords[type].size(), INIs[type].size(), record);
			}
		}
	}

	return result;
}
