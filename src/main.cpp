#include "Distribute.h"
#include "LookupConfigs.h"
#include "LookupForms.h"

namespace MessageHandler
{
	struct detail
	{
		static void send_event()
		{
			SKSE::ModCallbackEvent modEvent{
				"KID_KeywordDistributionDone",
				RE::BSFixedString(),
				0.0f,
				nullptr
			};

			SKSE::GetModCallbackEventSource()->SendEvent(&modEvent);
		}
	};

	void Distribute(SKSE::MessagingInterface::Message* a_message)
	{
		if (a_message->type == SKSE::MessagingInterface::kDataLoaded) {
			if (Lookup::Forms::GetForms()) {
				Distribute::AddKeywords();
			}
			detail::send_event();
		}
	}

	void NoDistribute(SKSE::MessagingInterface::Message* a_message)
	{
		if (a_message->type == SKSE::MessagingInterface::kDataLoaded) {
			detail::send_event();
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
	if (ver < SKSE::RUNTIME_1_5_39) {
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

    const auto messaging = SKSE::GetMessagingInterface();
	if (Lookup::Config::Read()) {
		messaging->RegisterListener(MessageHandler::Distribute);
	} else {
		messaging->RegisterListener(MessageHandler::NoDistribute);
	}

	return true;
}
