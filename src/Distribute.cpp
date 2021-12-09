#include "Distribute.h"

void Distribute::AddKeywords()
{
	if (const auto dataHandler = RE::TESDataHandler::GetSingleton(); dataHandler) {
		const auto distribute = [&]<typename T>(const ITEM::TYPE a_type, const std::string& a_record, RE::BSTArray<T*>& formArray) {
			auto& keywords = Keywords[a_type];
			if (!keywords.empty()) {
				for (const auto& item : formArray) {
					if (item) {
						add_keyword(*item, keywords);
					}
				}
				for (auto& formData : keywords) {
					auto keyword = std::get<DATA::kForm>(formData);
					auto count = std::get<DATA::kCount>(formData);

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
		distribute(ITEM::kBook, "books", dataHandler->GetFormArray<RE::TESObjectBOOK>());
	}
}
