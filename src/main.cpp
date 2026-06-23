#include "Distribute.h"
#include "ExclusiveGroups.h"
#include "Hooks.h"
#include "LookupConfigs.h"
#include "LookupForms.h"

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
			}
			break;
		case SKSE::MessagingInterface::kPostPostLoad:
			{
				logger::info("{:*^50}", "MERGES");
				MergeMapperPluginAPI::GetMergeMapperInterface001();  // Request interface
				if (g_mergeMapperInterface) {                        // Use Interface
					const auto version = g_mergeMapperInterface->GetBuildNumber();
					logger::info("\tGot MergeMapper interface buildnumber {}", version);
				} else {
					logger::info("INFO - MergeMapper not detected");
				}
			}
			break;
		case SKSE::MessagingInterface::kDataLoaded:
			{
				if (shouldLookupForms) {								
					clib_util::Timer timer;
					timer.start();
					if (Forms::LookupForms()) {
						Forms::LogFormLookup();
						timer.stop();
						logger::info("Form lookup took {}μs / {}ms", timer.duration_μs(), timer.duration_ms());

						ExclusiveGroups::Manager::GetSingleton()->LookupExclusiveGroups();
						ExclusiveGroups::Manager::GetSingleton()->LogExclusiveGroupsLookup();

						timer.start();
						Distribute::AddKeywords();
						timer.stop();

						logger::info("{:*^50}", "STATS");
						logger::info("Distribution took {}μs / {}ms", timer.duration_μs(), timer.duration_ms());
					} else {
						logger::error("No valid keywords found in configs. Distribution will be skipped.");
					}
				} else {		
					logger::warn("No configs or valid keyword entries found. Distribution will be skipped.");
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

#ifdef SKYRIM_AE
extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() {
	SKSE::PluginVersionData v;
	v.PluginVersion(Version::MAJOR);
	v.PluginName("Keyword Item Distributor");
	v.AuthorName("powerofthree");
	v.UsesAddressLibrary();
	v.UsesNoStructs();
	v.CompatibleVersions({ SKSE::RUNTIME_SSE_LATEST });

	return v;
}();
#else
extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = "Keyword Item Distributor";
	a_info->version = Version::MAJOR;

	if (a_skse->IsEditor()) {
		logger::critical("Loaded in editor, marking as incompatible"sv);
		return false;
	}

	const auto ver = a_skse->RuntimeVersion();
	if (ver <
#	ifdef SKYRIMVR
		SKSE::RUNTIME_VR_1_4_15
#	else
		SKSE::RUNTIME_SSE_1_5_39
#	endif
	) {
		logger::critical(FMT_STRING("Unsupported runtime version {}"), ver.string());
		return false;
	}

	return true;
}
#endif

void InitializeLog()
{
	auto path = logger::log_directory();
	if (!path) {
		stl::report_and_fail("Failed to find standard logging directory"sv);
	}

	*path /= Version::PROJECT;
	*path += ".log"sv;
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::info);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%H:%M:%S:%e] %v"s);

	logger::info(FMT_STRING("{} v{}"), Version::PROJECT, Version::NAME);
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	InitializeLog();

	logger::info("Game version : {}", a_skse->RuntimeVersion().string());

	SKSE::Init(a_skse, false);

	SKSE::GetMessagingInterface()->RegisterListener(MessageHandler::MessageHandler);

	return true;
}
