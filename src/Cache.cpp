#include "Cache.h"

namespace Cache
{
	RE::BGSKeyword* FindKeyword(const std::string& a_edid, bool a_skipEDID)
	{
		if (!a_skipEDID) {
			if (const auto keyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>(a_edid)) {
				return keyword;
			}
		}

		if (const auto it = addedKeywords.find(a_edid); it != addedKeywords.end()) {
			return it->second;
		}

		return nullptr;
	}

	void AddKeyword(RE::BSTArray<RE::BGSKeyword*>& a_keywordArray, RE::BGSKeyword* a_keyword)
	{
		a_keywordArray.emplace_back(a_keyword);
		addedKeywords.insert_or_assign(a_keyword->GetFormEditorID(), a_keyword);
	}
	
	RE::ActorValue ActorValue::GetActorValue(std::string_view a_av)
	{
		const auto it = map.find(a_av);
		return it != map.end() ? it->second : RE::ActorValue::kNone;
	}

	RE::ActorValue ActorValue::GetAssociatedSkill(RE::MagicItem* a_spell)
	{
		if (auto effect = a_spell->GetCostliestEffectItem(); effect && effect->baseEffect) {
			return effect->baseEffect->data.associatedSkill;
		}
		return RE::ActorValue::kNone;
	}
}
