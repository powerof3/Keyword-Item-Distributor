#include "Distribute.h"

void Distribute::AddKeywords()
{
	bool hasExclusions = ExclusiveGroups::Manager::GetSingleton()->HasExclusions();
	
	ForEachDistributable([hasExclusions]<typename T>(Distributable<T>& a_distributable) {
		distribute(a_distributable, hasExclusions);
	});

	logger::info("{:*^50}", "RESULT");

	ForEachDistributable([]<typename T>(Distributable<T>& a_distributable) {
		log_keyword_count(a_distributable);
	});

	ForEachDistributable([]<typename T>(Distributable<T>& a_distributable) {
		if (a_distributable.GetType() != DISTRIBUTION::TYPE::kBook) {
			a_distributable.clear();
		}
	});
}
