#pragma once

#include "Cache.h"

namespace Filter
{
	namespace detail
	{
		namespace strings
		{
			inline bool contains(const std::string& a_name, const StringVec& a_strings)
			{
				return std::ranges::any_of(a_strings, [&](const auto& str) {
					return string::icontains(a_name, str);
				});
			}

			inline bool matches(const std::string& a_name, const StringVec& a_strings)
			{
				return std::ranges::any_of(a_strings, [&](const auto& str) {
					return string::iequals(a_name, str);
				});
			}
		}

		namespace form
		{
			inline bool get_type(RE::TESForm* a_item, RE::TESForm* a_filter)
			{
				switch (a_filter->GetFormType()) {
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
					return a_item == a_filter;
				case RE::FormType::Location:
					{
						const auto loc = a_item->As<RE::BGSLocation>();
						const auto filterLoc = a_filter->As<RE::BGSLocation>();

						return loc && filterLoc && (loc == filterLoc || loc->IsParent(filterLoc));
					}
				case RE::FormType::Projectile:
					{
						const auto ammo = a_item->As<RE::TESAmmo>();
						const auto filterProj = a_filter->As<RE::BGSProjectile>();

						return ammo && ammo->data.projectile == filterProj;
					}
				case RE::FormType::MagicEffect:
					{
						const auto mgef = a_filter->As<RE::EffectSetting>();

						if (const auto spell = a_item->As<RE::MagicItem>(); spell) {
							return std::ranges::any_of(spell->effects, [&](const auto& effect) {
								return effect && effect->baseEffect == mgef;
							});
						}

						return a_item == mgef;
					}
				case RE::FormType::EffectShader:
					{
						const auto mgef = a_item->As<RE::EffectSetting>();
						return mgef && (mgef->data.effectShader == a_filter || mgef->data.enchantShader == a_filter);
					}
				case RE::FormType::ReferenceEffect:
					{
						const auto mgef = a_item->As<RE::EffectSetting>();
						return mgef && (mgef->data.hitVisuals == a_filter || mgef->data.enchantVisuals == a_filter);
					}
				case RE::FormType::ArtObject:
					{
						const auto mgef = a_item->As<RE::EffectSetting>();
						return mgef && (mgef->data.castingArt == a_filter || mgef->data.hitEffectArt == a_filter || mgef->data.enchantEffectArt == a_filter);
					}
				case RE::FormType::MusicType:
					{
						const auto loc = a_item->As<RE::BGSLocation>();
						return loc && loc->musicType == a_filter;
					}
				case RE::FormType::Faction:
					{
						const auto loc = a_item->As<RE::BGSLocation>();
						return loc && loc->unreportedCrimeFaction == a_filter;
					}
				case RE::FormType::Spell:
					{
						const auto book = a_item->As<RE::TESObjectBOOK>();
						return book && book->GetSpell() == a_filter;
					}
				case RE::FormType::Keyword:
					{
						const auto keyword = a_filter->As<RE::BGSKeyword>();
						const auto keywordForm = a_item->As<RE::BGSKeywordForm>();

						return keywordForm && keywordForm->HasKeyword(keyword);
					}
				case RE::FormType::FormList:
					{
						bool result = false;

						const auto list = a_filter->As<RE::BGSListForm>();
						list->ForEachForm([&](RE::TESForm& a_form) {
							if (result = get_type(a_item, &a_form); result) {
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

			inline bool matches(RE::TESForm& a_item, const FormVec& a_forms, bool a_matchesAll = false)
			{
				const auto get_form_or_file = [&a_item](const auto& a_formFile) {
					if (std::holds_alternative<RE::TESForm*>(a_formFile)) {
						auto form = std::get<RE::TESForm*>(a_formFile);
						return form && get_type(&a_item, form);
					}
					if (std::holds_alternative<const RE::TESFile*>(a_formFile)) {
						auto file = std::get<const RE::TESFile*>(a_formFile);
						return file && file->IsFormInMod(a_item.GetFormID());
					}
					return false;
				};

				if (a_matchesAll) {
					return std::ranges::all_of(a_forms, get_form_or_file);
				} else {
					return std::ranges::any_of(a_forms, get_form_or_file);
				}
			}
		}

		namespace keyword
		{
			inline bool contains(RE::TESForm& a_item, const StringVec& a_strings)
			{
				return std::ranges::any_of(a_strings, [&a_item](const auto& str) {
					auto keywordForm = a_item.As<RE::BGSKeywordForm>();
					return keywordForm && keywordForm->ContainsKeywordString(str);
				});
			}

			inline bool matches(RE::TESForm& a_item, const StringVec& a_strings, bool a_matchesAll = false)
			{
				const auto has_keyword = [&a_item](const auto& str) {
					auto keywordForm = a_item.As<RE::BGSKeywordForm>();
					return keywordForm && keywordForm->HasKeywordString(str);
				};

				if (a_matchesAll) {
					return std::ranges::all_of(a_strings, has_keyword);
				} else {
					return std::ranges::any_of(a_strings, has_keyword);
				}
			}
		}

		namespace actorvalue
		{
			inline bool matches(const RE::ActorValue a_skill, const StringVec& a_strings)
			{
				if (a_skill == RE::ActorValue::kNone) {
					return false;
				}

				const auto avInfo = RE::ActorValueList::GetSingleton()->GetActorValue(a_skill);
				if (!avInfo) {
					return false;
				}

				return std::ranges::any_of(a_strings, [&](const auto& str) {
					return string::iequals(avInfo->enumName, str);
				});
			}

			inline bool matches(const RE::SpellItem* a_spell, const StringVec& a_strings)
			{
				if (!a_spell) {
					return false;
				}

				const auto avInfo = RE::ActorValueList::GetSingleton()->GetActorValue(a_spell->GetAssociatedSkill());
				if (!avInfo) {
					return false;
				}

				return std::ranges::any_of(a_strings, [&](const auto& str) {
					return string::iequals(avInfo->enumName, str);
				});
			}
		}
	}

	template <class T>
	bool strings(T& a_item, const KeywordData& a_keywordData)
	{
		auto& [strings_ALL, strings_NOT, strings_MATCH, strings_ANY] = std::get<DATA::kStrings>(a_keywordData);

		if (!strings_ALL.empty() && !detail::keyword::matches(a_item, strings_ALL, true)) {
			return false;
		}

		const std::string name = a_item.GetName();
		const std::string editorID = Cache::EditorID::GetEditorID(&a_item);

		const auto get_match = [&](const StringVec& a_strings) {
			bool result = false;
			if (!name.empty() && detail::strings::matches(name, a_strings)) {
				result = true;
			}
			if (!result && detail::strings::matches(editorID, a_strings)) {
				result = true;
			}
			if (!result && detail::keyword::matches(a_item, a_strings)) {
				result = true;
			}
			if constexpr (std::is_same_v<T, RE::EffectSetting>) {
				if (!result && Cache::Archetype::Matches(a_item.data.archetype, a_strings)) {
					result = true;
				}
				if (!result && detail::actorvalue::matches(a_item.GetMagickSkill(), a_strings)) {
					result = true;
				}
			} else if constexpr (std::is_same_v<T, RE::TESObjectBOOK>) {
				if (!result && detail::actorvalue::matches(a_item.GetSkill(), a_strings)) {
					result = true;
				}
				if (!result && detail::actorvalue::matches(a_item.GetSpell(), a_strings)) {
					result = true;
				}
			}
			return result;
		};

		const auto get_match_Any = [&](const StringVec& a_strings) {
			bool result = false;
			if (!name.empty() && detail::strings::contains(name, a_strings)) {
				result = true;
			}
			if (!result && detail::strings::contains(editorID, a_strings)) {
				result = true;
			}
			if (!result && detail::keyword::contains(a_item, a_strings)) {
				result = true;
			}
			return result;
		};

		if (!strings_NOT.empty() && get_match(strings_NOT)) {
			return false;
		}
		if (!strings_MATCH.empty() && !get_match(strings_MATCH)) {
			return false;
		}
		if (!strings_ANY.empty() && !get_match_Any(strings_ANY)) {
			return false;
		}

		return true;
	}

	inline bool forms(RE::TESForm& a_item, const KeywordData& a_keywordData)
	{
		auto& [filterForms_ALL, filterForms_NOT, filterForms_MATCH] = std::get<DATA::kFilters>(a_keywordData);

		if (!filterForms_ALL.empty() && !detail::form::matches(a_item, filterForms_ALL, true)) {
			return false;
		}

		if (!filterForms_NOT.empty() && detail::form::matches(a_item, filterForms_NOT)) {
			return false;
		}

		if (!filterForms_MATCH.empty() && !detail::form::matches(a_item, filterForms_MATCH)) {
			return false;
		}

		return true;
	}

	template <class T>
	bool secondary(T& a_item, const KeywordData& a_keywordData)
	{
		const auto& traits = std::get<DATA::kTraits>(a_keywordData);

		if constexpr (std::is_same_v<T, RE::TESObjectARMO>) {
			const auto& [enchanted, templated, ARValue, armorType] = std::get<TRAITS::kArmor>(traits);
			if (enchanted && (a_item.formEnchanting != nullptr) != *enchanted) {
				return false;
			}
			if (templated && (a_item.templateArmor != nullptr) != *templated) {
				return false;
			}
			if (ARValue) {
				auto& [min, max] = *ARValue;
				auto AR = a_item.GetArmorRating();

				if (min < RE::NI_INFINITY && max < RE::NI_INFINITY) {
					if (AR < min || AR > max) {
						return false;
					}
				} else if (min < RE::NI_INFINITY && AR < min) {
					return false;
				} else if (max < RE::NI_INFINITY && AR > max) {
					return false;
				}
			}
			if (armorType && a_item.GetArmorType() != *armorType) {
				return false;
			}
		} else if constexpr (std::is_same_v<T, RE::TESObjectWEAP>) {
			const auto& [enchanted, templated, weightValue] = std::get<TRAITS::kWeapon>(traits);
			if (enchanted && (a_item.formEnchanting != nullptr) != *enchanted) {
				return false;
			}
			if (templated && (a_item.templateWeapon != nullptr) != *templated) {
				return false;
			}
			if (weightValue) {
				auto& [min, max] = weightValue.value();
				auto weight = a_item.weight;

				if (min < RE::NI_INFINITY && max < RE::NI_INFINITY) {
					if (weight < min || weight > max) {
						return false;
					}
				} else if (min < RE::NI_INFINITY && weight < min) {
					return false;
				} else if (max < RE::NI_INFINITY && weight > max) {
					return false;
				}
			}
		} else if constexpr (std::is_same_v<T, RE::TESAmmo>) {
			const auto& isBolt = std::get<TRAITS::kAmmo>(traits);
			if (isBolt && a_item.IsBolt() != *isBolt) {
				return false;
			}
		} else if constexpr (std::is_same_v<T, RE::EffectSetting>) {
			const auto& [isHostile, castingType, deliveryType, skillValue] = std::get<TRAITS::kMagicEffect>(traits);
			if (isHostile && a_item.IsHostile() != *isHostile) {
				return false;
			}
			if (castingType && a_item.data.castingType != *castingType) {
				return false;
			}
			if (deliveryType && a_item.data.delivery != *deliveryType) {
				return false;
			}
			if (skillValue) {
				auto& [skill, minMax] = *skillValue;
				auto& [min, max] = minMax;

				if (skill != a_item.GetMagickSkill()) {
					return false;
				}

				auto minSkill = a_item.GetMinimumSkillLevel();

				if (min < INT_MAX && max < INT_MAX) {
					if (minSkill < min || minSkill > max) {
						return false;
					}
				} else if (min < INT_MAX && minSkill < min) {
					return false;
				} else if (max < INT_MAX && minSkill > max) {
					return false;
				}
			}
		} else if constexpr (std::is_same_v<T, RE::AlchemyItem>) {
			const auto& [isPoison, isFood] = std::get<TRAITS::kPotion>(traits);
			if (isPoison && a_item.IsPoison() != *isPoison) {
				return false;
			}
			if (isFood && a_item.IsFood() != *isFood) {
				return false;
			}
		} else if constexpr (std::is_same_v<T, RE::IngredientItem>) {
			const auto& isFood = std::get<TRAITS::kIngredient>(traits);
			if (isFood && a_item.IsFood() != *isFood) {
				return false;
			}
		} else if constexpr (std::is_same_v<T, RE::TESObjectBOOK>) {
			const auto& [spell, skill, av] = std::get<TRAITS::kBook>(traits);
			if (spell && a_item.TeachesSpell() != *spell) {
				return false;
			}
			if (skill && a_item.TeachesSkill() != *skill) {
				return false;
			}
			if (av) {
				const auto taughtSpell = a_item.GetSpell();
				if (a_item.GetSkill() != *av && (taughtSpell && taughtSpell->GetAssociatedSkill() != *av)) {
					return false;
				}
			}
		} else if constexpr (std::is_same_v<T, RE::TESSoulGem>) {
			const auto& [black, soulSize, gemSize] = std::get<TRAITS::kSoulGem>(traits);
			if (black && a_item.CanHoldNPCSoul() != *black) {
				return false;
			}
			if (soulSize && a_item.GetContainedSoul() != *soulSize) {
				return false;
			}
			if (gemSize && a_item.GetMaximumCapacity() != *gemSize) {
				return false;
			}
		}

		const auto chance = std::get<DATA::kChance>(a_keywordData);

		if (!numeric::essentially_equal(chance, 100.0f)) {
			if (RNG::GetSingleton()->Generate(0.0f, 100.0f) > chance) {
				return false;
			}
		}

		return true;
	}
}
