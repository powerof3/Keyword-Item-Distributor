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

/// A standardized way of converting any object to string.
///
///	<p>
///	Overload `operator<<` to provide custom formatting for your value.
///	Alternatively, specialize this method and provide your own implementation.
///	</p>
template <typename Value>
std::string describe(Value value)
{
	std::ostringstream os;
	os << value;
	return os.str();
}

inline std::ostream& operator<<(std::ostream& os, RE::TESFile* file)
{
	os << file->fileName;
	return os;
}

inline std::ostream& operator<<(std::ostream& os, RE::TESForm* form)
{
	if (const auto& edid = Cache::EditorID::GetEditorID(form); !edid.empty()) {
		os << edid << " ";
	}
	os << "["
	   << std::to_string(form->GetFormType())
	   << ":"
	   << std::setfill('0')
	   << std::setw(sizeof(RE::FormID) * 2)
	   << std::uppercase
	   << std::hex
	   << form->GetFormID()
	   << "]";

	return os;
}
