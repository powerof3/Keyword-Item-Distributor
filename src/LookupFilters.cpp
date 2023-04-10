#include "LookupFilters.h"

namespace Filter
{
	Data::Data(StringFilters a_stringFilters, FormFilters a_formFilters, TraitsPtr a_traits, Chance a_chance) :
		stringFilters(std::move(a_stringFilters)),
		formFilters(std::move(a_formFilters)),
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
		formID = a_item->GetFormID();
		edid = Cache::EditorID::GetEditorID(a_item);
		name = a_item->GetName();

		// STRING
		if (!stringFilters.ALL.empty() && !HasStringFilter(stringFilters.ALL, true)) {
			return false;
		}

		if (!stringFilters.NOT.empty() && HasStringFilter(stringFilters.NOT)) {
			return false;
		}

		if (!stringFilters.MATCH.empty() && !HasStringFilter(stringFilters.MATCH)) {
			return false;
		}

		if (!stringFilters.ANY.empty() && !ContainsStringFilter(stringFilters.ANY)) {
			return false;
		}

		// FORM
		if (!formFilters.ALL.empty() && !HasFormFilter(formFilters.ALL, true)) {
			return false;
		}

		if (!formFilters.NOT.empty() && HasFormFilter(formFilters.NOT)) {
			return false;
		}

		if (!formFilters.MATCH.empty() && !HasFormFilter(formFilters.MATCH)) {
			return false;
		}

		// TRAITS
		if (traits && !traits->PassFilter(a_item)) {
			return false;
		}

		return true;
	}

	bool Data::HasStringFilter(const StringVec& a_strings, bool a_all) const
	{
		if (a_all) {
			return std::ranges::all_of(a_strings, [&](const auto& str) {
				return kywdForm->HasKeywordString(str);
			});
		} else {
			auto result = std::ranges::any_of(a_strings, [&](const auto& str) {
				return string::iequals(name, str) || string::iequals(edid, str) || kywdForm->HasKeywordString(str);
			});
			switch (item->GetFormType()) {
			case RE::FormType::MagicEffect:
				{
					if (!result) {
						const auto mgef = item->As<RE::EffectSetting>();

						auto archetypeStr = std::to_string(mgef->data.archetype);
						result = std::ranges::any_of(a_strings, [&](const auto& str) {
							return archetypeStr == str;
						});

						if (!result) {
							result = has_actorvalue(mgef->GetMagickSkill(), a_strings);
						}
					}
				}
				break;
			case RE::FormType::Book:
				{
					if (!result) {
						const auto book = item->As<RE::TESObjectBOOK>();
						auto       skill = RE::ActorValue::kNone;
						if (book->TeachesSkill()) {
							skill = book->data.teaches.actorValueToAdvance;
						} else if (book->TeachesSpell() && book->data.teaches.spell) {
							skill = book->data.teaches.spell->GetAssociatedSkill();
						}
						result = has_actorvalue(skill, a_strings);
					}
				}
				break;
			case RE::FormType::AlchemyItem:
			case RE::FormType::Ingredient:
			case RE::FormType::Scroll:
			case RE::FormType::Spell:
				{
					if (!result) {
						const auto magicItem = item->As<RE::MagicItem>();
						result = has_actorvalue(magicItem->GetAssociatedSkill(), a_strings);
					}
				}
				break;
			default:
				break;
			}
			return result;
		}
	}

	bool Data::ContainsStringFilter(const StringVec& a_strings) const
	{
		return std::ranges::any_of(a_strings, [&](const auto& str) {
			return string::icontains(name, str) || string::icontains(edid, str) || kywdForm->ContainsKeywordString(str);
		});
	}

	bool Data::HasFormFilter(const FormVec& a_forms, bool all) const
	{
		const auto has_form_or_file = [&](const std::variant<RE::TESForm*, const RE::TESFile*>& a_formFile) {
			if (std::holds_alternative<RE::TESForm*>(a_formFile)) {
				const auto form = std::get<RE::TESForm*>(a_formFile);
				return form && has_form_filter(form);
			}
			if (std::holds_alternative<const RE::TESFile*>(a_formFile)) {
				const auto file = std::get<const RE::TESFile*>(a_formFile);
				return file && file->IsFormInMod(formID);
			}
			return false;
		};

		if (all) {
			return std::ranges::all_of(a_forms, has_form_or_file);
		} else {
			return std::ranges::any_of(a_forms, has_form_or_file);
		}
	}

	bool Data::has_actorvalue(RE::ActorValue a_av, const StringVec& a_strings)
	{
		if (a_av == RE::ActorValue::kNone) {
			return false;
		}
	    if (const auto avInfo = RE::ActorValueList::GetSingleton()->GetActorValue(a_av)) {
			return std::ranges::any_of(a_strings, [&](const auto& str) {
				return str == avInfo->enumName;
			});
		}
		return false;
	}

	bool Data::has_form_filter(RE::TESForm* a_formFilter) const
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
		case RE::FormType::FormList:
			{
				bool result = false;

				const auto list = a_formFilter->As<RE::BGSListForm>();
				list->ForEachForm([&](RE::TESForm& a_form) {
					if (result = has_form_filter(&a_form); result) {
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
}
