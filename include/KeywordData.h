#pragma once

#include "LookupConfigs.h"
#include "LookupFilters.h"

namespace Keyword
{
	struct Data
	{
		RE::BGSKeyword* keyword{ nullptr };
		FilterData      filters;
	};

	using DataVec = std::vector<Data>;
	using CountMap = std::map<RE::BGSKeyword*, std::uint32_t>;

	class Distributables
	{
	public:
		explicit                  operator bool() const;
		[[nodiscard]] std::size_t size() const;
		void                      clear();

		DataVec&        GetKeywords();
		const CountMap& GetKeywordCounts();
		void            IncrementCount(RE::BGSKeyword* a_keyword);
		void            LookupForms(RE::TESDataHandler* a_dataHandler, ITEM::TYPE a_type, INI::DataVec& a_INIDataVec);

	private:
		DataVec  keywords;
		CountMap keywordCount;
	};

	inline Distributables armors;
	inline Distributables weapons;
	inline Distributables ammo;
	inline Distributables magicEffects;
	inline Distributables potions;
	inline Distributables scrolls;
	inline Distributables locations;
	inline Distributables ingredients;
	inline Distributables books;
	inline Distributables miscItems;
	inline Distributables keys;
	inline Distributables soulGems;
	inline Distributables spells;

	void Clear();
}
using KeywordDataVec = Keyword::DataVec;
