#include "Data/ItemData.h"

#include "ExclusiveGroups.h"

ItemData::ItemData(RE::TESForm* a_form) :
	item(a_form)
{
	const auto kywdForm = a_form->As<RE::BGSKeywordForm>();
	if (kywdForm && kywdForm->numKeywords > 0) {
		originalKeywords = { kywdForm->keywords, kywdForm->numKeywords };
	}
}

const std::string& ItemData::GetEDID() const
{
	if (!cacheEDID) {
		edid = string::tolower(EDID::get_editorID(item));
		cacheEDID = true;
	}
	return edid;
}

const std::string& ItemData::GetName() const
{
	if (!cacheName) {
		name = string::tolower(item->GetName());
		cacheName = true;
	}
	return name;
}

const std::string& ItemData::GetModel() const
{
	if (!cacheModel) {
		if (const auto tesModel = item->As<RE::TESModel>()) {
			model = string::tolower(tesModel->GetModel());
		}
		cacheModel = true;
	}
	return model;
}

bool ItemData::FormFilter(RE::TESForm* a_formFilter) const
{
	if (item == a_formFilter) {
		return true;
	}

	switch (a_formFilter->GetFormType()) {
	case RE::FormType::Keyword:
		{
			auto keyword = a_formFilter->As<RE::BGSKeyword>();
			if (HasKeyword(keyword)) {
				return true;
			}
			if (auto mgef = GetCostliestMGEF(item)) {
				return mgef->HasKeyword(keyword);
			}
			if (const auto book = item->As<RE::TESObjectBOOK>()) {
				if (auto mgef = GetCostliestMGEF(book->GetSpell())) {
					return mgef->HasKeyword(keyword);
				}
			}
			if (const auto armor = item->As<RE::TESObjectARMO>()) {
				if (auto mgef = GetCostliestMGEF(armor->formEnchanting)) {
					return mgef->HasKeyword(keyword);
				}
			}
			if (const auto weapon = item->As<RE::TESObjectWEAP>()) {
				if (auto mgef = GetCostliestMGEF(weapon->formEnchanting)) {
					return mgef->HasKeyword(keyword);
				}
			}
			return false;
		}
	case RE::FormType::Armor:
		{
			if (const auto race = item->As<RE::TESRace>()) {
				return race->skin == a_formFilter;
			}
			return false;
		}
	case RE::FormType::Location:
		{
			const auto loc = item->As<RE::BGSLocation>();
			const auto filterLoc = a_formFilter->As<RE::BGSLocation>();

			return loc && filterLoc && (loc == filterLoc || loc->IsParent(filterLoc));
		}
	case RE::FormType::Projectile:
		{
			if (const auto ammo = item->As<RE::TESAmmo>()) {
				return ammo->data.projectile == a_formFilter;
			} else if (const auto mgef = item->As<RE::EffectSetting>()) {
				return mgef->data.projectileBase == a_formFilter;
			}
			return false;
		}
	case RE::FormType::MagicEffect:
		{
			if (const auto spell = item->As<RE::MagicItem>()) {
				return std::ranges::any_of(spell->effects, [&](const auto& effect) {
					return effect && effect->baseEffect == a_formFilter;
				});
			}
			return false;
		}
	case RE::FormType::EffectShader:
		{
			const auto mgef = item->As<RE::EffectSetting>();
			return mgef && (mgef->data.effectShader == a_formFilter || mgef->data.enchantShader == a_formFilter);
		}
	case RE::FormType::ReferenceEffect:
		{
			const auto mgef = item->As<RE::EffectSetting>();
			return mgef && (mgef->data.hitVisuals == a_formFilter || mgef->data.enchantVisuals == a_formFilter);
		}
	case RE::FormType::ArtObject:
		{
			if (const auto mgef = item->As<RE::EffectSetting>()) {
				return mgef->data.castingArt == a_formFilter || mgef->data.hitEffectArt == a_formFilter || mgef->data.enchantEffectArt == a_formFilter;
			}
			if (const auto race = item->As<RE::TESRace>()) {
				return race->dismemberBlood == a_formFilter;
			}
			return false;
		}
	case RE::FormType::MusicType:
		{
			const auto loc = item->As<RE::BGSLocation>();
			return loc && loc->musicType == a_formFilter;
		}
	case RE::FormType::Faction:
		{
			const auto loc = item->As<RE::BGSLocation>();
			return loc && loc->unreportedCrimeFaction == a_formFilter;
		}
	case RE::FormType::AlchemyItem:
	case RE::FormType::Ingredient:
	case RE::FormType::Misc:
		{
			if (const auto flora = item->As<RE::TESFlora>()) {
				return flora->produceItem == a_formFilter;
			}
			return false;
		}
	case RE::FormType::Spell:
		{
			const auto spell = a_formFilter->As<RE::SpellItem>();
			if (const auto book = item->As<RE::TESObjectBOOK>()) {
				return book->GetSpell() == spell;
			}
			if (const auto race = item->As<RE::TESRace>()) {
				return race->actorEffects && race->actorEffects->GetIndex(spell).has_value();
			}
			if (const auto furniture = item->As<RE::TESFurniture>()) {
				return furniture->associatedForm == a_formFilter;
			}
			return false;
		}
	case RE::FormType::Enchantment:
		{
			if (const auto weapon = item->As<RE::TESObjectWEAP>()) {
				return weapon->formEnchanting == a_formFilter;
			}
			if (const auto armor = item->As<RE::TESObjectARMO>()) {
				return armor->formEnchanting == a_formFilter;
			}
			if (const auto enchantment = item->As<RE::EnchantmentItem>()) {
				return enchantment == a_formFilter || enchantment->data.baseEnchantment == a_formFilter;
			}
			return false;
		}
	case RE::FormType::EquipSlot:
		{
			if (const auto equipType = item->As<RE::BGSEquipType>()) {
				return equipType->GetEquipSlot() == a_formFilter;
			}
			return false;
		}
	case RE::FormType::VoiceType:
		{
			if (const auto talkingActivator = item->As<RE::BGSTalkingActivator>()) {
				return talkingActivator->GetObjectVoiceType() == a_formFilter;
			}
			return false;
		}
	case RE::FormType::LeveledItem:
		{
			if (const auto flora = item->As<RE::TESFlora>()) {
				return flora->produceItem == a_formFilter;
			}
			return false;
		}
	case RE::FormType::Water:
		{
			if (const auto activator = item->As<RE::TESObjectACTI>()) {
				return activator->GetWaterType() == a_formFilter;
			}
			return false;
		}
	case RE::FormType::Perk:
		{
			if (const auto spell = item->As<RE::SpellItem>()) {
				return spell->data.castingPerk == a_formFilter;
			}
			if (const auto mgef = item->As<RE::EffectSetting>()) {
				return mgef->data.perk == a_formFilter;
			}
			return false;
		}
	case RE::FormType::FormList:
		{
			if (const auto enchantment = item->As<RE::EnchantmentItem>()) {
				if (enchantment->data.wornRestrictions == a_formFilter) {
					return true;
				}
			}

			bool       result = false;
			const auto list = a_formFilter->As<RE::BGSListForm>();
			list->ForEachForm([&](auto* a_form) {
				if (result = FormFilter(a_form); result) {
					return RE::BSContainer::ForEachResult::kStop;
				}
				return RE::BSContainer::ForEachResult::kContinue;
			});
			return result;
		}
	default:
		return false;
	}
}

bool ItemData::HasStringFilter(const std::string& a_str, StringType a_stringType, std::uint32_t a_enum) const
{
	if (a_stringType == StringType::kModel) {
		return GetModel() == a_str;
	}

	if (GetEDID() == a_str || GetName() == a_str) {
		return true;
	}

	switch (a_stringType) {
	case StringType::kAV:
		return MatchesActorValue(static_cast<RE::ActorValue>(a_enum));
	case StringType::kArchetype:
		return MatchesArchetype(static_cast<RE::EffectArchetype>(a_enum));
	default:
		return false;
	}
}

bool ItemData::ContainsStringFilter(const std::string& a_str, StringType a_stringType) const
{
	if (a_stringType == StringType::kModel) {
		return GetModel().contains(a_str);
	}

	if (GetEDID().contains(a_str) || GetName().contains(a_str)) {
		return true;
	}

	auto check_keywords = [&](const auto& kywds) {
		for (const auto& kywd : kywds) {
			if (kywd && kywd->formEditorID.contains(a_str)) {
				return true;
			}
		}
		return false;
	};

	return check_keywords(originalKeywords) || check_keywords(addedKeywords);
}

bool ItemData::MatchesActorValue(RE::ActorValue a_av) const
{
	switch (item->GetFormType()) {
	case RE::FormType::Weapon:
		return item->As<RE::TESObjectWEAP>()->weaponData.skill.get() == a_av;
	case RE::FormType::MagicEffect:
		{
			const auto& d = item->As<RE::EffectSetting>()->data;
			return d.associatedSkill == a_av ||
			       d.primaryAV == a_av ||
			       d.secondaryAV == a_av ||
			       d.resistVariable == a_av;
		}
	case RE::FormType::Book:
		{
			const auto book = item->As<RE::TESObjectBOOK>();
			if (book->TeachesSkill()) {
				return book->data.teaches.actorValueToAdvance == a_av;
			}
			if (book->TeachesSpell() && book->data.teaches.spell) {
				return AV::GetAssociatedSkill(book->data.teaches.spell) == a_av;
			}
			return false;
		}
	case RE::FormType::AlchemyItem:
	case RE::FormType::Ingredient:
	case RE::FormType::Scroll:
	case RE::FormType::Spell:
	case RE::FormType::Enchantment:
		{
			const auto magicItem = item->As<RE::MagicItem>();
			if (AV::GetAssociatedSkill(magicItem) == a_av) {
				return true;
			}
			if (const auto mgef = GetCostliestMGEF(magicItem)) {
				const auto& d = mgef->data;
				return d.associatedSkill == a_av ||
				       d.primaryAV == a_av ||
				       d.secondaryAV == a_av ||
				       d.resistVariable == a_av;
			}
			return false;
		}
	default:
		return false;
	}
}

bool ItemData::MatchesArchetype(RE::EffectArchetype a_archetype) const
{
	if (auto mgef = item->As<RE::EffectSetting>()) {
		return mgef->data.archetype == a_archetype;
	} else if (mgef = GetCostliestMGEF(item); mgef) {
		return mgef->data.archetype == a_archetype;
	}
	return false;
}

bool ItemData::HasKeyword(RE::BGSKeyword* a_keyword) const
{
	for (auto& kywd : originalKeywords) {
		if (kywd == a_keyword) {
			return true;
		}
	}
	for (auto& kywd : addedKeywords) {
		if (kywd == a_keyword) {
			return true;
		}
	}
	return false;
}

bool ItemData::HasMutuallyExclusiveKeyword(RE::BGSKeyword* a_keyword) const
{
	const auto& excludedForms = ExclusiveGroups::Manager::GetSingleton()->MutuallyExclusiveKeywordsForKeyword(a_keyword);
	if (excludedForms.empty()) {
		return false;
	}

	return std::ranges::any_of(excludedForms, [&](auto keyword) {
		return HasKeyword(keyword);
	});
}

void ItemData::AddKeyword(RE::BGSKeyword* a_keyword)
{
	addedKeywords.emplace_back(a_keyword);
}
