#pragma once

#include "KeywordData.h"
#include "ExclusiveGroups.h"

namespace Distribute
{
	using namespace Keyword;

	template <class T>
	void distribute(T* a_item, KeywordDataVec& a_keywords, bool a_hasExclusions)
	{
		ItemData itemData(a_item);

		std::vector<RE::BGSKeyword*> processedKeywords;
		processedKeywords.reserve(a_keywords.size());

		for (auto& [count, keyword, criteriaList] : a_keywords) {
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
					processedKeywords.emplace_back(keyword);
					itemData.AddKeyword(keyword);
					count.fetch_add(1, std::memory_order_relaxed);
					break;
				}
			}
		}

		if (!processedKeywords.empty()) {
			a_item->AddKeywords(processedKeywords);
		}
	}

	template <class T>
	void distribute(Distributable<T>& a_keywords, bool a_hasExclusions)
	{
		if (!a_keywords) {
			return;
		}

		auto& keywords = a_keywords.GetKeywords();
		auto& forms = RE::TESDataHandler::GetSingleton()->GetFormArray<T>();

		std::for_each(std::execution::par, forms.begin(), forms.end(),
			[&](T* item) {
				if (item) {
					distribute(item, keywords, a_hasExclusions);
				}
			});
	}

	template <class T>
	void log_keyword_count(const Distributable<T>& a_keywords)
	{
		if (a_keywords) {
			logger::info("{}", a_keywords.GetTypeString());

			const auto formArraySize = RE::TESDataHandler::GetSingleton()->GetFormArray<T>().size();

			for (const auto& keywordData : a_keywords.GetKeywords()) {
				const auto keyword = keywordData.keyword;
				const auto count = keywordData.count.load(std::memory_order_relaxed);
				if (const auto file = keyword->GetFile(0)) {
					logger::info("\t{} [0x{:X}~{}] added to {}/{}", keyword->GetFormEditorID(), keyword->GetLocalFormID(), file->GetFilename(), count, formArraySize);
				} else {
					logger::info("\t{} [0x{:X}] added to {}/{}", keyword->GetFormEditorID(), keyword->GetFormID(), count, formArraySize);
				}
			}
		}
	}

	void AddKeywords();
}
