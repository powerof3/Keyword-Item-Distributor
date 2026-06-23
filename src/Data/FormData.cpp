#include "Data/FormData.h"

#include "Data\BaseFilters.h"
#include "RE.h"

RawForm::RawForm(const std::string& a_str)
{
	if (const auto splitID = string::split(a_str, "~"); splitID.size() == 2) {
		formID = string::to_num<RE::FormID>(splitID[0], true);
		modNameOrEDID = splitID[1];
	} else if (string::is_only_hex(a_str, true)) {
		formID = string::to_num<RE::FormID>(a_str, true);
	} else {
		modNameOrEDID = a_str;
	}
	isMod = !formID && modNameOrEDID && IsMod(*modNameOrEDID);
}

std::string RawForm::to_string() const
{
	if (formID && modNameOrEDID) {
		return std::format("0x{:X}~{}", *formID, *modNameOrEDID);
	} else if (formID) {
		return std::format("0x{:X}", *formID);
	} else if (modNameOrEDID) {
		return *modNameOrEDID;
	} else {
		return std::string{};
	}
}

RawForm RawForm::GetMergedIDs() const
{
	RawForm mergedIDs = *this;

	if (g_mergeMapperInterface) {
		const auto [mergedModName, mergedFormID] = g_mergeMapperInterface->GetNewFormID(modNameOrEDID.value_or("").c_str(), formID.value_or(0));
		std::string conversion_log{};
		if (formID.value_or(0) && mergedFormID && formID.value_or(0) != mergedFormID) {
			conversion_log = std::format("0x{:X}->0x{:X}", formID.value_or(0), mergedFormID);
			mergedIDs.formID.emplace(mergedFormID);
		}
		const std::string mergedModString{ mergedModName };
		if (!modNameOrEDID.value_or("").empty() && !mergedModString.empty() && modNameOrEDID.value_or("") != mergedModString) {
			if (conversion_log.empty()) {
				conversion_log = std::format("{}->{}", modNameOrEDID.value_or(""), mergedModString);
			} else {
				conversion_log = std::format("{}~{}->{}", conversion_log, modNameOrEDID.value_or(""), mergedModString);
			}
			mergedIDs.modNameOrEDID.emplace(mergedModName);
		}
		if (!conversion_log.empty()) {
			buffered_logger::info("\t\tFound merged: {}", conversion_log);
		}
	}

	return mergedIDs;
}

RE::BGSKeyword* RawForm::to_keyword() const
{
	RE::BGSKeyword* keyword = nullptr;

	auto [mIsMod, mFormID, mModName] = GetMergedIDs();
	auto dataHandler = RE::TESDataHandler::GetSingleton();

	if (mFormID) {
		keyword = mModName ?
		              dataHandler->LookupForm<RE::BGSKeyword>(*mFormID, *mModName) :
		              RE::TESForm::LookupByID<RE::BGSKeyword>(*mFormID);
	} else if (mModName) {
		keyword = Cache::FindKeyword(*mModName);
		if (!keyword) {
			const auto factory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSKeyword>();
			if (keyword = factory ? factory->Create() : nullptr; keyword) {
				keyword->formEditorID = *mModName;
				Cache::AddKeyword(dataHandler->GetFormArray<RE::BGSKeyword>(), keyword);
			}
		}
	}

	return keyword;
}

ResolvedFilter::ResolvedFilter(const RawForm& a_id)
{
	auto [mIsMod, mFormID, mModName] = a_id.GetMergedIDs();
	auto dataHandler = RE::TESDataHandler::GetSingleton();

	if (mFormID) {
		filter = mModName ?
		                dataHandler->LookupForm(*mFormID, *mModName) :
		                RE::TESForm::LookupByID(*mFormID);
	} else if (mModName) {
		if (mIsMod) {
			filter = dataHandler->LookupModByName(*mModName);
		} else {
			if (auto filterForm = RE::TESForm::LookupByEditorID(*mModName)) {
				filter = filterForm;
			} else {
				if (auto keyword = Cache::FindKeyword(*mModName)) {
					filter = keyword;
				} else {
					filter = *mModName; // string value
				}
			}
		}
	}

	constexpr auto is_filter_whitelisted = [](RE::TESForm* a_form) {
		switch (a_form->GetFormType()) {
		case RE::FormType::Armor:
		case RE::FormType::Weapon:
		case RE::FormType::Ammo:
		case RE::FormType::MagicEffect:
		case RE::FormType::AlchemyItem:
		case RE::FormType::Scroll:
		case RE::FormType::Location:
		case RE::FormType::Ingredient:
		case RE::FormType::Book:
		case RE::FormType::Misc:
		case RE::FormType::KeyMaster:
		case RE::FormType::SoulGem:
		case RE::FormType::Activator:
		case RE::FormType::Flora:
		case RE::FormType::Furniture:
		case RE::FormType::Race:
		case RE::FormType::TalkingActivator:
		case RE::FormType::Enchantment:
		case RE::FormType::EffectShader:
		case RE::FormType::ReferenceEffect:
		case RE::FormType::ArtObject:
		case RE::FormType::MusicType:
		case RE::FormType::Faction:
		case RE::FormType::Keyword:
		case RE::FormType::Spell:  // also type
		case RE::FormType::Projectile:
		case RE::FormType::EquipSlot:
		case RE::FormType::VoiceType:
		case RE::FormType::LeveledItem:
		case RE::FormType::Water:
		case RE::FormType::Perk:
		case RE::FormType::FormList:
			return true;
		default:
			return false;
		}
	};

	std::visit(overload{
				   [&](RE::TESForm* form) {
					   if (form) {
						   formType = form->GetFormType();
						   isValid = is_filter_whitelisted(form);
					   } else {
						   isValid = false;
					   }
				   },
				   [&](const RE::TESFile* mod) {
					   isValid = mod != nullptr;
				   },
				   [&](FullString&) {
					   isValid = true;
				   } },
		filter);
}

RE::TESForm* ResolvedFilter::GetForm() const
{
	if (auto formPtr = std::get_if<RE::TESForm*>(&filter); formPtr) {
		return (*formPtr);
	}
	return nullptr;
}

RE::TESForm* ResolvedFilter::GetDependentForm() const
{
	if (auto form = GetForm()) {
		return form;
	}
	return nullptr;
}

std::string ResolvedFilter::to_string() const
{
	return std::visit(overload{
						  [](const RE::TESForm* a_form) { return a_form ? a_form->IsDynamicForm() ?
		                                                                  a_form->GetFormEditorID() :
		                                                                  std::format("{}~{}", a_form->GetLocalFormID(), a_form->GetFile(0)->fileName) :
		                                                                  std::string{}; },
						  [](const RE::TESFile* a_file) { return a_file ? a_file->fileName : std::string{}; },
						  [](const FullString& a_str) { return a_str; } },
		filter);
}
