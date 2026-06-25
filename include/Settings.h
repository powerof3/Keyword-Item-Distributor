#pragma once

class Settings : public REX::Singleton<Settings>
{
public:
	void LoadSettings()
	{
		constexpr auto path = L"Data/SKSE/Plugins/po3_KeywordItemDistributor.ini";

		CSimpleIniA ini;
		ini.SetUnicode();

		ini.LoadFile(path);

		ini::get_value(ini, verboseLogging, "Settings", "bVerboseLogging", nullptr);

		(void)ini.SaveFile(path);
	}

	bool ShouldEnableVerboseLogging() { return verboseLogging; }

private:
	// members
	bool verboseLogging{ false };
};
