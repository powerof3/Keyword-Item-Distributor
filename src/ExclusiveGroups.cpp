#include "ExclusiveGroups.h"

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
				group.notIDs.emplace_back(IDs);
			} else {
				group.matchIDs.emplace_back(IDs);
			}
		}

		exclusiveGroups.emplace_back(group);

		return true;
	}

	namespace detail
	{
		// Resolves a raw entry to a keyword, only accepting actual Keyword forms.
		RE::BGSKeyword* lookup_keyword(const std::string& a_groupName, const RawForm& a_id)
		{
			ResolvedFilter resolvedFilter(a_id);

			if (const auto form = resolvedFilter.GetForm()) {
				if (const auto keyword = form->As<RE::BGSKeyword>()) {
					return keyword;
				}
				buffered_logger::error("\t\tExclusive Group ({}): Attempted to add invalid Form {} to the group.", a_groupName, a_id.to_string());
				return nullptr;
			}

			if (a_id.IsMod()) {
				buffered_logger::error("\t\tExclusive Group ({}): Attempted to add plugin ({}) to the group.", a_groupName, a_id.to_string());
			} else {
				buffered_logger::error("\t\tExclusive Group ({}): Attempted to add unknown Keyword {} to the group..", a_groupName, a_id.to_string());
			}

			return nullptr;
		}
	}

	void Manager::LookupExclusiveGroups(INI::ExclusiveGroupsVec& exclusiveGroups)
	{
		groups.clear();
		linkedGroups.clear();
		mutualExclusionCache.clear();

		for (auto& [name, matchIDs, notIDs, path] : exclusiveGroups) {
			auto& forms = groups[name];

			for (const auto& id : matchIDs) {
				if (const auto keyword = detail::lookup_keyword(name, id)) {
					forms.insert(keyword);
				}
			}

			for (const auto& id : notIDs) {
				if (const auto keyword = detail::lookup_keyword(name, id)) {
					forms.erase(keyword);
				}
			}
		}
		// Remove empty groups
		erase_if(groups, [](const auto& pair) { return pair.second.empty(); });

		for (auto& [name, forms] : groups) {
			for (auto& form : forms) {
				linkedGroups[form].insert(name);
			}
		}

		for (const auto& [form, groupNames] : linkedGroups) {
			Set<RE::BGSKeyword*> excluded;
			for (const auto& name : groupNames) {
				const auto& group = groups.at(name);
				excluded.insert(group.begin(), group.end());
			}
			excluded.erase(form);
			if (!excluded.empty()) {
				mutualExclusionCache.emplace(form, std::move(excluded));
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

	const Set<RE::BGSKeyword*>& Manager::MutuallyExclusiveKeywordsForKeyword(RE::BGSKeyword* form) const
	{
		static Set<RE::BGSKeyword*> empty{};
		
		const auto it = mutualExclusionCache.find(form);
		return it != mutualExclusionCache.end() ? it->second : empty;
	}

	const GroupKeywordsMap& ExclusiveGroups::Manager::GetGroups() const
	{
		return groups;
	}
}
