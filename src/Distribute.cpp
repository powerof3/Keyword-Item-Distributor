#include "Distribute.h"

void Distribute::AddKeywords()
{
	ForEachDistributable([]<typename T>(Distributables<T>& a_distributables) {
		distribute(a_distributables);
	});

	logger::info("{:*^50}", "RESULT");

	ForEachDistributable([]<typename T>(Distributables<T>& a_distributables) {
		log_keyword_count(a_distributables);
	});

	// clear keywords
	Clear();
}
