#pragma once

#include "LookupConfigs.h"
#include "LookupFilters.h"

namespace Keyword
{
	struct Data
	{
		Count           count{ 0 };
		RE::BGSKeyword* keyword{ nullptr };
		FilterData      filters;
	};
	using DataVec = std::vector<Data>;

	class Distributables
	{
	public:
		explicit                  operator bool() const;
		[[nodiscard]] std::size_t size() const;
		void                      clear();

		DataVec& GetKeywords();
		void     LookupForms(RE::TESDataHandler* a_dataHandler, std::string_view a_type, INI::DataVec& a_INIDataVec);

	private:
		DataVec keywords;
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
