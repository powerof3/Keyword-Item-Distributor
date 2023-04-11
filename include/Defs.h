#pragma once

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
using Path = std::string;
using Count = std::uint32_t;
