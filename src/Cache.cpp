#include "Cache.h"

namespace Cache
{
	std::string EditorID::GetEditorID(RE::TESForm* a_form)
	{
		if (!a_form) {
			return {};
		}
		switch (a_form->GetFormType()) {
		case RE::FormType::Keyword:
		case RE::FormType::LocationRefType:
		case RE::FormType::Action:
		case RE::FormType::MenuIcon:
		case RE::FormType::Global:
		case RE::FormType::HeadPart:
		case RE::FormType::Race:
		case RE::FormType::Sound:
		case RE::FormType::Script:
		case RE::FormType::Navigation:
		case RE::FormType::Cell:
		case RE::FormType::WorldSpace:
		case RE::FormType::Land:
		case RE::FormType::NavMesh:
		case RE::FormType::Dialogue:
		case RE::FormType::Quest:
		case RE::FormType::Idle:
		case RE::FormType::AnimatedObject:
		case RE::FormType::ImageAdapter:
		case RE::FormType::VoiceType:
		case RE::FormType::Ragdoll:
		case RE::FormType::DefaultObject:
		case RE::FormType::MusicType:
		case RE::FormType::StoryManagerBranchNode:
		case RE::FormType::StoryManagerQuestNode:
		case RE::FormType::StoryManagerEventNode:
		case RE::FormType::SoundRecord:
			return a_form->GetFormEditorID();
		default:
			{
                static auto tweaks = GetModuleHandleA("po3_Tweaks");
				static auto function = reinterpret_cast<_GetFormEditorID>(GetProcAddress(tweaks, "GetFormEditorID"));
				if (function) {
					return function(a_form->GetFormID());
				}
				return {};
			}
		}
	}

	bool FormType::IsFilter(RE::FormType a_type)
	{
		return std::ranges::find(set, a_type) != set.end();
	}

	ITEM::TYPE Item::GetType(const std::string& a_type)
	{
        const auto it = map.find(a_type);
		return it != map.end() ? it->second : ITEM::TYPE::kNone;
	}

	std::string Item::GetType(ITEM::TYPE a_type)
	{
        const auto it = reverse_map.find(a_type);
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
