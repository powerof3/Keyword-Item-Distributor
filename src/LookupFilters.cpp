#include "LookupFilters.h"

namespace Filter
{
	Data::Data(ProcessedFilters a_processedFilters, TraitsPtr a_traits, Chance a_chance) :
		processedFilters(std::move(a_processedFilters)),
		traits(std::move(a_traits)),
		chance(a_chance)
	{}

	bool Data::PassedFilters(RE::TESForm* a_item)
	{
		// Fail chance first to avoid running unnecessary checks
		if (chance < 100) {
			const auto randNum = RNG(a_item->GetFormID()).Generate<Chance>(0, 100);
			if (randNum > chance) {
				return false;
			}
		}

		item = a_item;
		kywdForm = a_item->As<RE::BGSKeywordForm>();
		edid = Cache::EditorID::GetEditorID(a_item);
		name = a_item->GetName();

		// STRING,FORM
		if (!processedFilters.ALL.empty() && !HasFormOrStringFilter(processedFilters.ALL, true)) {
			return false;
		}
		if (!processedFilters.NOT.empty() && HasFormOrStringFilter(processedFilters.NOT)) {
			return false;
		}
		if (!processedFilters.MATCH.empty() && !HasFormOrStringFilter(processedFilters.MATCH)) {
			return false;
		}
		if (!processedFilters.ANY.empty() && !ContainsStringFilter(processedFilters.ANY)) {
			return false;
		}

		// TRAITS
		if (traits && !traits->PassFilter(a_item)) {
			return false;
		}

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
							   result = string::iequals(name, a_str) || string::iequals(edid, a_str) || has_skill_or_archetype(a_str);
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

	bool Data::HasFormFilter(RE::TESForm* a_formFilter) const
	{
		switch (a_formFilter->GetFormType()) {
		case RE::FormType::Armor:
		case RE::FormType::Weapon:
		case RE::FormType::Ammo:
		case RE::FormType::AlchemyItem:
		case RE::FormType::Scroll:
		case RE::FormType::Ingredient:
		case RE::FormType::Book:
		case RE::FormType::Misc:
		case RE::FormType::KeyMaster:
		case RE::FormType::SoulGem:
			return item == a_formFilter;
		case RE::FormType::Keyword:
			return kywdForm->HasKeyword(a_formFilter->As<RE::BGSKeyword>());
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
				const auto mgef = item->As<RE::EffectSetting>();
				return mgef && (mgef->data.castingArt == a_formFilter || mgef->data.hitEffectArt == a_formFilter || mgef->data.enchantEffectArt == a_formFilter);
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
		case RE::FormType::Spell:
			{
				if (const auto book = item->As<RE::TESObjectBOOK>()) {
					return book->GetSpell() == a_formFilter;
				}
				return item == a_formFilter;
			}
		case RE::FormType::EquipSlot:
			{
				if (const auto equipType = item->As<RE::BGSEquipType>()) {
					return equipType->GetEquipSlot() == a_formFilter;
				}
				return false;
			}
		case RE::FormType::FormList:
			{
				bool result = false;

				const auto list = a_formFilter->As<RE::BGSListForm>();
				list->ForEachForm([&](RE::TESForm& a_form) {
					if (result = HasFormFilter(&a_form); result) {
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

	bool Data::ContainsStringFilter(const std::vector<std::string>& a_strings) const
	{
		return std::ranges::any_of(a_strings, [&](const auto& str) {
			return string::icontains(name, str) || string::icontains(edid, str) || kywdForm->ContainsKeywordString(str);
		});
	}

	bool Data::has_skill_or_archetype(const std::string& a_str) const
	{
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
					return AV::GetActorValue(mgef->GetMagickSkill()) == a_str;
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
				{
					const auto magicItem = item->As<RE::MagicItem>();
					return AV::GetActorValue(magicItem->GetAssociatedSkill()) == a_str;
				}
			default:
				return false;
			}
		}

		if (ARCHETYPE::map.contains(a_str)) {
			const auto mgef = item->As<RE::EffectSetting>();
			return mgef && std::to_string(mgef->data.archetype) == a_str;
		}

		return false;
	}
}
