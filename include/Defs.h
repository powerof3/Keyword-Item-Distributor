#pragma once

// Keyword = formID~esp(OR)keywordEditorID|type|strings,formIDs(OR)editorIDs|traits|chance

using FormModPair = std::pair<
	std::optional<RE::FormID>,    // formID
	std::optional<std::string>>;  // modName

using FormOrEditorID = std::variant<
	FormModPair,   // formID~modName
	std::string>;  // editorID

template <class T>
struct Filters
{
	std::vector<T> ALL{};
	std::vector<T> NOT{};
	std::vector<T> MATCH{};
};

using StringVec = std::vector<std::string>;
struct StringFilters : Filters<std::string>
{
	StringVec ANY{};
};

using RawFormVec = std::vector<FormOrEditorID>;
using RawFormFilters = Filters<FormOrEditorID>;

using FormOrMod = std::variant<RE::TESForm*,  // form
	const RE::TESFile*>;                      // mod
using FormVec = std::vector<FormOrMod>;
using FormFilters = Filters<FormOrMod>;

using Chance = float;
using Path = std::string;
using Count = std::uint32_t;
