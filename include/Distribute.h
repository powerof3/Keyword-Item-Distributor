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
	void log_keyword_count(const Distributable<T>& a_keywords)
	{
		if (a_keywords) {
			logger::info("{}", a_keywords.GetTypeString());

			const auto formArraySize = RE::TESDataHandler::GetSingleton()->GetFormArray<T>().size();

			// Group the same entries together to show total number of distributed records in the log.
			std::map<RE::FormID, std::uint32_t> sums{};
		    for (auto& [count, keyword, filters] : a_keywords.GetKeywords()) {
				auto it = sums.find(keyword->GetFormID());
				if (it != sums.end()) {
					it->second += count;
				} else {
					sums.insert({ keyword->GetFormID(), count });
				}
			}

			for (auto& [count, keyword, filters] : a_keywords.GetKeywords()) {
				auto actualCount = sums.find(keyword->GetFormID())->second;
			    if (const auto file = keyword->GetFile(0)) {
					buffered_logger::info("\t{} [0x{:X}~{}] added to {}/{}", keyword->GetFormEditorID(), keyword->GetLocalFormID(), file->GetFilename(), actualCount, formArraySize);
				} else {
					buffered_logger::info("\t{} [0x{:X}] added to {}/{}", keyword->GetFormEditorID(), keyword->GetFormID(), actualCount, formArraySize);
				}
			}
		}
	}

	void AddKeywords();
}
