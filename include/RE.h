#pragma once

namespace RE
{
	inline void SanitizePath(std::string& a_string)
	{
		a_string = STR::TO_LOWER(a_string);

		static const boost::regex slashPattern(R"(/+|\\+)");
		static const boost::regex leadingSlashPattern(R"(^\\+)");
		static const boost::regex meshesPattern(R"(.*?[^\s]meshes\\|^meshes\\)", boost::regex::icase);

		a_string = boost::regex_replace(a_string, slashPattern, R"(\)");
		a_string = boost::regex_replace(a_string, leadingSlashPattern, "");
		a_string = boost::regex_replace(a_string, meshesPattern, "");
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
