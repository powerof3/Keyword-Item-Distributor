#pragma once

template <class K, class D>
using Map = ankerl::unordered_dense::map<K, D>;
template <class K>
using Set = ankerl::unordered_dense::set<K>;

struct string_hash
{
	using is_transparent = void;  // enable heterogeneous overloads
	using is_avalanching = void;  // mark class as high quality avalanching hash

	[[nodiscard]] std::uint64_t operator()(std::string_view str) const noexcept
	{
		return ankerl::unordered_dense::hash<std::string_view>{}(str);
	}
};

template <class D>
using StringMap = ankerl::unordered_dense::map<std::string_view, D, string_hash, std::equal_to<>>;
using StringSet = ankerl::unordered_dense::set<std::string_view, string_hash, std::equal_to<>>;

// Keyword = formID~esp(OR)keywordEditorID|type|strings,formIDs(OR)editorIDs|traits|chance

// for visting variants
template <class... Ts>
struct overload : Ts...
{
	using Ts::operator()...;
};

using FormModPair = distribution::formid_pair;
// std::variant<FormID~ModName, std::string>
using FormIDOrString = distribution::record;

template <class T>
struct Filters
{
	std::vector<T>           ALL{};
	std::vector<T>           NOT{};
	std::vector<T>           MATCH{};
	std::vector<std::string> ANY{};
};

using RawVec = std::vector<FormIDOrString>;
using RawFilters = Filters<FormIDOrString>;

using FormOrString = std::variant<
	RE::TESForm*,        // form
	const RE::TESFile*,  // mod
	std::string>;        // string
using ProcessedVec = std::vector<FormOrString>;
using ProcessedFilters = Filters<FormOrString>;

using Chance = float;
