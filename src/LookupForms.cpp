#include "LookupForms.h"

#include "KeywordData.h"
#include "KeywordDependencies.h"
#include "LookupConfigs.h"

namespace Forms
{
	using namespace Keyword;

	void CreateKeywords()
	{	
		logger::info("Keywords");

		for (auto& [type, dataVec] : INI::INIs) {
			if (dataVec.empty()) {
				continue;
			}		
			logger::info("\t{}", DISTRIBUTION::GetType(type));
			for (auto& iniData : dataVec) {
				RE::BGSKeyword* keyword = iniData.rawForm.to_keyword();
				if (!keyword) {
					buffered_logger::error("\t\t[{}] {} FAIL - keyword doesn't exist", iniData.path, iniData.rawForm.to_string());
					continue;
				}
				if (keyword->formEditorID.empty()) {
					buffered_logger::error("\t\t[{}] {} FAIL - keyword editorID is empty!", iniData.path, iniData.rawForm.to_string());
					continue;
				}
				iniData.resolvedKeyword = keyword;
			}
		}

		Dependencies::InitGlobalKeywordMap();
	}

	bool LookupForms()
	{
		logger::info("{:*^50}", "LOOKUP");

		CreateKeywords();

		logger::info("Filters");

		bool empty = true;
		ForEachDistributable([&]<typename T>(Distributable<T>& a_distributable) {
			a_distributable.LookupForms();
			Dependencies::ResolveKeywords(a_distributable);
			if (!a_distributable.empty()) {
				empty = false;
			}
		});

		return !empty;
	}

	void LogFormLookup()
	{
		logger::info("{:*^50}", "PROCESSING");

		ForEachDistributable([]<typename T>(const Distributable<T>& a_distributable) {
			const auto type = a_distributable.GetType();
			if (const auto& rawKeywords = INI::INIs[type]; !rawKeywords.empty()) {
				IStringSet uniqueRawKeywords;
				uniqueRawKeywords.reserve(rawKeywords.size());
				for (const auto& iniData : rawKeywords) {
					uniqueRawKeywords.emplace(iniData.rawForm.to_string());
				}
				logger::info("Adding {}/{} unique keywords to {} ({} total entries)", a_distributable.size(), uniqueRawKeywords.size(), DISTRIBUTION::form_strings[type], rawKeywords.size());
			}
		});

		// clear raw configs
		INI::INIs.clear();

		// clear dependencies map
		allKeywords.clear();

		// Clear logger's buffer to free some memory :)
		buffered_logger::clear();
	}
}
