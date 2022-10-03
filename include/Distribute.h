#pragma once

#include "LookupFilters.h"

namespace Distribute
{
	template <class T>
	void add_keyword(T& a_item, KeywordDataVec& a_keywordDataVec)
	{
		for (auto& keywordData : a_keywordDataVec) {
			if (!Filter::strings(a_item, keywordData) || !Filter::forms(a_item, keywordData) || !Filter::secondary(a_item, keywordData)) {
				continue;
			}
			if (const auto keyword = std::get<DATA::kForm>(keywordData); keyword) {
				if (const auto keywordForm = a_item.template As<RE::BGSKeywordForm>(); keywordForm && keywordForm->AddKeyword(keyword)) {
					++std::get<DATA::kCount>(keywordData);
				}
			}
		}
	}

	template <class T>
	void distribute(const ITEM::TYPE a_type)
	{
		if (auto& keywords = Keywords[a_type]; !keywords.empty()) {
			const auto formArray = RE::TESDataHandler::GetSingleton()->GetFormArray<T>();
			for (const auto& item : formArray) {
				if (item) {
					add_keyword(*item, keywords);
				}
			}
			for (auto& formData : keywords) {
				auto keyword = std::get<DATA::kForm>(formData);
				auto count = std::get<DATA::kCount>(formData);

				if (keyword) {
					logger::info("{} [0x{:X}] added to {}/{} {}", keyword->GetFormEditorID(), keyword->GetFormID(), count, formArray.size(), ITEM::map.find(a_type)->second);
				}
			}
		}
	}

	void AddKeywords();
}
