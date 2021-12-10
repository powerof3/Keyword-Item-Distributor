#include "Cache.h"

namespace Cache
{
	EditorID* EditorID::GetSingleton()
	{
		static EditorID singleton;
		return std::addressof(singleton);
	}

	void EditorID::FillMap()
	{
		const auto& [map, lock] = RE::TESForm::GetAllFormsByEditorID();
		const RE::BSReadLockGuard locker{ lock };
		if (map) {
			for (auto& [id, form] : *map) {
				_editorIDToFormIDMap.emplace(id.c_str(), form->GetFormID());
				_formIDToEditorIDMap.emplace(form->GetFormID(), id.c_str());
			}
		}
	}

	RE::FormID EditorID::GetFormID(const std::string& a_editorID)
	{
		Locker locker(_lock);
		auto it = _editorIDToFormIDMap.find(a_editorID);
		return it != _editorIDToFormIDMap.end() ? it->second : 0;
	}

	std::string EditorID::GetEditorID(RE::FormID a_formID)
	{
		Locker locker(_lock);
		auto it = _formIDToEditorIDMap.find(a_formID);
		return it != _formIDToEditorIDMap.end() ? it->second : std::string();
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
}

bool Cache::Archetype::Matches(Archetype a_archetype, const StringVec& a_strings)
{
	if (auto it = archetypeMap.find(a_archetype); it != archetypeMap.end()) {
		auto archetypeStr = it->second;
		return std::ranges::any_of(a_strings, [&](const auto& str) {
			return string::iequals(archetypeStr, str);
		});
	}
	return false;
}
