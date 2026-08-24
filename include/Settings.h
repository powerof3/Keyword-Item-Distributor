#pragma once

class Settings : public REX::TSingleton<Settings>
{
public:
	void LoadSettings()
	{
		constexpr auto path = "Data/SKSE/Plugins/po3_KeywordItemDistributor.ini";

		const auto store = REX::FIniSettingStore::GetSingleton();
		store->Init(path, "");

		store->Load();
		store->Save();
	}

	bool ShouldEnableVerboseLogging() { return verboseLogging.GetValue(); }

private:
	// members
	REX::TIniSetting<bool> verboseLogging{ "Settings", "bVerboseLogging", false };
};
