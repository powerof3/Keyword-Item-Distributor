#pragma once

#include "DependencyResolver.h"
#include "KeywordData.h"

namespace Keyword
{
	inline std::once_flag             init;
	inline StringMap<RE::BGSKeyword*> allKeywords{};

	namespace Dependencies
	{
		using Keyword = RE::BGSKeyword*;

		/// Comparator that preserves relative order at which Keywords appeared in config files.
		/// If that order is undefined it falls back to alphabetical order of EditorIDs.
		struct keyword_less
		{
			using RelativeOrderMap = Map<Keyword, std::int32_t>;

			const RelativeOrderMap relativeOrder;

			bool operator()(const Keyword& a, const Keyword& b) const
			{
				const auto aIdx = getIndex(a);
				const auto bIdx = getIndex(b);
				if (aIdx >= 0 && bIdx >= 0) {
					if (aIdx < bIdx) {
						return true;
					}
					if (aIdx > bIdx) {
						return false;
					}
				}
				return a->GetFormEditorID() < b->GetFormEditorID();
			}

			[[nodiscard]] std::int32_t getIndex(const Keyword& kwd) const
			{
				if (auto it = relativeOrder.find(kwd); it != relativeOrder.end()) {
					return it->second;
				}
				return -1;
			}
		};

		using Resolver = DependencyResolver<Keyword, keyword_less>;

		/// Reads Forms::keywords and sorts them based on their relationship or alphabetical order.
		/// This must be called after initial Lookup was performed.

		template <class T>
		void ResolveKeywords(Distributable<T>& keywords)
		{
			if (!keywords) {
				return;
			}

			auto& keywordForms = keywords.GetKeywords();

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

			keyword_less::RelativeOrderMap orderMap;

			for (std::int32_t index = 0; index < keywordForms.size(); ++index) {
				orderMap.emplace(keywordForms[index].keyword, index);
			}

			Resolver resolver{ keyword_less(orderMap) };

			/// A map that will be used to map back keywords to their data wrappers.
			std::unordered_multimap<RE::BGSKeyword*, KeywordData> dataKeywords;

			logger::info("\tSorting keywords...");

			for (const auto& keywordData : keywordForms) {
				dataKeywords.emplace(keywordData.keyword, keywordData);
				resolver.AddIsolated(keywordData.keyword);

				const auto findKeyword = [&](const std::string& name) -> RE::BGSKeyword* {
					auto it = allKeywords.find(name);
					return it != allKeywords.end() ? it->second : nullptr;
				};

				const auto containsKeyword = [&](const std::string& name) -> RE::BGSKeyword* {
					for (const auto& [keywordName, keyword] : allKeywords) {
						if (keywordName.contains(name)) {
							return keyword;
						}
					}
					return nullptr;
				};

				const auto addFilterGroupDependencies = [&](const FilterGroup<ResolvedFilter>& a_group) {
					for (const auto& rule : a_group) {
						std::visit(overload{
									   [&](const ResolvedFilter& a_resolvedFilter) {
										   if (const auto form = a_resolvedFilter.GetForm()) {
											   if (const auto kwd = form->As<RE::BGSKeyword>()) {
												   resolver.AddDependency(keywordData.keyword, kwd);
											   }
										   } else if (auto stringPtr = std::get_if<FullString>(&a_resolvedFilter.filter)) {
											   if (const auto kywd = findKeyword(*stringPtr)) {
												   resolver.AddDependency(keywordData.keyword, kywd);
											   }
										   }
									   },
									   [&](const PartialString& a_str) {
										   if (const auto kywd = containsKeyword(a_str)) {
											   resolver.AddDependency(keywordData.keyword, kywd);
										   }
									   } },
							rule.value);
					}
				};

				// Collect all dependencies from merged criteria
				for (const auto& criteria : keywordData.filters) {
					const auto& filters = criteria.filters;
					for (const auto& group : filters.ALL) {
						addFilterGroupDependencies(group);
					}
					addFilterGroupDependencies(filters.ANY);
				}
			}

			const auto result = resolver.Resolve();

			keywordForms.clear();

			logger::info("\tSorted keywords: ");
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
