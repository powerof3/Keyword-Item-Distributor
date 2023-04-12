#include "LookupForms.h"

#include "KeywordData.h"
#include "LookupConfigs.h"

namespace Forms
{
	using namespace Keyword;

	bool LookupForms()
	{
		logger::info("{:*^50}", "LOOKUP");

		ForEachDistributable([]<typename T>(Distributables<T>& a_distributables) {
			a_distributables.LookupForms();
		});

		return armors || weapons || ammo || magicEffects || potions || scrolls || locations || ingredients || books || miscItems || keys || soulGems || spells;
	}

	void LogFormLookup()
	{
		logger::info("{:*^50}", "PROCESSING");

		ForEachDistributable([]<typename T>(const Distributables<T>& a_distributables) {
            const auto type = a_distributables.GetType();
		    if (const auto& rawKeywords = INI::INIs[type]; !rawKeywords.empty()) {
				logger::info("Adding {}/{} keywords to {}", a_distributables.size(), rawKeywords.size(), ITEM::logTypes[type]);
			}
		});

		// clear raw configs
		INI::INIs.clear();

		// Clear logger's buffer to free some memory :)
		buffered_logger::clear();
	}
}
