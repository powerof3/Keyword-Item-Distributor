#include "LookupFilters.h"

namespace Filter
{
	void SanitizeString(std::string& a_string)
	{
		std::ranges::transform(a_string, a_string.begin(),
			[](unsigned char c) { return static_cast<unsigned char>(std::tolower(c)); });
	}

	void SanitizePath(std::string& a_string)
	{
		SanitizeString(a_string);

		a_string = srell::regex_replace(a_string, srell::regex(R"(/+|\\+)"), R"(\)");
		a_string = srell::regex_replace(a_string, srell::regex(R"(^\\+)"), "");
		a_string = srell::regex_replace(a_string, srell::regex(R"(.*?[^\s]meshes\\|^meshes\\)", srell::regex::icase), "");
	}

	Data::Data(ProcessedFilters a_processedFilters, TraitsPtr a_traits, Chance a_chance) :
		processedFilters(std::move(a_processedFilters)),
		traits(std::move(a_traits)),
		chance(a_chance)
	{}
}

namespace Item
{
	Data::Data(RE::TESForm* a_item)
	{
		item = a_item;
		edid = EDID::get_editorID(a_item);
		name = a_item->GetName();

		if (const auto tesModel = a_item->As<RE::TESModel>()) {
			model = tesModel->GetModel();
			Filter::SanitizeString(model);
		}

		const auto kywdForm = a_item->As<RE::BGSKeywordForm>();
		keywords = { kywdForm->keywords, kywdForm->keywords + kywdForm->numKeywords };
	}

	bool Data::PassedFilters(RE::BGSKeyword* a_keyword, const FilterData& a_filters)
	{
		// Skip if keyword exists
		if (keywords.contains(a_keyword)) {
			return false;
		}

		// Fail chance first to avoid running unnecessary checks
		if (a_filters.chance < 100) {
			// create unique seed based on keyword editorID (can't use formID because it can be dynamic) and item formID
			// item formID alone would result in same RNG chance for different keywords
			const auto seed = hash::szudzik_pair(
				hash::fnv1a_32<std::string_view>(a_keyword->GetFormEditorID()),
				item->GetFormID());

			const auto randNum = RNG(seed).generate<Chance>(0, 100);
			if (randNum > a_filters.chance) {
				return false;
			}
		}

		// STRING,FORM
		if (!a_filters.processedFilters.ALL.empty() && !HasFormOrStringFilter(a_filters.processedFilters.ALL, true)) {
			return false;
		}
		if (!a_filters.processedFilters.NOT.empty() && HasFormOrStringFilter(a_filters.processedFilters.NOT)) {
			return false;
		}
		if (!a_filters.processedFilters.MATCH.empty() && !HasFormOrStringFilter(a_filters.processedFilters.MATCH)) {
			return false;
		}
		if (!a_filters.processedFilters.ANY.empty() && !ContainsStringFilter(a_filters.processedFilters.ANY)) {
			return false;
		}

		// TRAITS
		if (a_filters.traits && !a_filters.traits->PassFilter(item)) {
			return false;
		}

		keywords.emplace(a_keyword);

		return true;
	}

	bool Data::HasFormOrStringFilter(const ProcessedVec& a_processed, bool a_all) const
	{
		const auto has_form_or_string_filter = [&](const FormOrString& a_formString) {
			bool result = false;
			std::visit(overload{
						   [&](RE::TESForm* a_form) {
							   result = HasFormFilter(a_form);
						   },
						   [&](const RE::TESFile* a_file) {
							   result = a_file->IsFormInMod(item->GetFormID());
						   },
						   [&](const std::string& a_str) {
							   result = HasStringFilter(a_str);
						   } },
				a_formString);
			return result;
		};

		if (a_all) {
			return std::ranges::all_of(a_processed, has_form_or_string_filter);
		} else {
			return std::ranges::any_of(a_processed, has_form_or_string_filter);
		}
	}

	static RE::EffectSetting* GetCostliestMGEF(RE::TESForm* a_form)
	{
		if (a_form) {
			if (const auto magicItem = a_form->As<RE::MagicItem>()) {
				auto effect = magicItem->GetCostliestEffectItem();
				if (const auto mgef = effect ? effect->baseEffect : nullptr) {
					return mgef;
				}
			}
		}
		return nullptr;
	}

	bool Data::HasFormFilter(RE::TESForm* a_formFilter) const
	{
		switch (a_formFilter->GetFormType()) {
		case RE::FormType::Weapon:
		case RE::FormType::Ammo:
		case RE::FormType::Scroll:
		case RE::FormType::Book:
		case RE::FormType::KeyMaster:
		case RE::FormType::SoulGem:
		case RE::FormType::Flora:
		case RE::FormType::Activator:
		case RE::FormType::Furniture:
		case RE::FormType::Race:
		case RE::FormType::TalkingActivator:
			return item == a_formFilter;
		case RE::FormType::Keyword:
			{
				auto keyword = a_formFilter->As<RE::BGSKeyword>();
				if (keywords.contains(keyword)) {
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
				return item == a_formFilter;
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
				return item == a_formFilter;
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
				return item == a_formFilter;
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
				return item == a_formFilter;
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
					if (result = HasFormFilter(a_form); result) {
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

	bool Data::HasStringFilter(const std::string& a_str) const
	{
		if (string::iequals(edid, a_str) || string::iequals(name, a_str)) {
			return true;
		}

		if (AV::map.contains(a_str)) {
			switch (item->GetFormType()) {
			case RE::FormType::Weapon:
				{
					const auto weapon = item->As<RE::TESObjectWEAP>();
					return AV::GetActorValue(weapon->weaponData.skill.get()) == a_str;
				}
			case RE::FormType::MagicEffect:
				{
					const auto mgef = item->As<RE::EffectSetting>();
					return AV::GetActorValue(mgef->data.associatedSkill) == a_str ||
					       AV::GetActorValue(mgef->data.primaryAV) == a_str ||
					       AV::GetActorValue(mgef->data.secondaryAV) == a_str ||
					       AV::GetActorValue(mgef->data.resistVariable) == a_str;
				}
			case RE::FormType::Book:
				{
					const auto book = item->As<RE::TESObjectBOOK>();
					auto       skill = RE::ActorValue::kNone;
					if (book->TeachesSkill()) {
						skill = book->data.teaches.actorValueToAdvance;
					} else if (book->TeachesSpell() && book->data.teaches.spell) {
						skill = book->data.teaches.spell->GetAssociatedSkill();
					}
					return AV::GetActorValue(skill) == a_str;
				}
			case RE::FormType::AlchemyItem:
			case RE::FormType::Ingredient:
			case RE::FormType::Scroll:
			case RE::FormType::Spell:
			case RE::FormType::Enchantment:
				{
					const auto magicItem = item->As<RE::MagicItem>();
					if (AV::GetActorValue(magicItem->GetAssociatedSkill()) == a_str) {
						return true;
					}
					if (const auto mgef = GetCostliestMGEF(magicItem)) {
						return AV::GetActorValue(mgef->data.associatedSkill) == a_str ||
						       AV::GetActorValue(mgef->data.primaryAV) == a_str ||
						       AV::GetActorValue(mgef->data.secondaryAV) == a_str ||
						       AV::GetActorValue(mgef->data.resistVariable) == a_str;
					}
					return false;
				}
			default:
				return false;
			}
		}

		if (ARCHETYPE::map.contains(a_str)) {
			if (auto mgef = item->As<RE::EffectSetting>()) {
				return std::to_string(mgef->data.archetype) == a_str;
			} else if (mgef = GetCostliestMGEF(item); mgef) {
				return std::to_string(mgef->data.archetype) == a_str;
			}
		}

		if (a_str.contains(".nif")) {
			return model == a_str;
		}

		return false;
	}

	bool Data::ContainsStringFilter(const std::vector<std::string>& a_strings) const
	{
		return std::ranges::any_of(a_strings, [&](const auto& str) {
			if (str.contains(".nif")) {
				return model.contains(str);
			}
			return string::icontains(edid, str) ||
			       string::icontains(name, str) ||
			       std::any_of(keywords.begin(), keywords.end(), [&](const auto& keyword) {
					   return keyword->formEditorID.contains(str);
				   });
		});
	}
}
