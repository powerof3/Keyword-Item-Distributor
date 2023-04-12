#include "KeywordData.h"

namespace Keyword
{
	bool Data::operator==(const Data& a_rhs) const
	{
		if (!keyword || !a_rhs.keyword) {
			return false;
		}
		return keyword->GetFormID() == a_rhs.keyword->GetFormID();
	}

	void Clear()
	{
		ForEachDistributable([]<typename T>(Distributables<T>& a_distributables) {
			a_distributables.clear();
		});
	}
}
