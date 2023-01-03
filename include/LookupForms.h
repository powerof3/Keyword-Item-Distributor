#pragma once

#include "Cache.h"

namespace Lookup::Forms
{
	namespace detail
	{
		inline RE::BGSKeyword* find_existing_keyword(const RE::BSTArray<RE::BGSKeyword*>& a_keywordArray, const std::string& a_edid)
		{
			const auto result = std::ranges::find_if(a_keywordArray, [&](const auto& keywordInArray) {
				return keywordInArray && keywordInArray->formEditorID == a_edid.c_str();
			});

			return result != a_keywordArray.end() ? *result : nullptr;
		}

		inline void get_merged_IDs(std::optional<RE::FormID>& a_formID, std::optional<std::string>& a_modName)
		{
			const auto [mergedModName, mergedFormID] = g_mergeMapperInterface->GetNewFormID(a_modName.value_or("").c_str(), a_formID.value_or(0));
			std::string conversion_log{};
			if (a_formID.value_or(0) && mergedFormID && a_formID.value_or(0) != mergedFormID) {
				conversion_log = std::format("0x{:X}->0x{:X}", a_formID.value_or(0), mergedFormID);
				a_formID.emplace(mergedFormID);
			}
			const std::string mergedModString{ mergedModName };
			if (!a_modName.value_or("").empty() && !mergedModString.empty() && a_modName.value_or("") != mergedModString) {
				if (conversion_log.empty()) {
					conversion_log = std::format("{}->{}", a_modName.value_or(""), mergedModString);
				} else {
					conversion_log = std::format("{}~{}->{}", conversion_log, a_modName.value_or(""), mergedModString);
				}
				a_modName.emplace(mergedModName);
			}
			if (!conversion_log.empty()) {
				logger::info("\t\tFound merged: {}", conversion_log);
			}
		}

		inline bool formID_to_form(RE::TESDataHandler* a_dataHandler, FormIDVec& a_formIDVec, FormVec& a_formVec, StringVec& a_stringVec, const std::string& a_path)
		{
			if (a_formIDVec.empty()) {
				return true;
			}
			for (auto& formOrEditorID : a_formIDVec) {
				if (const auto formIDPair(std::get_if<FormIDPair>(&formOrEditorID)); formIDPair) {
					auto& [formID, modName] = *formIDPair;
					if (g_mergeMapperInterface) {
						get_merged_IDs(formID, modName);
					}
					if (modName && !formID) {
						if (const RE::TESFile* filterMod = a_dataHandler->LookupModByName(*modName); filterMod) {
							logger::info("\t\t\t[{}] Filter ({}) INFO - mod found", a_path, filterMod->fileName);
							a_formVec.push_back(filterMod);
						} else {
							logger::error("\t\t\t[{}] Filter ({}) SKIP - mod cannot be found", a_path, *modName);
						}
					} else if (formID) {
						if (auto filterForm = modName ?
						                          a_dataHandler->LookupForm(*formID, *modName) :
						                          RE::TESForm::LookupByID(*formID)) {
							if (const auto formType = filterForm->GetFormType(); Cache::FormType::IsFilter(formType)) {
								a_formVec.push_back(filterForm);
							} else {
								logger::error("\t\t\t[{}] Filter [0x{:X}] ({}) SKIP - invalid formtype ({})", a_path, *formID, modName.value_or(""), formType);
							}
						} else {
							logger::error("\t\t\t[{}] Filter [0x{:X}] ({}) SKIP - form doesn't exist", a_path, *formID, modName.value_or(""));
						}
					}
				} else if (std::holds_alternative<std::string>(formOrEditorID)) {
					if (auto editorID = std::get<std::string>(formOrEditorID); !editorID.empty()) {
						if (auto filterForm = RE::TESForm::LookupByEditorID(editorID); filterForm) {
							if (const auto formType = filterForm->GetFormType(); Cache::FormType::IsFilter(formType)) {
								a_formVec.push_back(filterForm);
								//remove editorIDs from strings
								std::erase(a_stringVec, editorID);
							} else {
								logger::error("\t\t\t[{}] Filter ({}) INFO - invalid formtype ({})", a_path, editorID, formType);
							}
						} else {
							if (auto keyword = find_existing_keyword(a_dataHandler->GetFormArray<RE::BGSKeyword>(), editorID)) {
								a_formVec.push_back(keyword);
								//remove editorIDs from strings
								std::erase(a_stringVec, editorID);
							} else {
								logger::error("\t\t\t[{}] Filter ({}) SKIP - form doesn't exist", a_path, editorID);
							}
						}
					}
				}
			}
			return !a_formVec.empty();
		}
	}

	inline void get_forms(RE::TESDataHandler* a_dataHandler, ITEM::TYPE a_type, INIDataVec& a_INIDataVec, KeywordDataVec& a_keywordDataVec)
	{
		if (a_INIDataVec.empty()) {
			return;
		}

		logger::info("\tStarting {} lookup", Cache::Item::GetType(a_type));

		for (auto& [formIDPair_ini, strings_ini, filterIDs_ini, traits_ini, chance_ini, path] : a_INIDataVec) {
			RE::BGSKeyword* keyword = nullptr;

			if (std::holds_alternative<FormIDPair>(formIDPair_ini)) {
				if (auto [formID, modName] = std::get<FormIDPair>(formIDPair_ini); formID) {
					if (g_mergeMapperInterface) {
						detail::get_merged_IDs(formID, modName);
					}
					keyword = modName ?
					              a_dataHandler->LookupForm<RE::BGSKeyword>(*formID, *modName) :
					              RE::TESForm::LookupByID<RE::BGSKeyword>(*formID);
					if (!keyword) {
						logger::error("\t\t[{}] [0x{:X}]({}) FAIL - keyword doesn't exist", path, *formID, modName.value_or(""));
					} else if (string::is_empty(keyword->GetFormEditorID())) {
						keyword = nullptr;
						logger::error("\t\t[{}] [0x{:X}] ({}) FAIL - invalid keyword editorID", path, *formID, modName.value_or(""));
					}
				}
			} else if (std::holds_alternative<std::string>(formIDPair_ini)) {
				if (auto keywordEDID = std::get<std::string>(formIDPair_ini); !keywordEDID.empty()) {
					auto& keywordArray = a_dataHandler->GetFormArray<RE::BGSKeyword>();

				    if (keyword = detail::find_existing_keyword(keywordArray, keywordEDID); keyword) {
						if (const auto file = keyword->GetFile(0)) {
							logger::info("\t\t[{}] {} [0x{:X}~{}] INFO - using existing keyword", path, keywordEDID, keyword->GetLocalFormID(), file->GetFilename());
						} else {
							logger::critical("\t\t[{}] {} INFO - using created keyword", path, keywordEDID);
						}
					} else {
						const auto factory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSKeyword>();
						if (keyword = factory ? factory->Create() : nullptr; keyword) {
							keyword->formEditorID = keywordEDID;
							keywordArray.push_back(keyword);

							logger::info("\t\t[{}] {} INFO - creating keyword", path, keywordEDID, keyword->GetFormID());
						} else {
							logger::critical("\t\t[{}] {} FAIL - couldn't create keyword", path, keywordEDID);
						}
					}
				}
			}

			if (!keyword) {
				continue;
			}

			bool invalidEntry = false;

			std::array<FormVec, 3> filterForms{};
			for (std::uint32_t i = 0; i < filterForms.size(); i++) {
				if (!detail::formID_to_form(a_dataHandler, filterIDs_ini[i], filterForms[i], strings_ini[i], path)) {
					invalidEntry = true;
					break;
				}
			}

			if (invalidEntry) {
				continue;
			}

			a_keywordDataVec.emplace_back(KeywordData{ keyword, strings_ini, filterForms, traits_ini, chance_ini, 0 });
		}
	}

	bool GetForms();
}
