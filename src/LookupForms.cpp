#include "LookupForms.h"

#include "KeywordData.h"
#include "LookupConfigs.h"

namespace Forms
{
	using namespace Keyword;

    bool LookupForms()
	{
		if (const auto dataHandler = RE::TESDataHandler::GetSingleton(); dataHandler) {
			logger::info("{:*^50}", "LOOKUP");

			const auto lookup_forms = [&](const ITEM::TYPE type, Distributables& a_distributables) {
				a_distributables.LookupForms(dataHandler, type, INI::INIs[type]);
			};

			lookup_forms(ITEM::kArmor, armors);
			lookup_forms(ITEM::kWeapon, weapons);
			lookup_forms(ITEM::kAmmo, ammo);
			lookup_forms(ITEM::kMagicEffect, magicEffects);
			lookup_forms(ITEM::kPotion, potions);
			lookup_forms(ITEM::kScroll, scrolls);
			lookup_forms(ITEM::kLocation, locations);
			lookup_forms(ITEM::kIngredient, ingredients);
			lookup_forms(ITEM::kBook, books);
			lookup_forms(ITEM::kMiscItem, miscItems);
			lookup_forms(ITEM::kKey, keys);
			lookup_forms(ITEM::kSoulGem, soulGems);
			lookup_forms(ITEM::kSpell, spells);
		}

		return armors || weapons || ammo || magicEffects || potions || scrolls || locations || ingredients || books || miscItems || keys || soulGems || spells;
	}

	void LogFormLookup()
	{
		logger::info("{:*^50}", "PROCESSING");

		const auto list_lookup_result = [&](const ITEM::TYPE type, const Distributables& a_distributables) {
			if (const auto& rawKeywords = INI::INIs[type]; !rawKeywords.empty()) {
				logger::info("Adding {}/{} keywords to {}", a_distributables.size(), rawKeywords.size(), ITEM::logTypes[type]);
			}
		};

		list_lookup_result(ITEM::kArmor, armors);
		list_lookup_result(ITEM::kWeapon, weapons);
		list_lookup_result(ITEM::kAmmo, ammo);
		list_lookup_result(ITEM::kMagicEffect, magicEffects);
		list_lookup_result(ITEM::kPotion, potions);
		list_lookup_result(ITEM::kScroll, scrolls);
		list_lookup_result(ITEM::kLocation, locations);
		list_lookup_result(ITEM::kIngredient, ingredients);
		list_lookup_result(ITEM::kBook, books);
		list_lookup_result(ITEM::kMiscItem, miscItems);
		list_lookup_result(ITEM::kKey, keys);
		list_lookup_result(ITEM::kSoulGem, soulGems);
		list_lookup_result(ITEM::kSpell, spells);

		// clear raw configs
		INI::INIs.clear();

		// Clear logger's buffer to free some memory :)
		logger::clear();
	}
}
