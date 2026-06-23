#pragma once

struct FilterRequirments;

class ItemData
{
public:
	ItemData(RE::TESForm* a_form);

	RE::FormID   GetFormID() const { return item->GetFormID(); }
	RE::TESForm* GetItem() const { return item; }

	const std::string& GetEDID() const;
	const std::string& GetName() const;
	const std::string& GetModel() const;

	bool FormFilter(RE::TESForm* a_form) const;

	bool HasStringFilter(const std::string& a_str, StringType a_stringType, std::uint32_t a_enum) const;
	bool ContainsStringFilter(const std::string& a_str, StringType a_stringType) const;

	[[nodiscard]] bool HasKeyword(RE::BGSKeyword* a_keyword) const;
	[[nodiscard]] bool HasMutuallyExclusiveKeyword(RE::BGSKeyword* a_keyword) const;
	void               AddKeyword(RE::BGSKeyword* a_keyword);

private:
	bool MatchesActorValue(RE::ActorValue a_av) const;
	bool MatchesArchetype(RE::EffectArchetype a_archetype) const;

	// members
	RE::TESForm* item{ nullptr };

	mutable std::string edid{};
	mutable std::string name{};
	mutable std::string model{};

	std::span<RE::BGSKeyword*>   originalKeywords{};
	std::vector<RE::BGSKeyword*> addedKeywords{};

	mutable bool cacheEDID{ false };
	mutable bool cacheName{ false };
	mutable bool cacheModel{ false };
};
