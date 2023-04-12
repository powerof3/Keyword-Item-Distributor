#pragma once

#include "KeywordData.h"

namespace Distribute
{
	using namespace Keyword;

	template <class T>
	void distribute(Distributables<T>& keywords)
	{
		if (keywords) {
			for (auto& item : RE::TESDataHandler::GetSingleton()->GetFormArray<T>()) {
				for (auto& [keyword, filters] : keywords.GetKeywords()) {
					if (filters.PassedFilters(item) && item->AddKeyword(keyword)) {
						keywords.IncrementCount(keyword);
					}
				}
			}
		}
	}

	template <class T>
	void log_keyword_count(const Distributables<T>& keywords)
	{
		if (keywords) {
			logger::info("{}", keywords.GetTypeString());

			const auto formArraySize = RE::TESDataHandler::GetSingleton()->GetFormArray<T>().size();

			for (const auto& [keyword, count] : keywords.GetKeywordCounts()) {
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
