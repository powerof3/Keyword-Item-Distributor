#include "Distribute.h"
#include "LookupConfigs.h"
#include "LookupForms.h"

namespace MessageHandler
{
	bool shouldLookupForms{ false };

	void MessageHandler(SKSE::MessagingInterface::Message* a_message)
	{
		switch (a_message->type) {
		case SKSE::MessagingInterface::kPostLoad:
			shouldLookupForms = Lookup::Config::Read();
			break;
		case SKSE::MessagingInterface::kPostPostLoad:
			{
				logger::info("{:*^30}", "MERGES");
				MergeMapperPluginAPI::GetMergeMapperInterface001();  // Request interface
				if (g_mergeMapperInterface) {                        // Use Interface
					const auto version = g_mergeMapperInterface->GetBuildNumber();
					logger::info("Got MergeMapper interface buildnumber {}", version);
				} else {
					logger::info("MergeMapper not detected");
				}
			}
			break;
		case SKSE::MessagingInterface::kDataLoaded:
			{
				if (shouldLookupForms && Lookup::Forms::GetForms()) {
					Distribute::AddKeywords();
				}
				SKSE::ModCallbackEvent modEvent{ "KID_KeywordDistributionDone", RE::BSFixedString(), 0.0f, nullptr };
				SKSE::GetModCallbackEventSource()->SendEvent(&modEvent);
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
	v.CompatibleVersions({ SKSE::RUNTIME_LATEST });

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
		SKSE::RUNTIME_1_5_39
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

	SKSE::Init(a_skse);

	SKSE::GetMessagingInterface()->RegisterListener(MessageHandler::MessageHandler);

	return true;
}
