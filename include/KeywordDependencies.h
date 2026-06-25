#pragma once

#include "DependencyResolver.h"
#include "KeywordData.h"

namespace Keyword
{
	inline IStringMap<RE::BGSKeyword*> allKeywords{};

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

		inline void InitGlobalKeywordMap()
		{
			const auto dataHandler = RE::TESDataHandler::GetSingleton();
			for (const auto& kywd : dataHandler->GetFormArray<RE::BGSKeyword>()) {
				if (kywd) {
					if (const auto edid = kywd->GetFormEditorID(); !string::is_empty(edid)) {
						allKeywords[string::tolower(edid)] = kywd;
					} else {
						if (const auto file = kywd->GetFile(0)) {
							const auto  modname = file->GetFilename();
							const auto  formID = kywd->GetLocalFormID();
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
		}

		/// Reads Forms::keywords and sorts them based on their relationship or alphabetical order.
		/// This must be called after initial Lookup was performed.

		template <class T>
		void ResolveKeywords(Distributable<T>& keywords)
		{
			if (!keywords) {
				return;
			}

			auto& keywordForms = keywords.GetKeywords();

			keyword_less::RelativeOrderMap orderMap;
			for (std::int32_t index = 0; index < keywordForms.size(); ++index) {
				orderMap.emplace(keywordForms[index].keyword, index);
			}

			Resolver resolver{ keyword_less(orderMap) };

			Map<RE::BGSKeyword*, KeywordData> dataKeywords;

			logger::info("\t\tSorting keywords...");

			//v3.0
			//addDependencies(filters.ALL);
			//addDependencies(filters.NOT);
			//addDependencies(filters.MATCH);
			//addANYDependencies(filters.ANY);

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

			const auto addDependencies = [&](RE::BGSKeyword* a_source, const FilterGroup<ResolvedFilter>& a_group, bool a_exactPass) {
				for (const auto& rule : a_group) {
					std::visit(overload{
								   [&](const ResolvedFilter& a_resolvedFilter) {
									   if (!a_exactPass) {
										   return;
									   }
									   if (const auto form = a_resolvedFilter.GetForm()) {
										   if (const auto kywd = form->As<RE::BGSKeyword>()) {
											   resolver.AddDependency(a_source, kywd);
										   }
									   } else if (auto stringPtr = std::get_if<ExactString>(&a_resolvedFilter.filter)) {
										   if (const auto kywd = findKeyword(*stringPtr)) {
											   resolver.AddDependency(a_source, kywd);
										   }
									   }
								   },
								   [&](const PartialString& a_str) {
									   if (a_exactPass) {
										   return;
									   }
									   if (const auto kywd = containsKeyword(a_str)) {
										   resolver.AddDependency(a_source, kywd);
									   }
								   } },
						rule.value);
				}
			};

			const auto collectAll = [&](const KeywordData& a_keywordData, bool a_exactPass) {
				for (const auto& criteria : a_keywordData.filters) {
					for (const auto& group : criteria.filters.ALL) {
						addDependencies(a_keywordData.keyword, group, a_exactPass);
					}
				}
			};

			const auto collectAny = [&](const KeywordData& a_keywordData, bool a_exactPass) {
				for (const auto& criteria : a_keywordData.filters) {
					addDependencies(a_keywordData.keyword, criteria.filters.ANY, a_exactPass);
				}
			};

			// ALL EXACT
			for (const auto& keywordData : keywordForms) {
				dataKeywords.emplace(keywordData.keyword, keywordData);
				resolver.AddIsolated(keywordData.keyword);
				collectAll(keywordData, true);
			}
			// ANY EXACT
			for (const auto& keywordData : keywordForms) {
				collectAny(keywordData, true);
			}
			// ALL WILDCARD
			for (const auto& keywordData : keywordForms) {
				collectAll(keywordData, false);
			}
			// ANY WILDCARD
			for (const auto& keywordData : keywordForms) {
				collectAny(keywordData, false);
			}

			const auto result = resolver.Resolve();

			keywordForms.clear();
			keywordForms.reserve(result.size());

			logger::info("\t\tSorted keywords: ");
			for (const auto& keyword : result) {
				if (auto it = dataKeywords.find(keyword); it != dataKeywords.end()) {
					logger::info("\t\t\t{}", describe(it->second.keyword));
					keywordForms.push_back(it->second);
				}
			}
		}
	}
}
