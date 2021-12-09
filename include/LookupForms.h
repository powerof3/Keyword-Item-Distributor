#pragma once

#include "Cache.h"

namespace Lookup::Forms
{
	namespace detail
	{
		inline bool formID_to_form(RE::TESDataHandler* a_dataHandler, const FormIDPairVec& a_formIDVec, FormVec& a_formVec)
		{
			if (a_formIDVec.empty()) {
				return true;
			}
			for (auto& [formID, modName] : a_formIDVec) {
				if (modName && !formID) {
					if (Lookup::detail::is_mod_name(*modName)) {
						if (const RE::TESFile* filterMod = a_dataHandler->LookupModByName(*modName); filterMod) {
							logger::info("			Filter ({}) INFO - mod found", filterMod->fileName);
							a_formVec.push_back(filterMod);
						} else {
							logger::error("			Filter ({}) SKIP - mod cannot be found", *modName);
						}
					} else {
						auto filterForm = RE::TESForm::LookupByEditorID(*modName);
						if (filterForm) {
							const auto formType = filterForm->GetFormType();
							if (const auto type = Cache::FormType::GetString(formType); !type.empty()) {
								a_formVec.push_back(filterForm);
							} else {
								logger::error("			Filter ({}) SKIP - invalid formtype ({})", *modName, formType);
							}
						} else {
							logger::error("			Filter ({}) SKIP - form doesn't exist", *modName);
						}
					}
				} else if (formID) {
					auto filterForm = modName ?
                                          a_dataHandler->LookupForm(*formID, *modName) :
                                          RE::TESForm::LookupByID(*formID);
					if (filterForm) {
						const auto formType = filterForm->GetFormType();
						if (const auto type = Cache::FormType::GetString(formType); !type.empty()) {
							a_formVec.push_back(filterForm);
						} else {
							logger::error("			Filter [0x{:X}] ({}) SKIP - invalid formtype ({})", *formID, modName.value_or(""), formType);
						}
					} else {
						logger::error("			Filter [0x{:X}] ({}) SKIP - form doesn't exist", *formID, modName.value_or(""));
					}
				}
			}
			return !a_formVec.empty();
		}
	}

	inline void get_forms(RE::TESDataHandler* a_dataHandler, const INIDataVec& a_INIDataVec, KeywordDataVec& a_keywordDataVec)
	{
		if (a_INIDataVec.empty()) {
			return;
		}

		logger::info("	Starting lookup");

		for (auto& [formIDPair_ini, strings_ini, filterIDs_ini, traits_ini, chance_ini] : a_INIDataVec) {
			RE::BGSKeyword* keyword = nullptr;

			if (std::holds_alternative<FormIDPair>(formIDPair_ini)) {
				if (auto [formID, modName] = std::get<FormIDPair>(formIDPair_ini); formID) {
					keyword = modName ?
                                  a_dataHandler->LookupForm<RE::BGSKeyword>(*formID, *modName) :
                                  RE::TESForm::LookupByID<RE::BGSKeyword>(*formID);
					if (!keyword) {
						logger::error("		Keyword [0x{:X}] ({}) doesn't exist", *formID, modName.value_or(""));
						continue;
					}
				}
			} else if (std::holds_alternative<std::string>(formIDPair_ini)) {
				if (auto keywordName = std::get<std::string>(formIDPair_ini); !keywordName.empty()) {
					auto& keywordArray = a_dataHandler->GetFormArray<RE::BGSKeyword>();

					auto result = std::ranges::find_if(keywordArray, [&](const auto& kywd) {
						return kywd && string::iequals(kywd->formEditorID, keywordName);
					});

					if (result != keywordArray.end()) {
						if (keyword = *result; keyword) {
							if (!keyword->IsDynamicForm()) {
								logger::info("		{} [0x{:X}] INFO - using existing keyword", keywordName, keyword->GetFormID());
							}
						} else {
							logger::critical("		{} FAIL - couldn't get existing keyword", keywordName);
							continue;
						}
					} else {
						const auto factory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSKeyword>();
						if (keyword = factory ? factory->Create() : nullptr; keyword) {
							keyword->formEditorID = keywordName;
							logger::info("		{} [0x{:X}] INFO - creating keyword", keywordName, keyword->GetFormID());

							keywordArray.push_back(keyword);
						} else {
							logger::critical("		{} FAIL - couldn't create keyword", keywordName);
							continue;
						}
					}
				}
			}

			bool invalidEntry = false;

			std::array<FormVec, 3> filterForms;
			for (std::uint32_t i = 0; i < filterForms.size(); i++) {
				if (!detail::formID_to_form(a_dataHandler, filterIDs_ini[i], filterForms[i])) {
					invalidEntry = true;
					break;
				}
			}

			if (invalidEntry) {
				continue;
			}

			std::uint32_t count = 0;
			KeywordData keywordData = { keyword, strings_ini, filterForms, traits_ini, chance_ini, count };
			a_keywordDataVec.emplace_back(keywordData);
		}
	}

	bool GetForms();
}
