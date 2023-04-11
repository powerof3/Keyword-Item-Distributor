#pragma once

#include "Traits.h"

namespace Filter
{
	struct Data
	{
		Data(ProcessedFilters a_processedFilters, TraitsPtr a_traits, Chance a_chance);

		[[nodiscard]] bool PassedFilters(RE::TESForm* a_item);

		// members
		ProcessedFilters processedFilters{};
		TraitsPtr        traits{};
		Chance           chance{};

	private:
		RE::TESForm*        item{ nullptr };
		RE::BGSKeywordForm* kywdForm{ nullptr };
		std::string         edid{};
		std::string         name{};

		[[nodiscard]] bool HasFormOrStringFilter(const ProcessedVec& a_processed, bool a_all = false) const;
		[[nodiscard]] bool HasFormFilter(RE::TESForm* a_formFilter) const;
		[[nodiscard]] bool ContainsStringFilter(const std::vector<std::string>& a_strings) const;

		[[nodiscard]] bool has_skill_or_archetype(const std::string& a_str) const;
	};
}
using FilterData = Filter::Data;
