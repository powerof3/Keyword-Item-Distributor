#pragma once

#include "Data\FormData.h"

struct ItemData;
class Traits;

namespace filter
{
	inline std::vector<std::string> skipLog;
}

enum class StringType : std::uint8_t
{
	kPlain = 0,  // editorID/name
	kAV,
	kArchetype,
	kModel
};

using PartialString = std::string;

template <class T>
using FilterValue = std::variant<T, PartialString>;

template <class T>
struct RuleData
{};

template <>
struct RuleData<ResolvedFilter>
{
	void SetStringData(const std::string& a_str)
	{
		if (a_str.ends_with(".nif")) {
			stringType = StringType::kModel;
			return;
		}
		if (const auto it = AV::map.find(a_str); it != AV::map.end()) {
			stringType = StringType::kAV;
			parsedEnum = static_cast<std::uint32_t>(it->second);
			return;
		}
		if (const auto it = ARCHETYPE::map.find(a_str); it != ARCHETYPE::map.end()) {
			stringType = StringType::kArchetype;
			parsedEnum = static_cast<std::uint32_t>(it->second);
			return;
		}
		stringType = StringType::kPlain;
	}

	// members
	REX::Enum<StringType, std::uint8_t> stringType{ StringType::kPlain };
	std::uint32_t                       parsedEnum{ static_cast<std::uint32_t>(-1) };
};

template <class T>
struct FilterRule : RuleData<T>
{
	FilterRule() = default;
	FilterRule(bool a_excludeModifier, bool a_partialModifier, const std::string& a_value) :
		isValid(true),
		excludeModifier(a_excludeModifier)
	{
		if (a_partialModifier) {
			value = a_value;
		} else {
			value = T(a_value);
		}
	}

	FilterRule(const FilterRule<RawForm>& a_from, bool a_allFilter, std::size_t a_idx)
		requires(std::is_same_v<T, ResolvedFilter>)
		:
		excludeModifier(a_from.excludeModifier)
	{
		std::visit(overload{
					   [&](const RawForm& rawFilter) {
						   auto resolvedFilter = ResolvedFilter(rawFilter);
						   if (!resolvedFilter.Valid()) {
							   isValid = false;
							   const auto tag = a_allFilter ? std::format("ALL #{}", a_idx) : "ANY";
							   if (resolvedFilter.formType) {
								   filter::skipLog.emplace_back(fmt::format("[{}][{}] SKIP - (invalid form type {})", tag, rawFilter, *resolvedFilter.formType));
							   } else {
								   filter::skipLog.emplace_back(fmt::format("[{}][{}] SKIP - ({} not found)", tag, rawFilter, rawFilter.IsMod() ? "mod" : "form"));
							   }
							   return;
						   }
						   if (const auto* str = std::get_if<ExactString>(&resolvedFilter.filter)) {
							   RuleData<ResolvedFilter>::SetStringData(*str);
							   resolvedFilter.filter = string::tolower(*str); 
						   }
						   value = resolvedFilter;
					   },
					   [&](const PartialString& a_str) {
						   RuleData<ResolvedFilter>::SetStringData(a_str);
						   value = string::tolower(a_str);
					   } },
			a_from.value);
	}

	bool Valid() { return isValid; }

	// sort filter according to how expensive it is
	// TESForm* / enum checks first
	std::int32_t GetFilterCost(bool a_allFilter) const
		requires(std::is_same_v<T, ResolvedFilter>)
	{
		std::int32_t cost = 0;

		const auto calc_string_cost = [&](bool a_partialModifier) {
			switch (this->stringType.get()) {
			case StringType::kAV:
			case StringType::kArchetype:
				cost += 10;
				break;
			case StringType::kModel:
				cost += (a_partialModifier ? 60 : 50);
				break;
			default:
				cost += (a_partialModifier ? 40 : 30);
				break;
			}
		};

		std::visit(overload{
					   [&](const ResolvedFilter& resolvedFilter) {
						   std::visit(overload{
										  [&](RE::TESForm*) { cost += 10; },
										  [&](const RE::TESFile*) { cost += 20; },
										  [&](const ExactString&) { calc_string_cost(false); } },
							   resolvedFilter.filter);
					   },
					   [&](const PartialString&) {
						   calc_string_cost(true);
					   } },
			value);

		if (a_allFilter ? !excludeModifier : excludeModifier) {
			cost -= 1;
		}

		return cost;
	}

	std::string to_string() const
	{
		std::string token;
		if (excludeModifier) {
			token += '-';
		}
		std::visit(overload{
					   [&](const T& a_value) { token += fmt::format("{}", a_value); },
					   [&](const PartialString& a_str) {
						   token += '*';
						   token += a_str;
					   } },
			value);
		return token;
	}

	// members
	bool           isValid{ true };
	bool           excludeModifier{ false };
	FilterValue<T> value{};
};

template <class T>
using FilterGroup = std::vector<FilterRule<T>>;  // [Guard+*Mage+-Thief+-*Bandit]

template <class T>
struct FilterSet
{
	FilterSet() = default;

	bool empty() const { return ALL.empty() && ANY.empty(); }

	std::string to_string() const
	{
		std::string result;
		const auto  append = [&](const std::string& a_token) {
			if (!a_token.empty()) {
				if (!result.empty()) {
					result += ',';
				}
				result += a_token;
			}
		};
		for (const auto& group : ALL) {
			std::string groupStr;
			for (const auto& filter : group) {
				if (!groupStr.empty()) {
					groupStr += '+';
				}
				groupStr += filter.to_string();
			}
			append(groupStr);
		}
		for (const auto& filter : ANY) {
			append(filter.to_string());
		}
		return result;
	}

	// members
	std::vector<FilterGroup<T>> ALL{};  // [Guard+*Mage+-Thief+-*Bandit],[Horse+-Deer+*Rabbit],[...]
	FilterGroup<T>              ANY{};  // Guard,*Mage,-Thief,-*Bandit,...
};

struct ConfigFilterSet : FilterSet<RawForm>
{
	ConfigFilterSet() = default;
	ConfigFilterSet(const std::string& a_str);
};

struct DistributableFilterSet : FilterSet<ResolvedFilter>
{
	DistributableFilterSet() = default;
	DistributableFilterSet(const ConfigFilterSet& a_rawFilters);

	bool valid() const { return isValid; }

	bool Pass(ItemData& a_refData) const;

	// members
	bool                      isValid{ false };  //
	std::vector<RE::TESForm*> dependencies{};    // used in sorting
};

struct Chance
{
	bool Pass(RE::BGSKeyword* a_keyword, const ItemData& a_data) const;

	// members
	float value{};
};

// collection of form filters+level+traits that needs to be passed before distribution
template <class FilterSet>
struct Criteria
{
	bool                       Validated() const { return filters.valid(); }
	std::vector<RE::TESForm*>& GetDependencies() { return filters.dependencies; }

	// members
	FilterSet               filters;
	std::shared_ptr<Traits> traits{ nullptr };
	Chance                  chance{ 100.0 };
};

using ConfigCriteria = Criteria<ConfigFilterSet>;

struct DistributableCriteria : public Criteria<DistributableFilterSet>
{
	DistributableCriteria() = default;
	DistributableCriteria(const ConfigCriteria& a_configCriteria);

	bool PassFilters(RE::BGSKeyword* a_keyword, ItemData& a_data) const;
};
