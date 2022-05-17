#include "Distribute.h"

void Distribute::AddKeywords()
{
	if (const auto dataHandler = RE::TESDataHandler::GetSingleton(); dataHandler) {
		logger::info("{:*^30}", "RESULT");

		distribute<RE::TESObjectARMO>(ITEM::kArmor);
		distribute<RE::TESObjectWEAP>(ITEM::kWeapon);
		distribute<RE::TESAmmo>(ITEM::kAmmo);
		distribute<RE::EffectSetting>(ITEM::kMagicEffect);
		distribute<RE::AlchemyItem>(ITEM::kPotion);
		distribute<RE::ScrollItem>(ITEM::kScroll);
		distribute<RE::BGSLocation>(ITEM::kLocation);
		distribute<RE::IngredientItem>(ITEM::kIngredient);
		distribute<RE::TESObjectBOOK>(ITEM::kBook);
		distribute<RE::TESObjectMISC>(ITEM::kMiscItem);
	}
}
