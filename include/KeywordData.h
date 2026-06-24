#pragma once

#include "LookupConfigs.h"

namespace Keyword
{
	struct Data
	{
		Data(RE::BGSKeyword* a_keyword, DistributableCriteria a_criteria);

		Data(const Data& a_other);
		Data(Data&& a_other);

		bool operator==(const Data& a_rhs) const;

		// members
		std::atomic_uint32_t               count{ 0 };
		RE::BGSKeyword*                    keyword{ nullptr };
		std::vector<DistributableCriteria> filters{};
	};

	using DataVec = std::vector<Data>;

	template <class T>
	class Distributable
	{
	public:
		Distributable(DISTRIBUTION::TYPE a_type);

		explicit                  operator bool() const;
		bool                      empty() const;
		[[nodiscard]] std::size_t size() const;
		void                      clear();

		[[nodiscard]] DISTRIBUTION::TYPE GetType() const;
		[[nodiscard]] std::string_view   GetTypeString() const;
		[[nodiscard]] const DataVec&     GetKeywords() const;
		[[nodiscard]] DataVec&           GetKeywords();
		void                             LookupForms();

	private:
		DISTRIBUTION::TYPE type;
		DataVec            keywords{};
	};

	inline Distributable<RE::TESObjectARMO>       armors{ DISTRIBUTION::TYPE::kArmor };
	inline Distributable<RE::TESObjectWEAP>       weapons{ DISTRIBUTION::TYPE::kWeapon };
	inline Distributable<RE::TESAmmo>             ammo{ DISTRIBUTION::TYPE::kAmmo };
	inline Distributable<RE::EffectSetting>       magicEffects{ DISTRIBUTION::TYPE::kMagicEffect };
	inline Distributable<RE::AlchemyItem>         potions{ DISTRIBUTION::TYPE::kPotion };
	inline Distributable<RE::ScrollItem>          scrolls{ DISTRIBUTION::TYPE::kScroll };
	inline Distributable<RE::BGSLocation>         locations{ DISTRIBUTION::TYPE::kLocation };
	inline Distributable<RE::IngredientItem>      ingredients{ DISTRIBUTION::TYPE::kIngredient };
	inline Distributable<RE::TESObjectBOOK>       books{ DISTRIBUTION::TYPE::kBook };
	inline Distributable<RE::TESObjectMISC>       miscItems{ DISTRIBUTION::TYPE::kMiscItem };
	inline Distributable<RE::TESKey>              keys{ DISTRIBUTION::TYPE::kKey };
	inline Distributable<RE::TESSoulGem>          soulGems{ DISTRIBUTION::TYPE::kSoulGem };
	inline Distributable<RE::SpellItem>           spells{ DISTRIBUTION::TYPE::kSpell };
	inline Distributable<RE::TESObjectACTI>       activators{ DISTRIBUTION::TYPE::kActivator };
	inline Distributable<RE::TESFlora>            flora{ DISTRIBUTION::TYPE::kFlora };
	inline Distributable<RE::TESFurniture>        furniture{ DISTRIBUTION::TYPE::kFurniture };
	inline Distributable<RE::TESRace>             races{ DISTRIBUTION::TYPE::kRace };
	inline Distributable<RE::BGSTalkingActivator> talkingActivators{ DISTRIBUTION::TYPE::kTalkingActivator };
	inline Distributable<RE::EnchantmentItem>     enchantments{ DISTRIBUTION::TYPE::kEnchantmentItem };

	template <typename Func, typename... Args>
	void ForEachDistributable(Func&& a_func, Args&&... args)
	{
		const auto process = [&](auto&& container) {
			a_func(container, std::forward<Args>(args)...);
		};

		process(magicEffects);

		process(armors);
		process(weapons);
		process(ammo);
		process(potions);
		process(scrolls);
		process(locations);
		process(ingredients);
		process(books);
		process(miscItems);
		process(keys);
		process(soulGems);
		process(spells);
		process(activators);
		process(flora);
		process(furniture);
		process(races);
		process(talkingActivators);
		process(enchantments);
	}
}
using KeywordData = Keyword::Data;
using KeywordDataVec = Keyword::DataVec;

template <class T>
Keyword::Distributable<T>::Distributable(DISTRIBUTION::TYPE a_type) :
	type(a_type)
{}

template <class T>
Keyword::Distributable<T>::operator bool() const
{
	return !empty();
}

template <class T>
bool Keyword::Distributable<T>::empty() const
{
	return keywords.empty();
}

template <class T>
std::size_t Keyword::Distributable<T>::size() const
{
	return keywords.size();
}

template <class T>
void Keyword::Distributable<T>::clear()
{
	keywords.clear();
	keywords.shrink_to_fit();
}

template <class T>
DISTRIBUTION::TYPE Keyword::Distributable<T>::GetType() const
{
	return type;
}

template <class T>
std::string_view Keyword::Distributable<T>::GetTypeString() const
{
	return DISTRIBUTION::GetType(type);
}

template <class T>
const KeywordDataVec& Keyword::Distributable<T>::GetKeywords() const
{
	return keywords;
}

template <class T>
KeywordDataVec& Keyword::Distributable<T>::GetKeywords()
{
	return keywords;
}

template <class T>
void Keyword::Distributable<T>::LookupForms()
{
	const auto& INIDataVec = INI::INIs[type];
	if (INIDataVec.empty()) {
		return;
	}

	logger::info("\t{}", GetTypeString());

	keywords.reserve(INIDataVec.size());

	Map<RE::BGSKeyword*, std::size_t> keywordToData;
	keywordToData.reserve(INIDataVec.size());

	std::size_t entryIdx = 0;

	for (auto& iniData : INIDataVec) {
		RE::BGSKeyword* keyword = iniData.resolvedKeyword;
		if (!keyword) {
			continue;
		}
		entryIdx++;

		buffered_logger::info("\t\t[{}] {}", iniData.path, iniData.rawForm.to_string());

		DistributableCriteria criteria(iniData.criteria);

		if (!criteria.Validated()) {
			buffered_logger::error("\t\t\tInvalid/missing filters, skipping distribution\n\t\t\t\t\t\t--------------------------------------------- [{}]", entryIdx);
			continue;
		}

		if (auto [it, inserted] = keywordToData.try_emplace(keyword, keywords.size()); inserted) {
			keywords.emplace_back(keyword, std::move(criteria));
		} else {
			keywords[it->second].filters.emplace_back(std::move(criteria));
		}
	}
}
