#include "KeywordData.h"

namespace Keyword
{
	namespace detail
	{
		inline RE::BGSKeyword* find_existing_keyword(const RE::BSTArray<RE::BGSKeyword*>& a_keywordArray, const std::string& a_edid, bool a_skipEDID = false)
		{
			if (!a_skipEDID) {
				if (const auto keyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>(a_edid)) {
					return keyword;
				}
			}

			// iterate from last added (most likely to be runtime)
			const auto rBegin = std::reverse_iterator(a_keywordArray.end());
			const auto rEnd = std::reverse_iterator(a_keywordArray.begin());

			const auto result = std::find_if(rBegin, rEnd, [&](const auto& keywordInArray) {
				return keywordInArray->formEditorID == a_edid.c_str();
			});

			return result != rEnd ? *result : nullptr;
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

		inline bool formID_to_form(RawVec& a_rawFormVec, ProcessedVec& a_formVec)
		{
			if (a_rawFormVec.empty()) {
				return true;
			}

			const auto  dataHandler = RE::TESDataHandler::GetSingleton();
			const auto& keywordArray = dataHandler->GetFormArray<RE::BGSKeyword>();

			for (auto& formOrEditorID : a_rawFormVec) {
				std::visit(overload{
							   [&](FormModPair& formMod) {
								   auto& [formID, modName] = formMod;
								   if (g_mergeMapperInterface) {
									   get_merged_IDs(formID, modName);
								   }
								   if (modName && !formID) {
									   if (const RE::TESFile* filterMod = dataHandler->LookupModByName(*modName)) {
										   a_formVec.push_back(filterMod);
									   } else {
										   logger::error("\t\tFilter ({}) SKIP - mod doesn't exist", *modName);
									   }
								   } else if (formID) {
									   if (auto filterForm = modName ?
						                                         dataHandler->LookupForm(*formID, *modName) :
						                                         RE::TESForm::LookupByID(*formID)) {
										   const auto formType = filterForm->GetFormType();
										   if (Cache::FormType::IsFilter(formType)) {
											   a_formVec.push_back(filterForm);
										   } else {
											   logger::error("\t\tFilter [0x{:X}] ({}) SKIP - invalid formtype ({})", *formID, modName.value_or(""), formType);
										   }
									   } else {
										   logger::error("\t\tFilter [0x{:X}] ({}) SKIP - form doesn't exist", *formID, modName.value_or(""));
									   }
								   }
							   },
							   [&](const std::string& editorID) {
								   if (auto filterForm = RE::TESForm::LookupByEditorID(editorID)) {
									   const auto formType = filterForm->GetFormType();
									   if (Cache::FormType::IsFilter(formType)) {
										   a_formVec.push_back(filterForm);
									   } else {
										   logger::error("\t\tFilter ({}) SKIP - invalid formtype ({})", editorID, formType);
									   }
								   } else {
									   if (auto keyword = find_existing_keyword(keywordArray, editorID, true)) {
										   a_formVec.push_back(keyword);
									   } else {
										   logger::info("\t\tFilter ({}) INFO - treating as string", editorID);
										   a_formVec.push_back(editorID);
									   }
								   }
							   } },
					formOrEditorID);
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

	void Distributables::LookupForms(RE::TESDataHandler* a_dataHandler, ITEM::TYPE a_type, INI::DataVec& a_INIDataVec)
	{
		if (a_INIDataVec.empty()) {
			return;
		}

		logger::info("{}", GetType(a_type));

		keywords.reserve(a_INIDataVec.size());

		auto& keywordArray = a_dataHandler->GetFormArray<RE::BGSKeyword>();

		for (auto& [rawForm, rawFilters, traits, chance, path] : a_INIDataVec) {
			RE::BGSKeyword* keyword = nullptr;

			std::visit(overload{
						   [&](FormModPair& a_formMod) {
							   auto& [formID, modName] = a_formMod;
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
								   logger::error("\t[{}] [0x{:X}]({}) FAIL - keyword editorID is empty!", path, *formID, modName.value_or(""));
							   }
						   },
						   [&](const std::string& a_edid) {
							   keyword = detail::find_existing_keyword(keywordArray, a_edid);
							   if (!keyword) {
								   const auto factory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSKeyword>();
								   if (keyword = factory ? factory->Create() : nullptr; keyword) {
									   keyword->formEditorID = a_edid;
									   keywordArray.push_back(keyword);
								   } else {
									   logger::critical("\t[{}] {} FAIL - couldn't create keyword", path, a_edid);
								   }
							   }
						   },
					   },
				rawForm);

			if (!keyword) {
				continue;
			}

			logger::info("\t[{}] {}", path, keyword->GetFormEditorID());

			ProcessedFilters processedFilters{};

			bool validEntry = detail::formID_to_form(rawFilters.ALL, processedFilters.ALL);
			if (validEntry) {
				validEntry = detail::formID_to_form(rawFilters.NOT, processedFilters.NOT);
			}
			if (validEntry) {
				validEntry = detail::formID_to_form(rawFilters.MATCH, processedFilters.MATCH);
			}
			if (validEntry) {
				processedFilters.ANY = std::move(rawFilters.ANY);
			}

			if (!validEntry) {
				logger::error("\t\tNo filters were processed, skipping distribution");
			    continue;
			}

			keywords.emplace_back(0, keyword, FilterData{ processedFilters, std::move(traits), chance });
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
