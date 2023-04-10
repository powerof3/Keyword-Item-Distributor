#include "Distribute.h"

void Distribute::AddKeywords()
{
	if (const auto dataHandler = RE::TESDataHandler::GetSingleton(); dataHandler) {
		logger::info("{:*^30}", "RESULT");

		distribute<RE::TESObjectARMO>(ITEM::kArmor, armors);
		distribute<RE::TESObjectWEAP>(ITEM::kWeapon, weapons);
		distribute<RE::TESAmmo>(ITEM::kAmmo, ammo);
		distribute<RE::EffectSetting>(ITEM::kMagicEffect, magicEffects);
		distribute<RE::AlchemyItem>(ITEM::kPotion, potions);
		distribute<RE::ScrollItem>(ITEM::kScroll, scrolls);
		distribute<RE::BGSLocation>(ITEM::kLocation, locations);
		distribute<RE::IngredientItem>(ITEM::kIngredient, ingredients);
		distribute<RE::TESObjectBOOK>(ITEM::kBook, books);
		distribute<RE::TESObjectMISC>(ITEM::kMiscItem, miscItems);
		distribute<RE::TESKey>(ITEM::kKey, keys);
		distribute<RE::TESSoulGem>(ITEM::kSoulGem, soulGems);
		distribute<RE::SpellItem>(ITEM::kSpell, spells);

		// clear keywords
		Clear();
	}
}
