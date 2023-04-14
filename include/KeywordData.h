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
							   [&](std::string& editorID) {
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
										   if (string::icontains(editorID, ".nif")) {
											   Filter::SanitizePath(editorID);
										   } else {
											   buffered_logger::info("\t\tFilter ({}) INFO - treating as string", editorID);
										   }
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
		std::uint32_t   count{ 0 };
		RE::BGSKeyword* keyword{ nullptr };
		FilterData      filters{};
	};

	using DataVec = std::vector<Data>;
	using CountMap = std::map<RE::BGSKeyword*, std::uint32_t>;

	template <class T>
	class Distributable
	{
	public:
		Distributable(ITEM::TYPE a_type);

		explicit                  operator bool() const;
		bool                      empty() const;
		[[nodiscard]] std::size_t size() const;
		void                      clear();

		[[nodiscard]] ITEM::TYPE       GetType() const;
		[[nodiscard]] std::string_view GetTypeString() const;
		[[nodiscard]] const DataVec&   GetKeywords() const;
		[[nodiscard]] DataVec&         GetKeywords();
		void                           LookupForms();

	private:
		ITEM::TYPE type;
		DataVec    keywords{};
	};

	inline Distributable<RE::TESObjectARMO>       armors{ ITEM::kArmor };
	inline Distributable<RE::TESObjectWEAP>       weapons{ ITEM::kWeapon };
	inline Distributable<RE::TESAmmo>             ammo{ ITEM::kAmmo };
	inline Distributable<RE::EffectSetting>       magicEffects{ ITEM::kMagicEffect };
	inline Distributable<RE::AlchemyItem>         potions{ ITEM::kPotion };
	inline Distributable<RE::ScrollItem>          scrolls{ ITEM::kScroll };
	inline Distributable<RE::BGSLocation>         locations{ ITEM::kLocation };
	inline Distributable<RE::IngredientItem>      ingredients{ ITEM::kIngredient };
	inline Distributable<RE::TESObjectBOOK>       books{ ITEM::kBook };
	inline Distributable<RE::TESObjectMISC>       miscItems{ ITEM::kMiscItem };
	inline Distributable<RE::TESKey>              keys{ ITEM::kKey };
	inline Distributable<RE::TESSoulGem>          soulGems{ ITEM::kSoulGem };
	inline Distributable<RE::SpellItem>           spells{ ITEM::kSpell };
	inline Distributable<RE::TESObjectACTI>       activators{ ITEM::kActivator };
	inline Distributable<RE::TESFlora>            flora{ ITEM::kFlora };
	inline Distributable<RE::TESFurniture>        furniture{ ITEM::kFurniture };
	inline Distributable<RE::TESRace>             races{ ITEM::kRace };
	inline Distributable<RE::BGSTalkingActivator> talkingActivators{ ITEM::kTalkingActivator };
	inline Distributable<RE::EnchantmentItem>     enchantments{ ITEM::kEnchantmentItem };

	template <typename Func, typename... Args>
	void ForEachDistributable(Func&& a_func, Args&&... args)
	{
		const auto process = [&](auto&& container) {
			a_func(container, std::forward<Args>(args)...);
		};

		process(armors);
		process(weapons);
		process(ammo);
		process(magicEffects);
		process(potions);
		process(scrolls);
		process(locations);
		process(ingredients);
		process(books);
		process(miscItems);
		process(keys);
		process(soulGems);
		process(spells);
		process(activators);
		process(flora);
		process(furniture);
		process(races);
		process(talkingActivators);
		process(enchantments);
	}

	void Clear();
}
using KeywordData = Keyword::Data;
using KeywordDataVec = Keyword::DataVec;

template <class T>
Keyword::Distributable<T>::Distributable(ITEM::TYPE a_type) :
	type(a_type)
{}

template <class T>
Keyword::Distributable<T>::operator bool() const
{
	return !empty();
}

template <class T>
bool Keyword::Distributable<T>::empty() const
{
	return keywords.empty();
}

template <class T>
std::size_t Keyword::Distributable<T>::size() const
{
	return keywords.size();
}

template <class T>
void Keyword::Distributable<T>::clear()
{
	keywords.clear();
}

template <class T>
ITEM::TYPE Keyword::Distributable<T>::GetType() const
{
	return type;
}

template <class T>
std::string_view Keyword::Distributable<T>::GetTypeString() const
{
	return ITEM::GetType(type);
}

template <class T>
const KeywordDataVec& Keyword::Distributable<T>::GetKeywords() const
{
	return keywords;
}

template <class T>
KeywordDataVec& Keyword::Distributable<T>::GetKeywords()
{
	return keywords;
}

template <class T>
void Keyword::Distributable<T>::LookupForms()
{
	auto& INIDataVec = INI::INIs[type];
	if (INIDataVec.empty()) {
		return;
	}

	logger::info("{}", GetTypeString());

	keywords.reserve(INIDataVec.size());

	const auto dataHandler = RE::TESDataHandler::GetSingleton();
	auto&      keywordArray = dataHandler->GetFormArray<RE::BGSKeyword>();

	// INIDataVec index, keyword
	std::map<std::uint32_t, RE::BGSKeyword*> processedKeywords{};

	// Process keywords to be distributed first.
	std::uint32_t index = 0;

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
						   } else if (keyword->formEditorID.empty()) {
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

		if (keyword) {
			processedKeywords.emplace(index, keyword);
		}
		index++;
	}

	// Get Filters
	for (auto& [vecIdx, keyword] : processedKeywords) {
		auto& [rawForm, rawFilters, traits, chance, path] = INIDataVec[vecIdx];

		buffered_logger::info("\t[{}] {}", path, keyword->GetFormEditorID());

		ProcessedFilters processedFilters{};

		bool validEntry = detail::formID_to_form(rawFilters.ALL, processedFilters.ALL);
		if (validEntry) {
			validEntry = detail::formID_to_form(rawFilters.NOT, processedFilters.NOT);
		}
		if (validEntry) {
			validEntry = detail::formID_to_form(rawFilters.MATCH, processedFilters.MATCH);
		}
		if (validEntry) {
			std::ranges::for_each(rawFilters.ANY, [](std::string& str) {
				if (string::icontains(str, ".nif")) {
					Filter::SanitizePath(str);
				}
			});
			processedFilters.ANY = rawFilters.ANY;
		}

		if (!validEntry) {
			logger::error("\t\tNo filters were processed, skipping distribution");
			continue;
		}

		keywords.emplace_back(Data{ 0, keyword, { processedFilters, traits, chance } });
	}
}
