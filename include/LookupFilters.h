#pragma once

#include "Traits.h"

namespace Filter
{
	struct Data
	{
		Data(StringFilters a_stringFilters, FormFilters a_formFilters, TraitsPtr a_traits, Chance a_chance);

		[[nodiscard]] bool PassedFilters(RE::TESForm* a_item);

		[[nodiscard]] bool HasStringFilter(const StringVec& a_strings, bool a_all = false) const;
		[[nodiscard]] bool ContainsStringFilter(const StringVec& a_strings) const;
		[[nodiscard]] bool HasFormFilter(const FormVec& a_forms, bool all = false) const;

		// members
		StringFilters stringFilters{};
		FormFilters   formFilters{};
		TraitsPtr     traits{};
		Chance        chance{};

	private:
		RE::TESForm*        item{ nullptr };
		RE::BGSKeywordForm* kywdForm{ nullptr };
		RE::FormID          formID{ 0 };
		std::string         edid{};
		std::string         name{};

		[[nodiscard]] static bool has_actorvalue(RE::ActorValue a_av, const StringVec& a_strings);
		[[nodiscard]] bool        has_form_filter(RE::TESForm* a_formFilter) const;
	};
}
using FilterData = Filter::Data;
