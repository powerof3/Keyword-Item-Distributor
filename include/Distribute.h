#pragma once

#include "KeywordData.h"
#include "LookupFilters.h"

namespace Distribute
{
	using namespace Keyword;

	template <class T>
	void distribute(Distributables& keywords)
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
	void log_keyword_count(const ITEM::TYPE a_type, Distributables& keywords)
	{
		if (keywords) {
			logger::info("{}", GetType(a_type));

			const auto formArraySize = RE::TESDataHandler::GetSingleton()->GetFormArray<T>().size();

			for (const auto& [keyword, count] : keywords.GetKeywordCounts()) {
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
