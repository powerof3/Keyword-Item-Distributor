#include "Distribute.h"

void Distribute::AddKeywords()
{
	ForEachDistributable([]<typename T>(Distributable<T>& a_distributable) {
		distribute(a_distributable);
	});

	logger::info("{:*^50}", "RESULT");

	ForEachDistributable([]<typename T>(Distributable<T>& a_distributable) {
		log_keyword_count(a_distributable);
	});

	// clear keywords
	Clear();
}
