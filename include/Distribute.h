#pragma once

#include "ExclusiveGroups.h"
#include "KeywordData.h"

namespace Distribute
{
	using namespace Keyword;

	template <class T, class OnKeywordAdded>
	void distribute_impl(
		T*               a_item,
		KeywordDataVec&  a_keywords,
		bool             a_hasExclusions,
		OnKeywordAdded&& a_onKeywordAdded)
	{
		ItemData itemData(a_item);

		std::vector<RE::BGSKeyword*> processedKeywords;
		processedKeywords.reserve(a_keywords.size());

		for (auto& [keyword, criteriaList] : a_keywords) {
			// Skip if the item already has this keyword.
			if (itemData.HasKeyword(keyword)) {
				continue;
			}

			// Skip if keyword from related exclusive groups is already present.
			if (a_hasExclusions && itemData.HasMutuallyExclusiveKeyword(keyword)) {
				continue;
			}

			for (const auto& criteria : criteriaList) {
				if (criteria.PassFilters(keyword, itemData)) {
					a_onKeywordAdded(keyword, a_item);

					processedKeywords.emplace_back(keyword);
					itemData.AddKeyword(keyword);

					break;
				}
			}
		}

		if (!processedKeywords.empty()) {
			a_item->AddKeywords(processedKeywords);
		}
	}


	template <class T>
	void distribute(T* a_item, KeywordDataVec& a_keywords, bool a_hasExclusions)
	{
		distribute_impl(
			a_item,
			a_keywords,
			a_hasExclusions,
			[](RE::BGSKeyword*, T*) {});
	}

	template <class T>
	void distribute(
		T*                  a_item,
		KeywordDataVec&     a_keywords,
		DistributedFormMap& a_distributedFormMap,
		bool                a_hasExclusions)
	{
		distribute_impl(
			a_item,
			a_keywords,
			a_hasExclusions,
			[&](RE::BGSKeyword* a_keyword, T* a_item) {
				a_distributedFormMap.emplace_or_visit(
					a_keyword,
					std::vector<RE::TESForm*>{ a_item },
					[&](auto& a_entry) {
						a_entry.second.push_back(a_item);
					});
			});
	}

	template <class T>
	void distribute(Distributable<T>& a_keywords, bool a_hasExclusions)
	{
		if (!a_keywords) {
			return;
		}

		auto& keywords = a_keywords.GetKeywords();
		auto& forms = RE::TESDataHandler::GetSingleton()->GetFormArray<T>();
		auto& distributedForms = a_keywords.GetDistributedForms();

		std::for_each(std::execution::par, forms.begin(), forms.end(),
			[&](T* item) {
				if (item) {
					distribute(item, keywords, distributedForms, a_hasExclusions);
				}
			});
	}

	template <class T>
	void log_keyword_count(Distributable<T>& a_keywords)
	{
		if (a_keywords) {
			logger::info("{}", a_keywords.GetTypeString());

			const auto formArraySize = RE::TESDataHandler::GetSingleton()->GetFormArray<T>().size();

			auto& distributedForms = a_keywords.GetDistributedForms();

			for (const auto& keywordData : a_keywords.GetKeywords()) {
				const auto keyword = keywordData.keyword;

				bool distributed = distributedForms.cvisit(keyword, [&](const auto& entry) {
					const auto count = entry.second.size();
					if (const auto file = keyword->GetFile(0)) {
						logger::info("\t{} [0x{:X}~{}] added to {}/{}", keyword->GetFormEditorID(), keyword->GetLocalFormID(), file->GetFilename(), count, formArraySize);
					} else {
						logger::info("\t{} [0x{:X}] added to {}/{}", keyword->GetFormEditorID(), keyword->GetFormID(), count, formArraySize);
					}
				});

				if (!distributed) {
					if (const auto file = keyword->GetFile(0)) {
						logger::info("\t{} [0x{:X}~{}] added to 0/{}", keyword->GetFormEditorID(), keyword->GetLocalFormID(), file->GetFilename(), formArraySize);
					} else {
						logger::info("\t{} [0x{:X}] added to 0/{}", keyword->GetFormEditorID(), keyword->GetFormID(), formArraySize);
					}
				}
			}

			distributedForms.clear();
		}
	}

	void AddKeywords();
}
