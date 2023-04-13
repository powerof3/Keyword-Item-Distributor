#pragma once

#include "Traits.h"

namespace Filter
{
    struct Data
	{
		Data() = default;
		Data(ProcessedFilters a_processedFilters, TraitsPtr a_traits, Chance a_chance);

		[[nodiscard]] bool PassedFilters(const RE::BGSKeyword* a_keyword, RE::TESForm* a_item);

		// members
		ProcessedFilters processedFilters{};
		TraitsPtr        traits{};
		Chance           chance{ 100 };

	private:
		RE::TESForm*        item{ nullptr };
		RE::BGSKeywordForm* kywdForm{ nullptr };
		std::string         edid{};
		std::string         name{};
		std::string         model{};

		[[nodiscard]] bool HasFormOrStringFilter(const ProcessedVec& a_processed, bool a_all = false) const;
		[[nodiscard]] bool HasFormFilter(RE::TESForm* a_formFilter) const;
		[[nodiscard]] bool HasStringFilter(const std::string& a_str) const;
		[[nodiscard]] bool ContainsStringFilter(const std::vector<std::string>& a_strings) const;
	};
}
using FilterData = Filter::Data;
