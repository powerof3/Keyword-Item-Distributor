#include "Cache.h"

namespace Cache
{
	std::string EditorID::GetEditorID(const RE::TESForm* a_form)
	{
		auto tweaks = GetModuleHandle("po3_Tweaks");
	    auto function = reinterpret_cast<_GetFormEditorID>(GetProcAddress(tweaks, "GetFormEditorID"));
		if (function) {
			return function(a_form->GetFormID());
		}
		return std::string();
	}

	bool FormType::IsFilter(RE::FormType a_type)
	{
		return set.find(a_type) != set.end();
	}

	ITEM::TYPE Item::GetType(const std::string& a_type)
	{
		auto it = map.find(a_type);
		return it != map.end() ? it->second : ITEM::TYPE::kNone;
	}

	std::string Item::GetType(ITEM::TYPE a_type)
	{
		auto it = reverse_map.find(a_type);
		return it != reverse_map.end() ? std::string(it->second) : std::string();
	}

	bool Archetype::Matches(Archetype a_archetype, const StringVec& a_strings)
	{
		if (const auto it = archetypeMap.find(a_archetype); it != archetypeMap.end()) {
			auto archetypeStr = it->second;
			return std::ranges::any_of(a_strings, [&](const auto& str) {
				return string::iequals(archetypeStr, str);
			});
		}
		return false;
	}
}
