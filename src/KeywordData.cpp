#include "KeywordData.h"

namespace Keyword
{
    namespace detail
	{
		inline RE::BGSKeyword* find_existing_keyword(const RE::BSTArray<RE::BGSKeyword*>& a_keywordArray, const std::string& a_edid)
		{
			const auto result = std::ranges::find_if(a_keywordArray, [&](const auto& keywordInArray) {
				return keywordInArray->formEditorID == a_edid.c_str();
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

		inline bool formID_to_form(RE::TESDataHandler* a_dataHandler, RawFormVec& a_rawFormVec, FormVec& a_formVec, StringVec& a_stringVec, const std::string& a_path)
		{
			if (a_rawFormVec.empty()) {
				return true;
			}
			for (auto& formOrEditorID : a_rawFormVec) {
				if (const auto formModPair(std::get_if<FormModPair>(&formOrEditorID)); formModPair) {
					auto& [formID, modName] = *formModPair;
					if (g_mergeMapperInterface) {
						get_merged_IDs(formID, modName);
					}
					if (modName && !formID) {
						if (const RE::TESFile* filterMod = a_dataHandler->LookupModByName(*modName); filterMod) {
							logger::info("\t\t[{}] Filter ({}) INFO - mod found", a_path, filterMod->fileName);
							a_formVec.push_back(filterMod);
						} else {
							logger::error("\t\t[{}] Filter ({}) SKIP - mod cannot be found", a_path, *modName);
						}
					} else if (formID) {
						auto filterForm = modName ?
						                      a_dataHandler->LookupForm(*formID, *modName) :
						                      RE::TESForm::LookupByID(*formID);
						if (filterForm) {
							const auto formType = filterForm->GetFormType();
							if (Cache::FormType::IsFilter(formType)) {
								a_formVec.push_back(filterForm);
							} else if (formType == RE::FormType::Keyword) {
								a_stringVec.push_back(filterForm->GetFormEditorID());
								return true;
							} else {
								logger::error("\t\t[{}] Filter [0x{:X}] ({}) SKIP - invalid formtype ({})", a_path, *formID, modName.value_or(""), formType);
							}
						} else {
							logger::error("\t\t[{}] Filter [0x{:X}] ({}) SKIP - form doesn't exist", a_path, *formID, modName.value_or(""));
						}
					}
				} else if (auto editorID = std::get<std::string>(formOrEditorID); !editorID.empty()) {
					if (auto filterForm = RE::TESForm::LookupByEditorID(editorID)) {
						const auto formType = filterForm->GetFormType();
						if (Cache::FormType::IsFilter(formType)) {
							a_formVec.push_back(filterForm);
							// remove filter editorIDs from strings (filters will handle it)
							std::erase(a_stringVec, editorID);
						} else if (filterForm->IsNot(RE::FormType::Keyword)) {
							logger::error("\t\t[{}] Filter ({}) INFO - invalid formtype ({})", a_path, editorID, formType);
						} else {
							// keyword found, let strings handle it
						    return true;
						}
					} else {
						// runtime form
						if (find_existing_keyword(a_dataHandler->GetFormArray<RE::BGSKeyword>(), editorID)) {
							// keyword found, let strings handle it
							return true;
						} else {
							logger::error("\t\t[{}] Filter ({}) SKIP - form doesn't exist", a_path, editorID);
						}
					}
				}
			}
			return !a_formVec.empty();
		}
	}

	Distributables::operator bool() const
	{
		return !keywords.empty();
	}

	std::size_t Distributables::size() const
	{
		return keywords.size();
	}

	void Distributables::clear()
	{
		keywords.clear();
	}

	DataVec& Distributables::GetKeywords()
	{
		return keywords;
	}

	void Distributables::LookupForms(RE::TESDataHandler* a_dataHandler, std::string_view a_type, INI::DataVec& a_INIDataVec)
	{
		if (a_INIDataVec.empty()) {
			return;
		}

		logger::info("Starting {} lookup", a_type);

		keywords.reserve(a_INIDataVec.size());

		for (auto& [formOrEditorID, stringFilters, rawFormFilters, traits, chance, path] : a_INIDataVec) {
			RE::BGSKeyword* keyword = nullptr;

			if (std::holds_alternative<FormModPair>(formOrEditorID)) {
				if (auto [formID, modName] = std::get<FormModPair>(formOrEditorID); formID) {
					if (g_mergeMapperInterface) {
						detail::get_merged_IDs(formID, modName);
					}
					keyword = modName ?
					              a_dataHandler->LookupForm<RE::BGSKeyword>(*formID, *modName) :
					              RE::TESForm::LookupByID<RE::BGSKeyword>(*formID);
					if (!keyword) {
						logger::error("\t[{}] [0x{:X}]({}) FAIL - keyword doesn't exist", path, *formID, modName.value_or(""));
					} else if (string::is_empty(keyword->GetFormEditorID())) {
						keyword = nullptr;
						logger::error("\t[{}] [0x{:X}] ({}) FAIL - invalid keyword editorID", path, *formID, modName.value_or(""));
					}
				}
			} else if (std::holds_alternative<std::string>(formOrEditorID)) {
				if (auto keywordEDID = std::get<std::string>(formOrEditorID); !keywordEDID.empty()) {
					auto& keywordArray = a_dataHandler->GetFormArray<RE::BGSKeyword>();

					if (keyword = detail::find_existing_keyword(keywordArray, keywordEDID); keyword) {
						if (const auto file = keyword->GetFile(0)) {
							logger::info("\t[{}] {} [0x{:X}~{}] INFO - using existing keyword", path, keywordEDID, keyword->GetLocalFormID(), file->GetFilename());
						} else {
							logger::critical("\t[{}] {} INFO - using created keyword", path, keywordEDID);
						}
					} else {
						const auto factory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSKeyword>();
						if (keyword = factory ? factory->Create() : nullptr; keyword) {
							keyword->formEditorID = keywordEDID;
							keywordArray.push_back(keyword);

							logger::info("\t[{}] {} INFO - creating keyword", path, keywordEDID, keyword->GetFormID());
						} else {
							logger::critical("\t[{}] {} FAIL - couldn't create keyword", path, keywordEDID);
						}
					}
				}
			}

			if (!keyword) {
				continue;
			}

			FormFilters filterForms{};

			bool validEntry = detail::formID_to_form(a_dataHandler, rawFormFilters.ALL, filterForms.ALL, stringFilters.ALL, path);
			if (validEntry) {
				validEntry = detail::formID_to_form(a_dataHandler, rawFormFilters.NOT, filterForms.NOT, stringFilters.NOT, path);
			}
			if (validEntry) {
				validEntry = detail::formID_to_form(a_dataHandler, rawFormFilters.MATCH, filterForms.MATCH, stringFilters.MATCH, path);
			}

			if (!validEntry) {
				continue;
			}

			keywords.emplace_back(0, keyword, FilterData{ stringFilters, filterForms, std::move(traits), chance });
		}
	}

	void Clear()
	{
		armors.clear();
		weapons.clear();
		ammo.clear();
		magicEffects.clear();
		potions.clear();
		scrolls.clear();
		locations.clear();
		ingredients.clear();
		books.clear();
		miscItems.clear();
		keys.clear();
		soulGems.clear();
		spells.clear();
	}
}
