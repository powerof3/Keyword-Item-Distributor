#pragma once

//implements https://github.com/Ryan-rsm-McKenzie/CCExtender/blob/master/src/EditorIDCache.h
namespace Cache
{
	class EditorID
	{
	public:
		static EditorID* GetSingleton();

		void FillMap();

		std::string GetEditorID(RE::FormID a_formID);
		RE::FormID GetFormID(const std::string& a_editorID);

	private:
		using Lock = std::mutex;
		using Locker = std::scoped_lock<Lock>;

		EditorID() = default;
		EditorID(const EditorID&) = delete;
		EditorID(EditorID&&) = delete;
		~EditorID() = default;

		EditorID& operator=(const EditorID&) = delete;
		EditorID& operator=(EditorID&&) = delete;

		mutable Lock _lock;
		robin_hood::unordered_flat_map<RE::FormID, std::string> _formIDToEditorIDMap;
		robin_hood::unordered_flat_map<std::string, RE::FormID> _editorIDToFormIDMap;
	};

	namespace FormType
	{
		inline constexpr frozen::set<RE::FormType, 16> set{
			//types
			{ RE::FormType::Armor },
			{ RE::FormType::Weapon },
			{ RE::FormType::Ammo },
			{ RE::FormType::MagicEffect },
			{ RE::FormType::AlchemyItem },
			{ RE::FormType::Scroll },
			{ RE::FormType::Location },
			{ RE::FormType::Ingredient },
			{ RE::FormType::Book },
			//filters
			{ RE::FormType::Keyword },
			{ RE::FormType::EffectShader },
			{ RE::FormType::ReferenceEffect },
			{ RE::FormType::ArtObject },
			{ RE::FormType::MusicType },
			{ RE::FormType::Faction },
			{ RE::FormType::Spell }
		};

		bool IsFilter(RE::FormType a_type);
	}

	namespace Item
	{
		inline constexpr frozen::map<std::string_view, ITEM::TYPE, 9> map = {
			{ "Armor"sv, ITEM::kArmor },
			{ "Weapon"sv, ITEM::kWeapon },
			{ "Ammo"sv, ITEM::kAmmo },
			{ "Magic Effect"sv, ITEM::kMagicEffect },
			{ "Potion"sv, ITEM::kPotion },
			{ "Scroll"sv, ITEM::kScroll },
			{ "Location"sv, ITEM::kLocation },
			{ "Ingredient"sv, ITEM::kIngredient },
			{ "Book"sv, ITEM::kBook }
		};

		inline constexpr frozen::map<ITEM::TYPE, std::string_view, 9> reverse_map = {
			{ ITEM::kArmor, "Armor"sv },
			{ ITEM::kWeapon, "Weapon"sv },
			{ ITEM::kAmmo, "Ammo"sv },
			{ ITEM::kMagicEffect, "Magic Effect"sv },
			{ ITEM::kPotion, "Potion"sv },
			{ ITEM::kScroll, "Scroll"sv },
			{ ITEM::kLocation, "Location"sv },
			{ ITEM::kIngredient, "Ingredient"sv },
			{ ITEM::kBook, "Book"sv }
		};

		ITEM::TYPE GetType(const std::string& a_type);
		std::string GetType(ITEM::TYPE a_type);
	}

	namespace Archetype
	{
		using Archetype = RE::EffectArchetypes::ArchetypeID;

		inline constexpr frozen::map<Archetype, std::string_view, 47> archetypeMap = {
			{ Archetype::kNone, "None"sv },
			{ Archetype::kValueModifier, "ValueMod"sv },
			{ Archetype::kScript, "Script"sv },
			{ Archetype::kDispel, "Dispel"sv },
			{ Archetype::kCureDisease, "CureDisease"sv },
			{ Archetype::kAbsorb, "Absorb"sv },
			{ Archetype::kDualValueModifier, "DualValueMod"sv },
			{ Archetype::kCalm, "Calm"sv },
			{ Archetype::kDemoralize, "Demoralize"sv },
			{ Archetype::kFrenzy, "Frenzy"sv },
			{ Archetype::kDisarm, "Disarm"sv },
			{ Archetype::kCommandSummoned, "CommandSummoned"sv },
			{ Archetype::kInvisibility, "Invisibility"sv },
			{ Archetype::kLight, "Light"sv },
			{ Archetype::kDarkness, "Darkness"sv },
			{ Archetype::kNightEye, "NightEye"sv },
			{ Archetype::kLock, "Lock"sv },
			{ Archetype::kOpen, "Open"sv },
			{ Archetype::kBoundWeapon, "BoundWeapon"sv },
			{ Archetype::kSummonCreature, "SummonCreature"sv },
			{ Archetype::kDetectLife, "DetectLife"sv },
			{ Archetype::kTelekinesis, "Telekinesis"sv },
			{ Archetype::kParalysis, "Paralysis"sv },
			{ Archetype::kReanimate, "Reanimate"sv },
			{ Archetype::kSoulTrap, "SoulTrap"sv },
			{ Archetype::kTurnUndead, "TurnUndead"sv },
			{ Archetype::kGuide, "Guide"sv },
			{ Archetype::kWerewolfFeed, "WerewolfFeed"sv },
			{ Archetype::kCureParalysis, "CureParalysis"sv },
			{ Archetype::kCureAddiction, "CureAddiction"sv },
			{ Archetype::kCurePoison, "CurePoison"sv },
			{ Archetype::kConcussion, "Concussion"sv },
			{ Archetype::kValueAndParts, "ValueAndParts"sv },
			{ Archetype::kAccumulateMagnitude, "AccumulateMagnitude"sv },
			{ Archetype::kStagger, "Stagger"sv },
			{ Archetype::kPeakValueModifier, "PeakValueMod"sv },
			{ Archetype::kCloak, "Cloak"sv },
			{ Archetype::kWerewolf, "Werewolf"sv },
			{ Archetype::kSlowTime, "SlowTime"sv },
			{ Archetype::kRally, "Rally"sv },
			{ Archetype::kEnhanceWeapon, "EnhanceWeapon"sv },
			{ Archetype::kSpawnHazard, "SpawnHazard"sv },
			{ Archetype::kEtherealize, "Etherealize"sv },
			{ Archetype::kBanish, "Banish"sv },
			{ Archetype::kSpawnScriptedRef, "SpawnScriptedRef"sv },
			{ Archetype::kDisguise, "Disguise"sv },
			{ Archetype::kGrabActor, "GrabActor"sv },
			{ Archetype::kVampireLord, "VampireLord"sv }
		};

		bool Matches(Archetype a_archetype, const StringVec& a_strings);
	}
}
