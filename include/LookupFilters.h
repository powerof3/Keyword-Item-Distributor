#pragma once

#include "Traits.h"

namespace Filter
{
	void SanitizeString(std::string& a_string);
	void SanitizePath(std::string& a_string);

	struct Data
	{
		Data() = default;
		Data(ProcessedFilters a_processedFilters, TraitsPtr a_traits, Chance a_chance);

		// members
		ProcessedFilters processedFilters{};
		TraitsPtr        traits{};
		Chance           chance{ 100 };
	};
}
using FilterData = Filter::Data;

namespace Item
{
	struct Data
	{
		Data(RE::TESForm* a_item);

		[[nodiscard]] bool PassedFilters(RE::BGSKeyword* a_keyword, const FilterData& a_filters);

	private:
		RE::TESForm* item{ nullptr };
		std::string  edid{};
		std::string  name{};
		std::string  model{};

		Set<RE::BGSKeyword*> keywords{};

		[[nodiscard]] bool HasFormOrStringFilter(const ProcessedVec& a_processed, bool a_all = false) const;
		[[nodiscard]] bool HasFormFilter(RE::TESForm* a_formFilter) const;
		[[nodiscard]] bool HasStringFilter(const std::string& a_str) const;
		[[nodiscard]] bool ContainsStringFilter(const std::vector<std::string>& a_strings) const;
	};
}
