#pragma once

#include "Data\EnumDefs.h"
#include "Data\Ranges.h"

using FullString = std::string;

struct RawForm
{
	RawForm() = default;
	explicit RawForm(const std::string& a_str);

	[[nodiscard]] bool IsMod() const { return isMod; }
	std::string        to_string() const;
	RE::BGSKeyword*    to_keyword() const;

	RawForm GetMergedIDs() const;

	// members
	bool                       isMod{ false };
	std::optional<RE::FormID>  formID;
	std::optional<std::string> modNameOrEDID;

private:
	static bool IsMod(const std::string& a_str) { return a_str.ends_with(".esm") || a_str.ends_with(".esp") || a_str.ends_with(".esl"); }
};

struct ResolvedFilter
{
	ResolvedFilter() = default;
	explicit ResolvedFilter(const RawForm& a_id);

	bool Valid() const { return isValid; }

	RE::TESForm* GetForm() const;
	RE::TESForm* GetDependentForm() const;

	std::string to_string() const;

	// members
	std::variant<RE::TESForm*, const RE::TESFile*, FullString>  filter{};
	std::optional<RE::FormType>                                 formType{};
	bool                                                        isValid{ false };
};

// formatting
template <class CharT>
struct fmt::formatter<RawForm, CharT> : fmt::formatter<std::string_view, CharT>
{
	template <class FormatContext>
	auto format(const RawForm& a_formOrEditorID, FormatContext& a_ctx) const
	{
		return formatter<std::string_view>::format(a_formOrEditorID.to_string(), a_ctx);
	}
};

template <class CharT>
struct fmt::formatter<ResolvedFilter, CharT> : fmt::formatter<std::string_view, CharT>
{
	template <class FormatContext>
	auto format(const ResolvedFilter& a_resolvedFilter, FormatContext& a_ctx) const
	{
		return formatter<std::string_view>::format(a_resolvedFilter.to_string(), a_ctx);
	}
};

template <class CharT>
struct std::formatter<RawForm, CharT> : std::formatter<std::string_view, CharT>
{
	template <class FormatContext>
	auto format(const RawForm& a_formOrEditorID, FormatContext& a_ctx) const
	{
		return formatter<std::string_view>::format(a_formOrEditorID.to_string(), a_ctx);
	}
};

template <class CharT>
struct std::formatter<ResolvedFilter, CharT> : std::formatter<std::string_view, CharT>
{
	template <class FormatContext>
	auto format(const ResolvedFilter& a_resolvedFilter, FormatContext& a_ctx) const
	{
		return formatter<std::string_view>::format(a_resolvedFilter.to_string(), a_ctx);
	}
};
