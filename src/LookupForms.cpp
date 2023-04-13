#include "LookupForms.h"

#include "KeywordData.h"
#include "KeywordDependencies.h"
#include "LookupConfigs.h"

namespace Forms
{
	using namespace Keyword;

	bool LookupForms()
	{
		logger::info("{:*^50}", "LOOKUP");

		ForEachDistributable([]<typename T>(Distributable<T>& a_distributable) {
			a_distributable.LookupForms();
			Dependencies::ResolveKeywords(a_distributable);
		});

		return armors || weapons || ammo || magicEffects || potions || scrolls || locations || ingredients || books || miscItems || keys || soulGems || spells;
	}

	void LogFormLookup()
	{
		logger::info("{:*^50}", "PROCESSING");

		ForEachDistributable([]<typename T>(const Distributable<T>& a_distributable) {
			const auto type = a_distributable.GetType();
			if (const auto& rawKeywords = INI::INIs[type]; !rawKeywords.empty()) {
				logger::info("Adding {}/{} keywords to {}", a_distributable.size(), rawKeywords.size(), ITEM::logTypes[type]);
			}
		});

		// clear raw configs
		INI::INIs.clear();
		// clear dependencies map
	    allKeywords.clear();

		// Clear logger's buffer to free some memory :)
		buffered_logger::clear();
	}
}
