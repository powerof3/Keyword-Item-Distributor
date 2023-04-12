#pragma once

#include "LookupConfigs.h"
#include "LookupFilters.h"

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
				buffered_logger::info("\t\tFound merged: {}", conversion_log);
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
										   buffered_logger::error("\t\tFilter ({}) SKIP - mod doesn't exist", *modName);
									   }
								   } else if (formID) {
									   if (auto filterForm = modName ?
						                                         dataHandler->LookupForm(*formID, *modName) :
						                                         RE::TESForm::LookupByID(*formID)) {
										   const auto formType = filterForm->GetFormType();
										   if (Cache::FormType::IsFilter(formType)) {
											   a_formVec.push_back(filterForm);
										   } else {
											   buffered_logger::error("\t\tFilter [0x{:X}] ({}) SKIP - invalid formtype ({})", *formID, modName.value_or(""), formType);
										   }
									   } else {
										   buffered_logger::error("\t\tFilter [0x{:X}] ({}) SKIP - form doesn't exist", *formID, modName.value_or(""));
									   }
								   }
							   },
							   [&](const std::string& editorID) {
								   if (auto filterForm = RE::TESForm::LookupByEditorID(editorID)) {
									   const auto formType = filterForm->GetFormType();
									   if (Cache::FormType::IsFilter(formType)) {
										   a_formVec.push_back(filterForm);
									   } else {
										   buffered_logger::error("\t\tFilter ({}) SKIP - invalid formtype ({})", editorID, formType);
									   }
								   } else {
									   if (auto keyword = find_existing_keyword(keywordArray, editorID, true)) {
										   a_formVec.push_back(keyword);
									   } else {
										   buffered_logger::info("\t\tFilter ({}) INFO - treating as string", editorID);
										   a_formVec.push_back(editorID);
									   }
								   }
							   } },
					formOrEditorID);
			}

			return !a_formVec.empty();
		}
	}

	struct Data
	{
		bool operator==(const Data& a_rhs) const;

		// members
		RE::BGSKeyword* keyword{ nullptr };
		FilterData      filters{};
	};

	using DataVec = std::vector<Data>;
	using CountMap = std::map<RE::BGSKeyword*, std::uint32_t>;

	template <class T>
	class Distributables
	{
	public:
		Distributables(ITEM::TYPE a_type);

		explicit                  operator bool() const;
		[[nodiscard]] std::size_t size() const;
		void                      clear();

		[[nodiscard]] ITEM::TYPE       GetType() const;
		[[nodiscard]] std::string_view GetTypeString() const;
		[[nodiscard]] const DataVec&   GetKeywords() const;
		[[nodiscard]] DataVec&         GetKeywords();
		[[nodiscard]] const CountMap&  GetKeywordCounts() const;
		void                           IncrementCount(RE::BGSKeyword* a_keyword);
		void                           LookupForms();

	private:
		ITEM::TYPE type;
		DataVec    keywords{};
		CountMap   keywordCount{};
	};

	inline Distributables<RE::TESObjectARMO>  armors{ ITEM::kArmor };
	inline Distributables<RE::TESObjectWEAP>  weapons{ ITEM::kWeapon };
	inline Distributables<RE::TESAmmo>        ammo{ ITEM::kAmmo };
	inline Distributables<RE::EffectSetting>  magicEffects{ ITEM::kMagicEffect };
	inline Distributables<RE::AlchemyItem>    potions{ ITEM::kPotion };
	inline Distributables<RE::ScrollItem>     scrolls{ ITEM::kScroll };
	inline Distributables<RE::BGSLocation>    locations{ ITEM::kLocation };
	inline Distributables<RE::IngredientItem> ingredients{ ITEM::kIngredient };
	inline Distributables<RE::TESObjectBOOK>  books{ ITEM::kBook };
	inline Distributables<RE::TESObjectMISC>  miscItems{ ITEM::kMiscItem };
	inline Distributables<RE::TESKey>         keys{ ITEM::kKey };
	inline Distributables<RE::TESSoulGem>     soulGems{ ITEM::kSoulGem };
	inline Distributables<RE::SpellItem>      spells{ ITEM::kSpell };

	template <typename Func, typename... Args>
	void ForEachDistributable(Func&& a_func, Args&&... args)
	{
		a_func(armors, std::forward<Args>(args)...);
		a_func(weapons, std::forward<Args>(args)...);
		a_func(ammo, std::forward<Args>(args)...);
		a_func(magicEffects, std::forward<Args>(args)...);
		a_func(potions, std::forward<Args>(args)...);
		a_func(scrolls, std::forward<Args>(args)...);
		a_func(locations, std::forward<Args>(args)...);
		a_func(ingredients, std::forward<Args>(args)...);
		a_func(books, std::forward<Args>(args)...);
		a_func(miscItems, std::forward<Args>(args)...);
		a_func(keys, std::forward<Args>(args)...);
		a_func(soulGems, std::forward<Args>(args)...);
		a_func(spells, std::forward<Args>(args)...);
	}

	void Clear();
}
using KeywordData = Keyword::Data;
using KeywordDataVec = Keyword::DataVec;

template <class T>
Keyword::Distributables<T>::Distributables(ITEM::TYPE a_type) :
	type(a_type)
{}

template <class T>
Keyword::Distributables<T>::operator bool() const
{
	return !keywords.empty();
}

template <class T>
std::size_t Keyword::Distributables<T>::size() const
{
	return keywords.size();
}

template <class T>
void Keyword::Distributables<T>::clear()
{
	keywords.clear();
	keywordCount.clear();
}

template <class T>
ITEM::TYPE Keyword::Distributables<T>::GetType() const
{
	return type;
}

template <class T>
std::string_view Keyword::Distributables<T>::GetTypeString() const
{
	return ITEM::GetType(type);
}

template <class T>
const KeywordDataVec& Keyword::Distributables<T>::GetKeywords() const
{
	return keywords;
}

template <class T>
KeywordDataVec& Keyword::Distributables<T>::GetKeywords()
{
	return keywords;
}

template <class T>
const Keyword::CountMap& Keyword::Distributables<T>::GetKeywordCounts() const
{
	return keywordCount;
}

template <class T>
void Keyword::Distributables<T>::IncrementCount(RE::BGSKeyword* a_keyword)
{
	++keywordCount[a_keyword];
}

template <class T>
void Keyword::Distributables<T>::LookupForms()
{
	auto& INIDataVec = INI::INIs[type];
	if (INIDataVec.empty()) {
		return;
	}

	logger::info("{}", GetTypeString());

	keywords.reserve(INIDataVec.size());

	const auto dataHandler = RE::TESDataHandler::GetSingleton();
	auto&      keywordArray = dataHandler->GetFormArray<RE::BGSKeyword>();

	for (auto& [rawForm, rawFilters, traits, chance, path] : INIDataVec) {
		RE::BGSKeyword* keyword = nullptr;

		std::visit(overload{
					   [&](FormModPair& a_formMod) {
						   auto& [formID, modName] = a_formMod;
						   if (g_mergeMapperInterface) {
							   detail::get_merged_IDs(formID, modName);
						   }
						   keyword = modName ?
			                             dataHandler->LookupForm<RE::BGSKeyword>(*formID, *modName) :
			                             RE::TESForm::LookupByID<RE::BGSKeyword>(*formID);
						   if (!keyword) {
							   buffered_logger::error("\t[{}] [0x{:X}]({}) FAIL - keyword doesn't exist", path, *formID, modName.value_or(""));
						   } else if (string::is_empty(keyword->GetFormEditorID())) {
							   keyword = nullptr;
							   buffered_logger::error("\t[{}] [0x{:X}]({}) FAIL - keyword editorID is empty!", path, *formID, modName.value_or(""));
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
								   buffered_logger::critical("\t[{}] {} FAIL - couldn't create keyword", path, a_edid);
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
			processedFilters.ANY = rawFilters.ANY;
		}

		if (!validEntry) {
			logger::error("\t\tNo filters were processed, skipping distribution");
			continue;
		}

		keywords.emplace_back(Data{ keyword, { processedFilters, traits, chance } });
	}
}
