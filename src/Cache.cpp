#include "Cache.h"

namespace Cache
{
	std::string EditorID::GetEditorID(const RE::TESForm* a_form)
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

	Item::TYPE Item::GetType(const std::string& a_type)
	{
		const auto it = std::ranges::find_if(itemTypes,
			[&](const auto& element) { return element.first == a_type; });
		return it != itemTypes.end() ?
		           static_cast<TYPE>(it - std::begin(itemTypes)) :
		           kNone;
	}

	std::string_view Item::GetType(TYPE a_type)
	{
		return itemTypes[a_type].first;
	}

	std::string_view ActorValue::GetActorValue(RE::ActorValue a_av)
	{
		const auto it = r_map.find(a_av);
		return it != r_map.end() ? it->second : "None";
	}

	RE::ActorValue ActorValue::GetActorValue(std::string_view a_av)
	{
		const auto it = map.find(a_av);
		return it != map.end() ? it->second : RE::ActorValue::kNone;
	}
}
