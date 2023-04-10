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
			logger::info("{}", GetType(a_type));

			const auto formArray = RE::TESDataHandler::GetSingleton()->GetFormArray<T>();
			const auto formArraySize = formArray.size();

			auto& keywordVec = keywords.GetKeywords();

			for (auto& item : formArray) {
				add_keyword(item, keywordVec);
			}

			for (const auto& [count, keyword, filters] : keywordVec) {
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
