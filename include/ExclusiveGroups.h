#pragma once
#include "LookupConfigs.h"
#include <unordered_set>

namespace ExclusiveGroups
{
	namespace INI
	{
		struct RawExclusiveGroup
		{
			std::string name{};

			/// Raw filters in RawExclusiveGroup only use NOT and MATCH, there is no meaning for ALL, so it's ignored.
			Filters<FormIDOrString> formIDs{};
			std::string             path{};
		};

		using ExclusiveGroupsVec = std::vector<RawExclusiveGroup>;

		/// <summary>
		/// A list of RawExclusiveGroups that will be processed along with configs.
		/// </summary>
		inline ExclusiveGroupsVec exclusiveGroups{};

		bool TryParse(const std::string& a_key, const std::string& a_value, const std::string& a_path);
	}

	using Group = std::string;
	using KeywordGroupMap = std::unordered_map<RE::BGSKeyword*, std::unordered_set<Group>>;
	using GroupKeywordsMap = std::unordered_map<Group, std::unordered_set<RE::BGSKeyword*>>;

	class Manager
	{
		
	public:
		static Manager* GetSingleton()
		{
			static Manager singleton;
			return std::addressof(singleton);
		}

		/// <summary>
		/// Does a forms lookup similar to what Filters do.
		///
		/// As a result this method configures Manager with discovered valid exclusive groups.
		/// </summary>
		/// <param name="rawExclusiveGroups">A raw exclusive group entries that should be processed.</param>
		void LookupExclusiveGroups(INI::ExclusiveGroupsVec& rawExclusiveGroups = INI::exclusiveGroups);

		void LogExclusiveGroupsLookup();

		/// <summary>
		/// Gets a set of all keywords that are in the same exclusive group as the given keyword.
		/// Note that a keyword can appear in multiple exclusive groups, all of those groups are returned.
		/// </summary>
		/// <param name="keyword">A keyword for which mutually exclusive keywords will be returned.</param>
		/// <returns>A union of all groups that contain a given keyword.</returns>
		std::unordered_set<RE::BGSKeyword*> MutuallyExclusiveKeywordsForKeyword(RE::BGSKeyword*) const;

		/// <summary>
		/// Retrieves all exclusive groups.
		/// </summary>
		/// <returns>A reference to discovered exclusive groups</returns>
		const GroupKeywordsMap& GetGroups() const;

	private:
		/// <summary>
		/// A map of exclusive group names related to each keyword in the exclusive groups.
		/// Provides a quick and easy way to get names of all groups that need to be checked.
		/// </summary>
		KeywordGroupMap linkedGroups{};

		/// <summary>
		///  A map of exclusive groups names and the keywords that are part of each exclusive group.
		/// </summary>
		GroupKeywordsMap groups{};

		Manager() = default;
		~Manager() = default;

		Manager(const Manager&) = delete;
		Manager(Manager&&) = delete;
		Manager& operator=(const Manager&) = delete;
		Manager& operator=(Manager&&) = delete;
	};
}
