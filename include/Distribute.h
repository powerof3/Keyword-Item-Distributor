#pragma once

#include "KeywordData.h"

namespace Distribute
{
	using namespace Keyword;

	template <class T>
	void distribute(Distributable<T>& keywords)
	{
		if (keywords) {
			for (auto& item : RE::TESDataHandler::GetSingleton()->GetFormArray<T>()) {
				for (auto& [count, keyword, filters] : keywords.GetKeywords()) {
					if (filters.PassedFilters(keyword, item) && item->AddKeyword(keyword)) {
						++count;
					}
				}
			}
		}
	}

	template <class T>
	void log_keyword_count(const Distributable<T>& keywords)
	{
		if (keywords) {
			logger::info("{}", keywords.GetTypeString());

			const auto formArraySize = RE::TESDataHandler::GetSingleton()->GetFormArray<T>().size();

			// Group the same entries together to show total number of distributed records in the log.
			std::map<RE::FormID, KeywordData> sums{};
			for (auto& keywordData : keywords.GetKeywords()) {
				if (const auto& keyword = keywordData.keyword) {
					auto it = sums.find(keyword->GetFormID());
					if (it != sums.end()) {
						it->second.count += keywordData.count;
					} else {
						sums.insert({ keyword->GetFormID(), keywordData });
					}
				}
			}

			for (auto& entry : sums | std::views::values) {
				auto& [count, keyword, filters] = entry;
				if (const auto file = keyword->GetFile(0)) {
					buffered_logger::info("\t{} [0x{:X}~{}] added to {}/{}", keyword->GetFormEditorID(), keyword->GetLocalFormID(), file->GetFilename(), count, formArraySize);
				} else {
					buffered_logger::info("\t{} [0x{:X}] added to {}/{}", keyword->GetFormEditorID(), keyword->GetFormID(), count, formArraySize);
				}
			}
		}
	}

	void AddKeywords();
}
