#include "Distribute.h"
#include "ExclusiveGroups.h"
#include "Hooks.h"
#include "LookupConfigs.h"
#include "LookupForms.h"
#include "Settings.h"

namespace MessageHandler
{
	bool shouldLookupForms{ false };
	bool shouldLogErrors{ false };

	void MessageHandler(SKSE::MessagingInterface::Message* a_message)
	{
		switch (a_message->type) {
		case SKSE::MessagingInterface::kPostLoad:
			{
				std::tie(shouldLookupForms, shouldLogErrors) = INI::GetConfigs();
				Hooks::Install();
				if (shouldLookupForms) {
					Settings::GetSingleton()->LoadSettings();
				}
			}
			break;
		case SKSE::MessagingInterface::kPostPostLoad:
			{
				REX::INFO("{:*^50}", "MERGES");
				MergeMapperPluginAPI::GetMergeMapperInterface001();  // Request interface
				if (g_mergeMapperInterface) {                        // Use Interface
					const auto version = g_mergeMapperInterface->GetBuildNumber();
					REX::INFO("Got MergeMapper interface buildnumber {}", version);
				} else {
					REX::INFO("INFO - MergeMapper not detected");
				}
			}
			break;
		case SKSE::MessagingInterface::kDataLoaded:
			{
				if (shouldLookupForms) {
					REX::FTimer timer;
					timer.Start();
					if (Forms::LookupForms()) {
						Forms::LogFormLookup();
						timer.Stop();
						REX::INFO("Form lookup took {}/{}", timer.GetDurationString_us(), timer.GetDurationString_ms());

						ExclusiveGroups::Manager::GetSingleton()->LookupExclusiveGroups();
						ExclusiveGroups::Manager::GetSingleton()->LogExclusiveGroupsLookup();

						timer.Start();
						Distribute::AddKeywords();
						timer.Stop();

						REX::INFO("{:*^50}", "STATS");
						REX::INFO("Distribution took {}/{}", timer.GetDurationString_us(), timer.GetDurationString_ms());
					} else {
						REX::ERROR("No valid keywords found in configs. Distribution will be skipped.");
					}
				} else {
					REX::WARN("No configs or valid keyword entries found. Distribution will be skipped.");
				}

				// Clear logger's buffer to free some memory :)
				buffered_logger::clear();

				const SKSE::ModCallbackEvent modEvent{ "KID_KeywordDistributionDone", {}, 0.0f, nullptr };
				SKSE::GetModCallbackEventSource()->SendEvent(&modEvent);

				if (shouldLogErrors) {
					const auto error = std::format("[KID] Errors found when reading configs. Check {}.log in {} for more info\n", Version::PROJECT, SKSE::log::log_directory()->string());
					RE::ConsoleLog::GetSingleton()->Print(error.c_str());
				}
			}
			break;
		default:
			break;
		}
	}
}

#ifdef SKYRIM_SUPPORT_AE
constexpr REL::Version MIN_ADDRESS_LIBRARY_V5_RUNTIME{ 1, 7, 99, 0 };

SKSE_PLUGIN_VERSION = []() {
	SKSE::PluginVersionData v;
	v.PluginVersion(REL::Version{ Version::MAJOR, Version::MINOR, Version::PATCH });
	v.PluginName("Keyword Item Distributor");
	v.AuthorName("powerofthree");
	v.UsesAddressLibrary();
	v.UsesNoStructs();
	v.CompatibleVersions({ SKSE::RUNTIME_SSE_LATEST });

	if constexpr (SKSE::RUNTIME_SSE_LATEST < MIN_ADDRESS_LIBRARY_V5_RUNTIME) {
		v.MinimumRequiredXSEVersion(REL::Version{ 2, 2, 5 });
	} else {
		v.MinimumRequiredXSEVersion(REL::Version{ 2, 3, 0 });
	}

	return v;
}();
#else
SKSE_PLUGIN_QUERY(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = "Keyword Item Distributor";
	a_info->version = Version::MAJOR;

	if (a_skse->IsEditor()) {
		REX::CRITICAL("Loaded in editor, marking as incompatible");
		return false;
	}

	if (const auto ver = a_skse->RuntimeVersion(); ver < SKSE::RUNTIME_SSE_1_5_39) {
		REX::CRITICAL("Unsupported runtime version {}", ver);
		return false;
	}

	return true;
}
#endif

SKSE_PLUGIN_LOAD(const SKSE::LoadInterface* a_skse)
{
	SKSE::Init(a_skse, { .log = true,
						   .logName = Version::PROJECT.data() });

	const auto runtimeVersion = a_skse->RuntimeVersion();

	REX::INFO("Game version : {}", runtimeVersion);

#ifdef SKYRIM_SUPPORT_AE
	if constexpr (SKSE::RUNTIME_SSE_LATEST < MIN_ADDRESS_LIBRARY_V5_RUNTIME) {
		if (runtimeVersion >= MIN_ADDRESS_LIBRARY_V5_RUNTIME) {
			REX::FAIL(
				"You are using a newer version of Skyrim than this version of {0} supports.\n"
				"Install the correct version of {0} for your game version.\n"
				"Runtime: {1}\n"
				"Supported: 1.6.1170 (Steam) / 1.6.1179 (GOG)",
				Version::PROJECT, runtimeVersion);
		}
	}
#endif

	SKSE::GetMessagingInterface()->RegisterListener(MessageHandler::MessageHandler);

	return true;
}
