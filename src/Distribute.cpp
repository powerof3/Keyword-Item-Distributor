#include "Distribute.h"

void Distribute::AddKeywords()
{
	if (const auto dataHandler = RE::TESDataHandler::GetSingleton(); dataHandler) {
		distribute<RE::TESObjectARMO>(armors);
		distribute<RE::TESObjectWEAP>(weapons);
		distribute<RE::TESAmmo>(ammo);
		distribute<RE::EffectSetting>(magicEffects);
		distribute<RE::AlchemyItem>(potions);
		distribute<RE::ScrollItem>(scrolls);
		distribute<RE::BGSLocation>(locations);
		distribute<RE::IngredientItem>(ingredients);
		distribute<RE::TESObjectBOOK>(books);
		distribute<RE::TESObjectMISC>(miscItems);
		distribute<RE::TESKey>(keys);
		distribute<RE::TESSoulGem>(soulGems);
		distribute<RE::SpellItem>(spells);

		logger::info("{:*^50}", "RESULT");

		log_keyword_count<RE::TESObjectARMO>(ITEM::kArmor, armors);
		log_keyword_count<RE::TESObjectWEAP>(ITEM::kWeapon, weapons);
		log_keyword_count<RE::TESAmmo>(ITEM::kAmmo, ammo);
		log_keyword_count<RE::EffectSetting>(ITEM::kMagicEffect, magicEffects);
		log_keyword_count<RE::AlchemyItem>(ITEM::kPotion, potions);
		log_keyword_count<RE::ScrollItem>(ITEM::kScroll, scrolls);
		log_keyword_count<RE::BGSLocation>(ITEM::kLocation, locations);
		log_keyword_count<RE::IngredientItem>(ITEM::kIngredient, ingredients);
		log_keyword_count<RE::TESObjectBOOK>(ITEM::kBook, books);
		log_keyword_count<RE::TESObjectMISC>(ITEM::kMiscItem, miscItems);
		log_keyword_count<RE::TESKey>(ITEM::kKey, keys);
		log_keyword_count<RE::TESSoulGem>(ITEM::kSoulGem, soulGems);
		log_keyword_count<RE::SpellItem>(ITEM::kSpell, spells);

		// clear keywords
		Clear();
	}
}
