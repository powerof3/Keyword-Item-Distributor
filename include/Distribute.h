#pragma once

#include "KeywordData.h"

namespace Distribute
{
	using namespace Keyword;

	template <class T>
	void distribute(T* a_item, KeywordDataVec& a_keywords)
	{
		Item::Data itemData(a_item);

		std::vector<RE::BGSKeyword*> processedKeywords;
		processedKeywords.reserve(a_keywords.size());
		for (auto& [count, keyword, filters] : a_keywords) {
			if (itemData.PassedFilters(keyword, filters)) {
				processedKeywords.emplace_back(keyword);
				++count;
			}
		}

		if (!processedKeywords.empty()) {
			a_item->AddKeywords(processedKeywords);
		}
	}

	template <class T>
	void distribute(Distributable<T>& a_keywords)
	{
		if (a_keywords) {
			auto& keywords = a_keywords.GetKeywords();
		    for (auto& item : RE::TESDataHandler::GetSingleton()->GetFormArray<T>()) {
				if (item) {
					distribute(item, keywords);
				}
			}
		}
	}

	template <class T>
	void log_keyword_count(const Distributable<T>& a_keywords)
	{
		if (a_keywords) {
			logger::info("{}", a_keywords.GetTypeString());

			const auto formArraySize = RE::TESDataHandler::GetSingleton()->GetFormArray<T>().size();

			// Group the same entries together to show total number of distributed records in the log.t
		    tsl::ordered_map<RE::FormID, Data> sums{};
			for (auto& keywordData : a_keywords.GetKeywords()) {
				auto it = sums.find(keywordData.keyword->GetFormID());
				if (it != sums.end()) {
					it.value().count += keywordData.count;
				} else {
					sums.insert({ keywordData.keyword->GetFormID(), keywordData });
				}
			}

			for (auto& entry : sums | std::views::values) {
				auto& [count, keyword, filters] = entry;
				if (const auto file = keyword->GetFile(0)) {
					logger::info("\t{} [0x{:X}~{}] added to {}/{}", keyword->GetFormEditorID(), keyword->GetLocalFormID(), file->GetFilename(), count, formArraySize);
				} else {
					logger::info("\t{} [0x{:X}] added to {}/{}", keyword->GetFormEditorID(), keyword->GetFormID(), count, formArraySize);
				}
			}
			sums.clear();
		}
	}

	void AddKeywords();
}
