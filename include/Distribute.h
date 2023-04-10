#pragma once

#include "KeywordData.h"
#include "LookupFilters.h"

namespace Distribute
{
	using namespace Keyword;

	template <class T>
	void add_keyword(T* a_item, KeywordDataVec& keywords)
	{
	    for (auto& [count, keyword, filters] : keywords) {
			if (filters.PassedFilters(a_item)) {
				a_item->AddKeyword(keyword);
			    ++count;
			}
		}
	}

	template <class T>
	void distribute(const ITEM::TYPE a_type, Distributables& keywords)
	{
		if (keywords) {
			const auto formArray = RE::TESDataHandler::GetSingleton()->GetFormArray<T>();
			auto&      keywordVec = keywords.GetKeywords();

			for (auto& item : formArray) {
				add_keyword(item, keywordVec);
			}

			const auto formArraySize = formArray.size();
			const auto type = ITEM::logTypes[a_type];

			for (const auto& [count, keyword, filters] : keywordVec) {
				if (const auto file = keyword->GetFile(0)) {
					logger::info("{} [0x{:X}~{}] added to {}/{} {}", keyword->GetFormEditorID(), keyword->GetLocalFormID(), file->GetFilename(), count, formArraySize, type);
				} else {
					logger::info("{} [0x{:X}] added to {}/{} {}", keyword->GetFormEditorID(), keyword->GetFormID(), count, formArraySize, type);
				}
			}
		}
	}

	void AddKeywords();
}
