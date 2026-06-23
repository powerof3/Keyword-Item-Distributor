#include "Data\BaseFilters.h"

#include "Data\ItemData.h"
#include "Data\Traits.h"
#include "RE.h"

ConfigFilterSet::ConfigFilterSet(const std::string& a_str)
{
	constexpr auto get_filter = [](std::string& entry) {
		auto  topLevelModifier = (entry[0] == '+' || entry[0] == '-') ? entry[0] : '+';  // -*Guard
		auto& filterEntry = (topLevelModifier == entry[0]) ? entry.erase(0, 1) : entry;  // *Guard
		bool  partialModifier = !filterEntry.empty() && filterEntry[0] == '*';

		if (partialModifier) {
			filterEntry.erase(0, 1);  // Guard
		}

		if (string::icontains(filterEntry, ".nif")) {
			RE::SanitizePath(filterEntry);
		}

		return FilterRule<RawForm>(topLevelModifier == '-', partialModifier, filterEntry);
	};

	for (auto& entry : string::split(a_str, ",")) {
		string::trim(entry);
		if (!distribution::is_valid_entry(entry)) {
			continue;
		}
		if (entry.contains('+')) {
			// A, (X + Y + Z), B
			FilterGroup<RawForm> filterGroup;
			for (auto& ALLEntry : string::split(entry, "+")) {
				string::trim(ALLEntry);
				if (ALLEntry.empty()) {
					continue;
				}
				filterGroup.emplace_back(get_filter(ALLEntry));
			}
			ALL.emplace_back(std::move(filterGroup));
		} else {
			// A or *B or -B or -*D
			ANY.emplace_back(get_filter(entry));
		}
	}
}

DistributableFilterSet::DistributableFilterSet(const ConfigFilterSet& a_rawFilters)
{
	if (a_rawFilters.empty()) {
		isValid = true;
		return;
	}

	const auto convert_group = [&](const FilterGroup<RawForm>& a_from, FilterGroup<ResolvedFilter>& a_to, bool a_allFilter, std::size_t a_idx = 0) {
		a_to.reserve(a_from.size());
		
		for (const auto& fromFilter : a_from) {
			auto toFilter = FilterRule<ResolvedFilter>(fromFilter, a_allFilter, a_idx);
			if (toFilter.Valid()) {
				if (auto resolvedFilter = std::get_if<ResolvedFilter>(&toFilter.value)) {
					if (auto form = resolvedFilter->GetDependentForm()) {
						dependencies.push_back(form);
					}
				}
				a_to.emplace_back(std::move(toFilter));
			}
		}

		std::ranges::stable_sort(a_to, {}, [&](const auto& a_filter) {
			return a_filter.GetFilterCost(a_allFilter);
		});

		return a_to.size() == a_from.size();
	};

	convert_group(a_rawFilters.ANY, ANY, false);

	ALL.reserve(a_rawFilters.ALL.size());
	for (std::size_t i = 0; i < a_rawFilters.ALL.size(); ++i) {
		FilterGroup<ResolvedFilter> toGroup{};
		if (convert_group(a_rawFilters.ALL[i], toGroup, true, i)) {
			ALL.emplace_back(std::move(toGroup));
		}
	}

	isValid = !ALL.empty() || !ANY.empty();
}

bool DistributableFilterSet::Pass(ItemData& a_refData) const
{
	if (ALL.empty() && ANY.empty()) {
		return true;
	}

	const auto matches = [&](const FilterRule<ResolvedFilter>& f) {
		bool result = false;
		std::visit(overload{
					   [&](const ResolvedFilter& a_resolvedFilter) {
						   std::visit(overload{
										  [&](RE::TESForm* a_form) {
											  result = a_form && a_refData.FormFilter(a_form);
										  },
										  [&](const RE::TESFile* a_file) {
											  result = a_file && a_file->IsFormInMod(a_refData.GetFormID());
										  },
										  [&](const FullString& a_str) {
											  result = a_refData.HasStringFilter(a_str, f.stringType.get(), f.parsedEnum);
										  } },
							   a_resolvedFilter.filter);
					   },
					   [&](const PartialString& a_str) {
						   result = a_refData.ContainsStringFilter(a_str, f.stringType.get());
					   } },
			f.value);
		return result;
	};

	const auto matches_any = [&](const std::vector<FilterRule<ResolvedFilter>>& a_group) {
		bool hasMatch = false;
		bool matchPassed = false;

		for (const auto& f : a_group) {
			if (f.excludeModifier) {
				if (matches(f)) {
					return false;
				}
			} else {
				hasMatch = true;
				if (!matchPassed) {
					if (matches(f)) {
						matchPassed = true;
					}
				}
			}
		}

		return !hasMatch || matchPassed;
	};

	const auto matches_all = [&](const FilterGroup<ResolvedFilter>& a_group) {
		for (const auto& f : a_group) {
			if (f.excludeModifier) {
				if (matches(f)) {
					return false;
				}
			} else if (!matches(f)) {
				return false;
			}
		}
		return true;
	};

	// ALL filters; at least one filter group must match (X+Y+Z or A+B+C)
	if (!ALL.empty()) {
		bool any_group_matched = false;
		for (const auto& group : ALL) {
			if (matches_all(group)) {
				any_group_matched = true;
				break;
			}
		}
		if (!any_group_matched) {
			return false;
		}
	}

	if (!ANY.empty() && !matches_any(ANY)) {
		return false;
	}

	return true;
}

bool Chance::Pass(RE::BGSKeyword* a_keyword, const ItemData& a_data) const
{
	if (value < 1.0f) [[unlikely]] {
		// create unique seed based on keyword editorID (can't use formID because it can be dynamic) and item formID
		// item formID alone would result in same RNG chance for different keywords
		const auto seed = hash::szudzik_pair(
			hash::fnv1a_32<std::string_view>(a_keyword->GetFormEditorID()),
			a_data.GetFormID());

		auto       RNG = clib_util::RNG(seed);
		const auto randNum = RNG.generate();
		if (randNum > value) {
			return false;
		}
	}
	return true;
}

DistributableCriteria::DistributableCriteria(const ConfigCriteria& a_configCriteria) :
	Criteria<DistributableFilterSet>{
		DistributableFilterSet(a_configCriteria.filters),
		a_configCriteria.traits,
		Chance{ a_configCriteria.chance.value / 100.0f }
	}
{}

bool DistributableCriteria::PassFilters(RE::BGSKeyword* a_keyword, ItemData& a_data) const
{
	if (!chance.Pass(a_keyword, a_data)) {
		return false;
	}

	if (traits && !traits->PassFilter(a_data.GetItem())) {
		return false;
	}

	if (!filters.Pass(a_data)) {
		return false;
	}

	return true;
}
