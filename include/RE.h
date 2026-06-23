#pragma once

namespace RE
{
	inline void SanitizePath(std::string& a_string)
	{
		a_string = string::tolower(a_string);

		static const srell::regex slashPattern(R"(/+|\\+)");
		static const srell::regex leadingSlashPattern(R"(^\\+)");
		static const srell::regex meshesPattern(R"(.*?[^\s]meshes\\|^meshes\\)", srell::regex::icase);

		a_string = srell::regex_replace(a_string, slashPattern, R"(\)");
		a_string = srell::regex_replace(a_string, leadingSlashPattern, "");
		a_string = srell::regex_replace(a_string, meshesPattern, "");
	}

	inline RE::EffectSetting* GetCostliestMGEF(RE::TESForm* a_form)
	{
		if (a_form) {
			if (const auto magicItem = a_form->As<RE::MagicItem>()) {
				auto effect = magicItem->GetCostliestEffectItem();
				if (const auto mgef = effect ? effect->baseEffect : nullptr) {
					return mgef;
				}
			}
		}
		return nullptr;
	}
}
