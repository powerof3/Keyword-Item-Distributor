#pragma once

#include "DependencyResolver.h"
#include "KeywordData.h"

namespace Keyword
{
	inline std::once_flag init;

    inline StringMap<RE::BGSKeyword*> allKeywords{};

    namespace Dependencies
	{
		using KYWD = RE::BGSKeyword*;

		struct keyword_less
		{
			bool operator()(const KYWD& a, const KYWD& b) const
			{
				return a->GetFormEditorID() < b->GetFormEditorID();
			}
		};

		using Resolver = DependencyResolver<KYWD, keyword_less>;

		/// Reads Forms::keywords and sorts them based on their relationship or alphabetical order.
		/// This must be called after initial Lookup was performed.

        template <class T>
		void ResolveKeywords(Distributable<T> & keywords)
		{
			if (!keywords) {
				return;
			}

            std::call_once(init, []() {
				const auto dataHandler = RE::TESDataHandler::GetSingleton();
				for (const auto& kwd : dataHandler->GetFormArray<RE::BGSKeyword>()) {
					if (kwd) {
						if (const auto edid = kwd->GetFormEditorID(); !string::is_empty(edid)) {
							allKeywords[edid] = kwd;
						} else {
							if (const auto file = kwd->GetFile(0)) {
								const auto  modname = file->GetFilename();
								const auto  formID = kwd->GetLocalFormID();
								std::string mergeDetails;
								if (g_mergeMapperInterface && g_mergeMapperInterface->isMerge(modname.data())) {
									const auto [mergedModName, mergedFormID] = g_mergeMapperInterface->GetOriginalFormID(
										modname.data(),
										formID);
									mergeDetails = std::format("->0x{:X}~{}", mergedFormID, mergedModName);
								}
								logger::error("\tWARN : [0x{:X}~{}{}] keyword has an empty editorID!", formID, modname, mergeDetails);
							}
						}
					}
				}
			});

			Resolver resolver;

			auto& keywordForms = keywords.GetKeywords();

			/// A map that will be used to map back keywords to their data wrappers.
			std::unordered_multimap<RE::BGSKeyword*, KeywordData> dataKeywords;

			logger::info("\tSorting keywords...");
            for (const auto& keywordData : keywordForms) {
				dataKeywords.emplace(keywordData.keyword, keywordData);
				resolver.AddIsolated(keywordData.keyword);

				const auto addDependencies = [&](const ProcessedVec& a_processed) {
					for (const auto& filter : a_processed) {
						if (const auto formPtr = std::get_if<RE::TESForm*>(&filter)) {
							if (const auto& kwd = (*formPtr)->As<RE::BGSKeyword>()) {
								resolver.AddDependency(keywordData.keyword, kwd);
							}
						}
					}
				};

				const auto addANYDependencies = [&](const std::vector<std::string>& a_strings) {
					for (const auto& filter : a_strings) {
						for (const auto& [keywordName, kwd] : allKeywords) {
							if (string::icontains(keywordName, filter)) {
								resolver.AddDependency(keywordData.keyword, kwd);
							}
						}
					}
				};

				const auto& filters = keywordData.filters.processedFilters;

				addDependencies(filters.ALL);
				addDependencies(filters.NOT);
				addDependencies(filters.MATCH);

				addANYDependencies(filters.ANY);
			}

			const auto result = resolver.Resolve();

			keywordForms.clear();
			logger::info("\tSorted keywords :");
			for (const auto& keyword : result) {
				const auto& [begin, end] = dataKeywords.equal_range(keyword);
				if (begin != end) {
					logger::info("\t\t{}", describe(begin->second.keyword));
				}
				for (auto it = begin; it != end; ++it) {
					keywordForms.push_back(it->second);
				}
			}
		}
	}
}
