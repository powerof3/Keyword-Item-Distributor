#include "KeywordData.h"

namespace Keyword
{
	Data::Data(RE::BGSKeyword* a_keyword, DistributableCriteria a_criteria) :
		keyword(a_keyword),
		filters{ std::move(a_criteria) }
	{}

	Data::Data(const Data& a_other) :
		keyword(a_other.keyword),
		filters(a_other.filters)
	{}

	Data::Data(Data&& a_other) :
		keyword(a_other.keyword),
		filters(std::move(a_other.filters))
	{}
	
	bool Data::operator==(const Data& a_rhs) const
	{
		if (!keyword || !a_rhs.keyword) {
			return false;
		}
		return keyword->GetFormID() == a_rhs.keyword->GetFormID();
	}
}
