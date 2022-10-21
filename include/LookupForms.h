#pragma once

#include "Cache.h"

namespace Lookup::Forms
{
	namespace detail
	{
		inline void formID_to_form(RE::TESDataHandler* a_dataHandler, FormIDPairVec& a_formIDVec, FormVec& a_formVec, StringVec& a_stringVec, const std::string& a_path)
		{
			if (a_formIDVec.empty()) {
				return;
			}
			for (auto& [formID, modName] : a_formIDVec) {
				if (modName && !formID) {
					if (Lookup::detail::is_mod_name(*modName)) {
						if (const RE::TESFile* filterMod = a_dataHandler->LookupModByName(*modName); filterMod) {
							a_formVec.push_back(filterMod);
						} else {
							logger::error("			{} : Filter ({}) SKIP - mod not found", a_path, *modName);
						}
					} else {
						if (auto filterForm = RE::TESForm::LookupByEditorID(*modName); filterForm) {
							const auto formType = filterForm->GetFormType();
							if (Cache::FormType::IsFilter(formType)) {
								a_formVec.push_back(filterForm);

								//remove editorIDs from strings
								std::erase(a_stringVec, *modName);
							} else {
								logger::error("			{} : Filter ({}) INFO - invalid formtype ({})", a_path, *modName, formType);
							}
						} else {
							logger::error("			{} : Filter ({}) INFO - invalid editorID, treating string as name or other", a_path, *modName);
						}
					}
				} else if (formID) {
					if (g_mergeMapperInterface) {
						const auto [mergedModName, mergedFormID] = g_mergeMapperInterface->GetNewFormID(modName.value_or("").c_str(), formID.value_or(0));
						std::string conversion_log;
						if (formID.value_or(0) && mergedFormID && formID.value_or(0) != mergedFormID) {
							conversion_log = fmt::format("0x{:X}->0x{:X}", formID.value_or(0), mergedFormID);
							formID.emplace(mergedFormID);
						}
						const std::string mergedModString{ mergedModName };
						if (!(modName.value_or("").empty()) && !mergedModString.empty() && modName.value_or("") != mergedModString) {
							if (conversion_log.empty()) {
								conversion_log = fmt::format("{}->{}", modName.value_or(""), mergedModString);
							} else {
								conversion_log = fmt::format("{}~{}->{}", conversion_log, modName.value_or(""), mergedModString);
							}
							modName.emplace(mergedModName);
						}
						if (!conversion_log.empty()) {
							logger::info("\t\tFound merged: {}", conversion_log);
						}
					}
				    auto filterForm = modName ?
					                      a_dataHandler->LookupForm(*formID, *modName) :
					                      RE::TESForm::LookupByID(*formID);
					if (filterForm) {
						const auto formType = filterForm->GetFormType();
						if (Cache::FormType::IsFilter(formType)) {
							a_formVec.push_back(filterForm);
						} else {
							logger::error("			{} : Filter [0x{:X}] ({}) SKIP - invalid formtype ({})", a_path, *formID, modName.value_or(""), formType);
						}
					} else {
						logger::error("			{} : Filter [0x{:X}] ({}) SKIP - form doesn't exist", a_path, *formID, modName.value_or(""));
					}
				}
			}
		}
	}

	inline void get_forms(RE::TESDataHandler* a_dataHandler, ITEM::TYPE a_type, INIDataVec& a_INIDataVec, KeywordDataVec& a_keywordDataVec)
	{
		if (a_INIDataVec.empty()) {
			return;
		}

		logger::info("	Starting {} lookup", Cache::Item::GetType(a_type));

		for (auto& [formIDPair_ini, strings_ini, filterIDs_ini, traits_ini, chance_ini, path] : a_INIDataVec) {
			RE::BGSKeyword* keyword = nullptr;

			if (std::holds_alternative<FormIDPair>(formIDPair_ini)) {
				if (auto [formID, modName] = std::get<FormIDPair>(formIDPair_ini); formID) {
					if (g_mergeMapperInterface) {
						const auto [mergedModName, mergedFormID] = g_mergeMapperInterface->GetNewFormID(modName.value_or("").c_str(), formID.value_or(0));
						std::string conversion_log;
						if (formID.value_or(0) && mergedFormID && formID.value_or(0) != mergedFormID) {
							conversion_log = fmt::format("0x{:X}->0x{:X}", formID.value_or(0), mergedFormID);
							formID.emplace(mergedFormID);
						}
						const std::string mergedModString{ mergedModName };
						if (!(modName.value_or("").empty()) && !mergedModString.empty() && modName.value_or("") != mergedModString) {
							if (conversion_log.empty()) {
								conversion_log = fmt::format("{}->{}", modName.value_or(""), mergedModString);
							} else {
								conversion_log = fmt::format("{}~{}->{}", conversion_log, modName.value_or(""), mergedModString);
							}
							modName.emplace(mergedModName);
						}
						if (!conversion_log.empty()) {
							logger::info("\t\tFound merged: {}", conversion_log);
						}
					}
					keyword = modName ?
					              a_dataHandler->LookupForm<RE::BGSKeyword>(*formID, *modName) :
					              RE::TESForm::LookupByID<RE::BGSKeyword>(*formID);
					if (!keyword) {
						logger::error("		{} : [0x{:X}]({}) FAIL - keyword doesn't exist", path, *formID, modName.value_or(""));
					} else if (string::is_empty(keyword->GetFormEditorID())) {
						keyword = nullptr;
						logger::error("		{} : [0x{:X}] ({}) FAIL - invalid keyword editorID", path, *formID, modName.value_or(""));
					}
				}
			} else if (std::holds_alternative<std::string>(formIDPair_ini)) {
				if (auto keywordEDID = std::get<std::string>(formIDPair_ini); !keywordEDID.empty()) {
					auto& keywordArray = a_dataHandler->GetFormArray<RE::BGSKeyword>();

					auto result = std::ranges::find_if(keywordArray, [&](const auto& keyword) {
						return keyword && string::iequals(keyword->formEditorID, keywordEDID);
					});

					if (result != keywordArray.end()) {
						if (keyword = *result; keyword) {
							if (auto file = keyword->GetFile(0); file) {
								logger::info("		{} [0x{:X}~{}] INFO - using existing keyword", keywordEDID, keyword->GetLocalFormID(), file->GetFilename());
							}
						} else {
							logger::critical("		{} : {} FAIL - couldn't get existing keyword", path, keywordEDID);
						}
					} else {
						const auto factory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSKeyword>();
						if (keyword = factory ? factory->Create() : nullptr; keyword) {
							keyword->formEditorID = keywordEDID;
							keywordArray.push_back(keyword);

							logger::info("		{} INFO - creating keyword", keywordEDID, keyword->GetFormID());
						} else {
							logger::critical("		{} : {} FAIL - couldn't create keyword", path, keywordEDID);
						}
					}
				}
			}

			if (!keyword) {
				continue;
			}

			std::array<FormVec, 3> filterForms;
			for (std::uint32_t i = 0; i < filterForms.size(); i++) {
				detail::formID_to_form(a_dataHandler, filterIDs_ini[i], filterForms[i], strings_ini[i], path);
			}

			KeywordData keywordData{ keyword, strings_ini, filterForms, traits_ini, chance_ini, 0 };
			a_keywordDataVec.emplace_back(keywordData);
		}
	}

	bool GetForms();
}
