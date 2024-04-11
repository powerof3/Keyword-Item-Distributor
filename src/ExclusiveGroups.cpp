#include "ExclusiveGroups.h"
#include "KeywordData.h"

namespace ExclusiveGroups
{
	bool INI::TryParse(const std::string& a_key, const std::string& a_value, const std::string& a_path)
	{
		if (a_key != "ExclusiveGroup") {
			return false;
		}

		const auto sections = string::split(a_value, "|");
		const auto size = sections.size();

		if (size < 2) {
			logger::warn("IGNORED: ExclusiveGroup must have a name and at least one Form Filter: {} = {}"sv, a_key, a_value);
			return true;
		}

		auto split_IDs = distribution::split_entry(sections[1]);

		if (split_IDs.empty()) {
			logger::warn("ExclusiveGroup must have at least one Form Filter : {} = {}"sv, a_key, a_value);
			return true;
		}

		RawExclusiveGroup group{};
		group.name = sections[0];
		group.path = a_path;

		for (auto& IDs : split_IDs) {
			if (IDs.at(0) == '-') {
				IDs.erase(0, 1);
				group.formIDs.NOT.push_back(distribution::get_record(IDs));
			} else {
				group.formIDs.MATCH.push_back(distribution::get_record(IDs));
			}
		}

		exclusiveGroups.emplace_back(group);

		return true;
	}

	void formID_to_form(RE::TESDataHandler* const dataHandler, const std::string& groupName, RawVec& a_rawFormVec, ProcessedVec& a_formVec)
	{
		if (a_rawFormVec.empty()) {
			return;
		}

		const auto& keywordArray = dataHandler->GetFormArray<RE::BGSKeyword>();

		for (auto& formOrEditorID : a_rawFormVec) {
			std::visit(overload{
						   [&](FormModPair& formMod) {
							   auto& [formID, modName] = formMod;
							   if (g_mergeMapperInterface) {
								   Keyword::detail::get_merged_IDs(formID, modName);
							   }
							   if (modName && !formID) {
								   buffered_logger::error("\t\tExclusive Group ({}): Attempted to add plugin ({}) to the group.", groupName, *modName);
							   } else if (formID) {
								   if (auto filterForm = modName ?
					                                         dataHandler->LookupForm(*formID, *modName) :
					                                         RE::TESForm::LookupByID(*formID)) {
									   const auto formType = filterForm->GetFormType();
									   if (const auto keyword = filterForm->As<RE::BGSKeyword>(); keyword && formType == RE::FormType::Keyword) {
										   a_formVec.push_back(keyword);
									   } else {
										   buffered_logger::error("\t\tExclusive Group ({}): Attempted to add invalid Form {} [0x{:X}] ({}) to the group.", groupName, formType, *formID, modName.value_or(""));
									   }
								   } else {
									   buffered_logger::error("\t\tExclusive Group ({}): Form doesn't exist", groupName, *formID, modName.value_or(""));
								   }
							   }
						   },
						   [&](std::string& editorID) {
							   if (auto filterForm = RE::TESForm::LookupByEditorID(editorID)) {
								   const auto formType = filterForm->GetFormType();
								   if (const auto keyword = filterForm->As<RE::BGSKeyword>(); keyword && formType == RE::FormType::Keyword) {
									   a_formVec.push_back(keyword);
								   } else {
									   buffered_logger::error("\t\tExclusive Group ({}): Attempted to add invalid Form {} to the group. Expected {}, but got {}", groupName, editorID, RE::FormType::Keyword, formType);
								   }
							   } else {
								   if (auto keyword = Keyword::detail::find_existing_keyword(keywordArray, editorID, true)) {
									   a_formVec.push_back(keyword);
								   } else {
									   buffered_logger::error("\t\tExclusive Group ({}): Attempted to add unknown Keyword {} to the group.", groupName, editorID);
								   }
							   }
						   } },
				formOrEditorID);
		}
	}

	void Manager::LookupExclusiveGroups(INI::ExclusiveGroupsVec& exclusiveGroups)
	{
		groups.clear();
		linkedGroups.clear();

		const auto dataHandler = RE::TESDataHandler::GetSingleton();

		for (auto& [name, filterIDs, path] : exclusiveGroups) {
			auto&        forms = groups[name];
			ProcessedVec match{};
			ProcessedVec formsNot{};

			formID_to_form(dataHandler, name, filterIDs.MATCH, match);
			formID_to_form(dataHandler, name, filterIDs.NOT, formsNot);

			for (const auto& form : match) {
				if (std::holds_alternative<RE::TESForm*>(form)) {
					if (const auto keyword = std::get<RE::TESForm*>(form)->As<RE::BGSKeyword>(); keyword) {
						forms.insert(keyword);
					}
				}
			}

			for (auto& form : formsNot) {
				if (std::holds_alternative<RE::TESForm*>(form)) {
					if (const auto keyword = std::get<RE::TESForm*>(form)->As<RE::BGSKeyword>(); keyword) {
						forms.erase(keyword);
					}
				}
			}
		}

		// Remove empty groups
		std::erase_if(groups, [](const auto& pair) { return pair.second.empty(); });

		for (auto& [name, forms] : groups) {
			for (auto& form : forms) {
				linkedGroups[form].insert(name);
			}
		}
	}

	void Manager::LogExclusiveGroupsLookup()
	{
		if (groups.empty()) {
			return;
		}

		logger::info("{:*^50}", "EXCLUSIVE GROUPS");

		for (const auto& [group, forms] : groups) {
			logger::info("Adding '{}' exclusive group", group);
			for (const auto& form : forms) {
				logger::info("\t{}", describe(form));
			}
		}
	}

	Set<RE::BGSKeyword*> Manager::MutuallyExclusiveKeywordsForKeyword(RE::BGSKeyword* form) const
	{
		Set<RE::BGSKeyword*> forms{};
		if (auto it = linkedGroups.find(form); it != linkedGroups.end()) {
			std::ranges::for_each(it->second, [&](const Group& name) {
				const auto& group = groups.at(name);
				forms.insert(group.begin(), group.end());
			});
		}

		// Remove self from the list.
		forms.erase(form);

		return forms;
	}

	const GroupKeywordsMap& ExclusiveGroups::Manager::GetGroups() const
	{
		return groups;
	}
}
