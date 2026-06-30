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

	bool entryValid = true;

	const auto add_dependency = [&](const FilterRule<ResolvedFilter>& a_rule) {
		if (auto resolved = std::get_if<ResolvedFilter>(&a_rule.value)) {
			if (auto form = resolved->GetDependentForm()) {
				dependencies.push_back(form);
			}
		}
	};

	std::size_t rawMatch = 0;
	std::size_t okMatch = 0;

	std::size_t rawNot = 0;
	std::size_t okNot = 0;

	ANY.reserve(a_rawFilters.ANY.size());
	for (const auto& fromFilter : a_rawFilters.ANY) {
		const bool isPartial = std::holds_alternative<PartialString>(fromFilter.value);
		const bool isExclude = fromFilter.excludeModifier;
		if (isExclude) {
			++rawNot;
		} else if (!isPartial) {
			++rawMatch;
		}
		auto toFilter = FilterRule<ResolvedFilter>(fromFilter, false, 0);
		if (toFilter.Valid()) {
			if (isExclude) {
				++okNot;
			} else if (!isPartial) {
				++okMatch;
			}
			add_dependency(toFilter);
			ANY.emplace_back(std::move(toFilter));
		}
	}
	std::ranges::stable_sort(ANY, {}, [&](const auto& f) { return f.GetFilterCost(false); });

	if ((rawMatch > 0 && okMatch == 0) || (rawNot > 0 && okNot == 0)) {
		entryValid = false;
	}

	const std::size_t rawAll = a_rawFilters.ALL.size();
	std::size_t       okAll = 0;

	ALL.reserve(a_rawFilters.ALL.size());
	for (std::size_t i = 0; i < a_rawFilters.ALL.size(); ++i) {
		const auto&                 fromGroup = a_rawFilters.ALL[i];
		FilterGroup<ResolvedFilter> toGroup{};
		toGroup.reserve(fromGroup.size());
		for (const auto& fromFilter : fromGroup) {
			auto toFilter = FilterRule<ResolvedFilter>(fromFilter, true, i);
			if (toFilter.Valid()) {
				add_dependency(toFilter);
				toGroup.emplace_back(std::move(toFilter));
			}
		}
		std::ranges::stable_sort(toGroup, {}, [&](const auto& f) { return f.GetFilterCost(true); });

		if (toGroup.size() == fromGroup.size()) {
			++okAll;
			ALL.emplace_back(std::move(toGroup));
		}
	}

	if (rawAll > 0 && okAll == 0) {
		entryValid = false;
	}

	isValid = entryValid && (!ALL.empty() || !ANY.empty());
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
										  [&](const ExactString& a_str) {
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
		bool hasExact = false;
		bool exactPassed = false;

		bool hasPartial = false;
		bool partialPassed = false;

		for (const auto& f : a_group) {
			if (f.excludeModifier) {
				if (matches(f)) {
					return false;
				}
				continue;
			}

			if (std::holds_alternative<PartialString>(f.value)) {
				hasPartial = true;
				if (!partialPassed && matches(f)) {
					partialPassed = true;
				}
			} else {
				hasExact = true;
				if (!exactPassed && matches(f)) {
					exactPassed = true;
				}
			}
		}

		return (!hasExact || exactPassed) && (!hasPartial || partialPassed);
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
