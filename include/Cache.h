#pragma once

namespace Cache
{
	namespace EditorID
	{
		using _GetFormEditorID = const char* (*)(std::uint32_t);

		std::string GetEditorID(const RE::TESForm* a_form);
	}

	namespace Item
	{
		enum TYPE : std::uint32_t
		{
			kNone = static_cast<std::underlying_type_t<TYPE>>(-1),
			kArmor = 0,
			kWeapon,
			kAmmo,
			kMagicEffect,
			kPotion,
			kScroll,
			kLocation,
			kIngredient,
			kBook,
			kMiscItem,
			kKey,
			kSoulGem,
			kSpell,
			kActivator,
			kFlora,
			kFurniture,
			kRace,
			kTalkingActivator,

			kTotal
		};

		inline constexpr std::array itemTypes{
			std::pair{ "Armor"sv, "armors"sv },
			std::pair{ "Weapon"sv, "weapons"sv },
			std::pair{ "Ammo"sv, "ammo"sv },
			std::pair{ "Magic Effect"sv, "magic effects"sv },
			std::pair{ "Potion"sv, "potions"sv },
			std::pair{ "Scroll"sv, "scrolls"sv },
			std::pair{ "Location"sv, "location"sv },
			std::pair{ "Ingredient"sv, "ingredients"sv },
			std::pair{ "Book"sv, "books"sv },
			std::pair{ "Misc Item"sv, "misc items"sv },
			std::pair{ "Key"sv, "keys"sv },
			std::pair{ "Soul Gem"sv, "soul gems"sv },
			std::pair{ "Spell"sv, "spells"sv },
			std::pair{ "Activator"sv, "activators"sv },
			std::pair{ "Flora"sv, "flora"sv },
			std::pair{ "Furniture"sv, "furniture"sv },
		    std::pair{ "Race"sv, "races"sv },
		    std::pair{ "Talking Activator"sv, "talking activators"sv }
		};

		TYPE             GetType(const std::string& a_type);
		std::string_view GetType(TYPE a_type);
	}

	namespace FormType
	{
		inline constexpr std::array set{
			//types
			RE::FormType::Armor,
			RE::FormType::Weapon,
			RE::FormType::Ammo,
			RE::FormType::MagicEffect,
			RE::FormType::AlchemyItem,
			RE::FormType::Scroll,
			RE::FormType::Location,
			RE::FormType::Ingredient,
			RE::FormType::Book,
			RE::FormType::Misc,
			RE::FormType::KeyMaster,
			RE::FormType::SoulGem,
			RE::FormType::Activator,
			RE::FormType::Flora,
			RE::FormType::Furniture,
			RE::FormType::Race,
			RE::FormType::TalkingActivator,
			//filters
			RE::FormType::EffectShader,
			RE::FormType::ReferenceEffect,
			RE::FormType::ArtObject,
			RE::FormType::MusicType,
			RE::FormType::Faction,
			RE::FormType::Keyword,
			RE::FormType::Spell,  // also type
			RE::FormType::Projectile,
			RE::FormType::EquipSlot,
			RE::FormType::VoiceType,
			RE::FormType::LeveledItem,
			RE::FormType::FormList
		};

		bool IsFilter(RE::FormType a_type);
	}

	namespace Archetype
	{
		inline const StringMap<RE::EffectArchetype> map{
			{ "None"sv, RE::EffectArchetype::kNone },
			{ "ValueMod"sv, RE::EffectArchetype::kValueModifier },
			{ "Script"sv, RE::EffectArchetype::kScript },
			{ "Dispel"sv, RE::EffectArchetype::kDispel },
			{ "CureDisease"sv, RE::EffectArchetype::kCureDisease },
			{ "Absorb"sv, RE::EffectArchetype::kAbsorb },
			{ "DualValueMod"sv, RE::EffectArchetype::kDualValueModifier },
			{ "Calm"sv, RE::EffectArchetype::kCalm },
			{ "Demoralize"sv, RE::EffectArchetype::kDemoralize },
			{ "Frenzy"sv, RE::EffectArchetype::kFrenzy },
			{ "Disarm"sv, RE::EffectArchetype::kDisarm },
			{ "CommandSummoned"sv, RE::EffectArchetype::kCommandSummoned },
			{ "Invisibility"sv, RE::EffectArchetype::kInvisibility },
			{ "Light"sv, RE::EffectArchetype::kLight },
			{ "Darkness"sv, RE::EffectArchetype::kDarkness },
			{ "NightEye"sv, RE::EffectArchetype::kNightEye },
			{ "Lock"sv, RE::EffectArchetype::kLock },
			{ "Open"sv, RE::EffectArchetype::kOpen },
			{ "BoundWeapon"sv, RE::EffectArchetype::kBoundWeapon },
			{ "SummonCreature"sv, RE::EffectArchetype::kSummonCreature },
			{ "DetectLife"sv, RE::EffectArchetype::kDetectLife },
			{ "Telekinesis"sv, RE::EffectArchetype::kTelekinesis },
			{ "Paralysis"sv, RE::EffectArchetype::kParalysis },
			{ "Reanimate"sv, RE::EffectArchetype::kReanimate },
			{ "SoulTrap"sv, RE::EffectArchetype::kSoulTrap },
			{ "TurnUndead"sv, RE::EffectArchetype::kTurnUndead },
			{ "Guide"sv, RE::EffectArchetype::kGuide },
			{ "WerewolfFeed"sv, RE::EffectArchetype::kWerewolfFeed },
			{ "CureParalysis"sv, RE::EffectArchetype::kCureParalysis },
			{ "CureAddiction"sv, RE::EffectArchetype::kCureAddiction },
			{ "CurePoison"sv, RE::EffectArchetype::kCurePoison },
			{ "Concussion"sv, RE::EffectArchetype::kConcussion },
			{ "ValueAndParts"sv, RE::EffectArchetype::kValueAndParts },
			{ "AccumulateMagnitude"sv, RE::EffectArchetype::kAccumulateMagnitude },
			{ "Stagger"sv, RE::EffectArchetype::kStagger },
			{ "PeakValueMod"sv, RE::EffectArchetype::kPeakValueModifier },
			{ "Cloak"sv, RE::EffectArchetype::kCloak },
			{ "Werewolf"sv, RE::EffectArchetype::kWerewolf },
			{ "SlowTime"sv, RE::EffectArchetype::kSlowTime },
			{ "Rally"sv, RE::EffectArchetype::kRally },
			{ "EnhanceWeapon"sv, RE::EffectArchetype::kEnhanceWeapon },
			{ "SpawnHazard"sv, RE::EffectArchetype::kSpawnHazard },
			{ "Etherealize"sv, RE::EffectArchetype::kEtherealize },
			{ "Banish"sv, RE::EffectArchetype::kBanish },
			{ "SpawnScriptedRef"sv, RE::EffectArchetype::kSpawnScriptedRef },
			{ "Disguise"sv, RE::EffectArchetype::kDisguise },
			{ "GrabActor"sv, RE::EffectArchetype::kGrabActor },
			{ "VampireLord"sv, RE::EffectArchetype::kVampireLord }
		};
	}

	namespace ActorValue
	{
		inline const StringMap<RE::ActorValue> map{
			{ "Aggression"sv, RE::ActorValue::kAggression },
			{ "Confidence"sv, RE::ActorValue::kConfidence },
			{ "Energy"sv, RE::ActorValue::kEnergy },
			{ "Morality"sv, RE::ActorValue::kMorality },
			{ "Mood"sv, RE::ActorValue::kMood },
			{ "Assistance"sv, RE::ActorValue::kAssistance },
			{ "OneHanded"sv, RE::ActorValue::kOneHanded },
			{ "TwoHanded"sv, RE::ActorValue::kTwoHanded },
			{ "Marksman"sv, RE::ActorValue::kArchery },
			{ "Block"sv, RE::ActorValue::kBlock },
			{ "Smithing"sv, RE::ActorValue::kSmithing },
			{ "HeavyArmor"sv, RE::ActorValue::kHeavyArmor },
			{ "LightArmor"sv, RE::ActorValue::kLightArmor },
			{ "Pickpocket"sv, RE::ActorValue::kPickpocket },
			{ "Lockpicking"sv, RE::ActorValue::kLockpicking },
			{ "Sneak"sv, RE::ActorValue::kSneak },
			{ "Alchemy"sv, RE::ActorValue::kAlchemy },
			{ "Speechcraft"sv, RE::ActorValue::kSpeech },
			{ "Alteration"sv, RE::ActorValue::kAlteration },
			{ "Conjuration"sv, RE::ActorValue::kConjuration },
			{ "Destruction"sv, RE::ActorValue::kDestruction },
			{ "Illusion"sv, RE::ActorValue::kIllusion },
			{ "Restoration"sv, RE::ActorValue::kRestoration },
			{ "Enchanting"sv, RE::ActorValue::kEnchanting },
			{ "Health"sv, RE::ActorValue::kHealth },
			{ "Magicka"sv, RE::ActorValue::kMagicka },
			{ "Stamina"sv, RE::ActorValue::kStamina },
			{ "HealRate"sv, RE::ActorValue::kHealRate },
			{ "MagickaRate"sv, RE::ActorValue::kMagickaRate },
			{ "StaminaRate"sv, RE::ActorValue::kStaminaRate },
			{ "SpeedMult"sv, RE::ActorValue::kSpeedMult },
			{ "InventoryWeight"sv, RE::ActorValue::kInventoryWeight },
			{ "CarryWeight"sv, RE::ActorValue::kCarryWeight },
			{ "CritChance"sv, RE::ActorValue::kCriticalChance },
			{ "MeleeDamage"sv, RE::ActorValue::kMeleeDamage },
			{ "UnarmedDamage"sv, RE::ActorValue::kUnarmedDamage },
			{ "Mass"sv, RE::ActorValue::kMass },
			{ "VoicePoints"sv, RE::ActorValue::kVoicePoints },
			{ "VoiceRate"sv, RE::ActorValue::kVoiceRate },
			{ "DamageResist"sv, RE::ActorValue::kDamageResist },
			{ "PoisonResist"sv, RE::ActorValue::kPoisonResist },
			{ "FireResist"sv, RE::ActorValue::kResistFire },
			{ "ElectricResist"sv, RE::ActorValue::kResistShock },
			{ "FrostResist"sv, RE::ActorValue::kResistFrost },
			{ "MagicResist"sv, RE::ActorValue::kResistMagic },
			{ "DiseaseResist"sv, RE::ActorValue::kResistDisease },
			{ "PerceptionCondition"sv, RE::ActorValue::kPerceptionCondition },
			{ "EnduranceCondition"sv, RE::ActorValue::kEnduranceCondition },
			{ "LeftAttackCondition"sv, RE::ActorValue::kLeftAttackCondition },
			{ "RightAttackCondition"sv, RE::ActorValue::kRightAttackCondition },
			{ "LeftMobilityCondition"sv, RE::ActorValue::kLeftMobilityCondition },
			{ "RightMobilityCondition"sv, RE::ActorValue::kRightMobilityCondition },
			{ "BrainCondition"sv, RE::ActorValue::kBrainCondition },
			{ "Paralysis"sv, RE::ActorValue::kParalysis },
			{ "Invisibility"sv, RE::ActorValue::kInvisibility },
			{ "NightEye"sv, RE::ActorValue::kNightEye },
			{ "DetectLifeRange"sv, RE::ActorValue::kDetectLifeRange },
			{ "WaterBreathing"sv, RE::ActorValue::kWaterBreathing },
			{ "WaterWalking"sv, RE::ActorValue::kWaterWalking },
			{ "IgnoreCrippledLimbs"sv, RE::ActorValue::kIgnoreCrippledLimbs },
			{ "Fame"sv, RE::ActorValue::kFame },
			{ "Infamy"sv, RE::ActorValue::kInfamy },
			{ "JumpingBonus"sv, RE::ActorValue::kJumpingBonus },
			{ "WardPower"sv, RE::ActorValue::kWardPower },
			{ "RightItemCharge"sv, RE::ActorValue::kRightItemCharge },
			{ "ArmorPerks"sv, RE::ActorValue::kArmorPerks },
			{ "ShieldPerks"sv, RE::ActorValue::kShieldPerks },
			{ "WardDeflection"sv, RE::ActorValue::kWardDeflection },
			{ "Variable01"sv, RE::ActorValue::kVariable01 },
			{ "Variable02"sv, RE::ActorValue::kVariable02 },
			{ "Variable03"sv, RE::ActorValue::kVariable03 },
			{ "Variable04"sv, RE::ActorValue::kVariable04 },
			{ "Variable05"sv, RE::ActorValue::kVariable05 },
			{ "Variable06"sv, RE::ActorValue::kVariable06 },
			{ "Variable07"sv, RE::ActorValue::kVariable07 },
			{ "Variable08"sv, RE::ActorValue::kVariable08 },
			{ "Variable09"sv, RE::ActorValue::kVariable09 },
			{ "Variable10"sv, RE::ActorValue::kVariable10 },
			{ "BowSpeedBonus"sv, RE::ActorValue::kBowSpeedBonus },
			{ "FavorActive"sv, RE::ActorValue::kFavorActive },
			{ "FavorsPerDay"sv, RE::ActorValue::kFavorsPerDay },
			{ "FavorsPerDayTimer"sv, RE::ActorValue::kFavorsPerDayTimer },
			{ "LeftItemCharge"sv, RE::ActorValue::kLeftItemCharge },
			{ "AbsorbChance"sv, RE::ActorValue::kAbsorbChance },
			{ "Blindness"sv, RE::ActorValue::kBlindness },
			{ "WeaponSpeedMult"sv, RE::ActorValue::kWeaponSpeedMult },
			{ "ShoutRecoveryMult"sv, RE::ActorValue::kShoutRecoveryMult },
			{ "BowStaggerBonus"sv, RE::ActorValue::kBowStaggerBonus },
			{ "Telekinesis"sv, RE::ActorValue::kTelekinesis },
			{ "FavorPointsBonus"sv, RE::ActorValue::kFavorPointsBonus },
			{ "LastBribedIntimidated"sv, RE::ActorValue::kLastBribedIntimidated },
			{ "LastFlattered"sv, RE::ActorValue::kLastFlattered },
			{ "MovementNoiseMult"sv, RE::ActorValue::kMovementNoiseMult },
			{ "BypassVendorStolenCheck"sv, RE::ActorValue::kBypassVendorStolenCheck },
			{ "BypassVendorKeywordCheck"sv, RE::ActorValue::kBypassVendorKeywordCheck },
			{ "WaitingForPlayer"sv, RE::ActorValue::kWaitingForPlayer },
			{ "OneHandedMod"sv, RE::ActorValue::kOneHandedModifier },
			{ "TwoHandedMod"sv, RE::ActorValue::kTwoHandedModifier },
			{ "MarksmanMod"sv, RE::ActorValue::kMarksmanModifier },
			{ "BlockMod"sv, RE::ActorValue::kBlockModifier },
			{ "SmithingMod"sv, RE::ActorValue::kSmithingModifier },
			{ "HeavyArmorMod"sv, RE::ActorValue::kHeavyArmorModifier },
			{ "LightArmorMod"sv, RE::ActorValue::kLightArmorModifier },
			{ "PickPocketMod"sv, RE::ActorValue::kPickpocketModifier },
			{ "LockpickingMod"sv, RE::ActorValue::kLockpickingModifier },
			{ "SneakMod"sv, RE::ActorValue::kSneakingModifier },
			{ "AlchemyMod"sv, RE::ActorValue::kAlchemyModifier },
			{ "SpeechcraftMod"sv, RE::ActorValue::kSpeechcraftModifier },
			{ "AlterationMod"sv, RE::ActorValue::kAlterationModifier },
			{ "ConjurationMod"sv, RE::ActorValue::kConjurationModifier },
			{ "DestructionMod"sv, RE::ActorValue::kDestructionModifier },
			{ "IllusionMod"sv, RE::ActorValue::kIllusionModifier },
			{ "RestorationMod"sv, RE::ActorValue::kRestorationModifier },
			{ "EnchantingMod"sv, RE::ActorValue::kEnchantingModifier },
			{ "OneHandedSkillAdvance"sv, RE::ActorValue::kOneHandedSkillAdvance },
			{ "TwoHandedSkillAdvance"sv, RE::ActorValue::kTwoHandedSkillAdvance },
			{ "MarksmanSkillAdvance"sv, RE::ActorValue::kMarksmanSkillAdvance },
			{ "BlockSkillAdvance"sv, RE::ActorValue::kBlockSkillAdvance },
			{ "SmithingSkillAdvance"sv, RE::ActorValue::kSmithingSkillAdvance },
			{ "HeavyArmorSkillAdvance"sv, RE::ActorValue::kHeavyArmorSkillAdvance },
			{ "LightArmorSkillAdvance"sv, RE::ActorValue::kLightArmorSkillAdvance },
			{ "PickPocketSkillAdvance"sv, RE::ActorValue::kPickpocketSkillAdvance },
			{ "LockpickingSkillAdvance"sv, RE::ActorValue::kLockpickingSkillAdvance },
			{ "SneakSkillAdvance"sv, RE::ActorValue::kSneakingSkillAdvance },
			{ "AlchemySkillAdvance"sv, RE::ActorValue::kAlchemySkillAdvance },
			{ "SpeechcraftSkillAdvance"sv, RE::ActorValue::kSpeechcraftSkillAdvance },
			{ "AlterationSkillAdvance"sv, RE::ActorValue::kAlterationSkillAdvance },
			{ "ConjurationSkillAdvance"sv, RE::ActorValue::kConjurationSkillAdvance },
			{ "DestructionSkillAdvance"sv, RE::ActorValue::kDestructionSkillAdvance },
			{ "IllusionSkillAdvance"sv, RE::ActorValue::kIllusionSkillAdvance },
			{ "RestorationSkillAdvance"sv, RE::ActorValue::kRestorationSkillAdvance },
			{ "EnchantingSkillAdvance"sv, RE::ActorValue::kEnchantingSkillAdvance },
			{ "LeftWeaponSpeedMult"sv, RE::ActorValue::kLeftWeaponSpeedMultiply },
			{ "DragonSouls"sv, RE::ActorValue::kDragonSouls },
			{ "CombatHealthRegenMult"sv, RE::ActorValue::kCombatHealthRegenMultiply },
			{ "OneHandedPowerMod"sv, RE::ActorValue::kOneHandedPowerModifier },
			{ "TwoHandedPowerMod"sv, RE::ActorValue::kTwoHandedPowerModifier },
			{ "MarksmanPowerMod"sv, RE::ActorValue::kMarksmanPowerModifier },
			{ "BlockPowerMod"sv, RE::ActorValue::kBlockPowerModifier },
			{ "SmithingPowerMod"sv, RE::ActorValue::kSmithingPowerModifier },
			{ "HeavyArmorPowerMod"sv, RE::ActorValue::kHeavyArmorPowerModifier },
			{ "LightArmorPowerMod"sv, RE::ActorValue::kLightArmorPowerModifier },
			{ "PickPocketPowerMod"sv, RE::ActorValue::kPickpocketPowerModifier },
			{ "LockpickingPowerMod"sv, RE::ActorValue::kLockpickingPowerModifier },
			{ "SneakPowerMod"sv, RE::ActorValue::kSneakingPowerModifier },
			{ "AlchemyPowerMod"sv, RE::ActorValue::kAlchemyPowerModifier },
			{ "SpeechcraftPowerMod"sv, RE::ActorValue::kSpeechcraftPowerModifier },
			{ "AlterationPowerMod"sv, RE::ActorValue::kAlterationPowerModifier },
			{ "ConjurationPowerMod"sv, RE::ActorValue::kConjurationPowerModifier },
			{ "DestructionPowerMod"sv, RE::ActorValue::kDestructionPowerModifier },
			{ "IllusionPowerMod"sv, RE::ActorValue::kIllusionPowerModifier },
			{ "RestorationPowerMod"sv, RE::ActorValue::kRestorationPowerModifier },
			{ "EnchantingPowerMod"sv, RE::ActorValue::kEnchantingPowerModifier },
			{ "DragonRend"sv, RE::ActorValue::kDragonRend },
			{ "AttackDamageMult"sv, RE::ActorValue::kAttackDamageMult },
			{ "HealRateMult"sv, RE::ActorValue::kHealRateMult },
			{ "MagickaRateMult"sv, RE::ActorValue::kMagickaRateMult },
			{ "StaminaRateMult"sv, RE::ActorValue::kStaminaRateMult },
			{ "WerewolfPerks"sv, RE::ActorValue::kWerewolfPerks },
			{ "VampirePerks"sv, RE::ActorValue::kVampirePerks },
			{ "GrabActorOffset"sv, RE::ActorValue::kGrabActorOffset },
			{ "Grabbed"sv, RE::ActorValue::kGrabbed },
			{ "DEPRECATED05"sv, RE::ActorValue::kDEPRECATED05 },
			{ "ReflectDamage"sv, RE::ActorValue::kReflectDamage },
		};

		inline const Map<RE::ActorValue, std::string_view> r_map{
			{ RE::ActorValue::kAggression, "Aggression"sv },
			{ RE::ActorValue::kConfidence, "Confidence"sv },
			{ RE::ActorValue::kEnergy, "Energy"sv },
			{ RE::ActorValue::kMorality, "Morality"sv },
			{ RE::ActorValue::kMood, "Mood"sv },
			{ RE::ActorValue::kAssistance, "Assistance"sv },
			{ RE::ActorValue::kOneHanded, "OneHanded"sv },
			{ RE::ActorValue::kTwoHanded, "TwoHanded"sv },
			{ RE::ActorValue::kArchery, "Marksman"sv },
			{ RE::ActorValue::kBlock, "Block"sv },
			{ RE::ActorValue::kSmithing, "Smithing"sv },
			{ RE::ActorValue::kHeavyArmor, "HeavyArmor"sv },
			{ RE::ActorValue::kLightArmor, "LightArmor"sv },
			{ RE::ActorValue::kPickpocket, "Pickpocket"sv },
			{ RE::ActorValue::kLockpicking, "Lockpicking"sv },
			{ RE::ActorValue::kSneak, "Sneak"sv },
			{ RE::ActorValue::kAlchemy, "Alchemy"sv },
			{ RE::ActorValue::kSpeech, "Speechcraft"sv },
			{ RE::ActorValue::kAlteration, "Alteration"sv },
			{ RE::ActorValue::kConjuration, "Conjuration"sv },
			{ RE::ActorValue::kDestruction, "Destruction"sv },
			{ RE::ActorValue::kIllusion, "Illusion"sv },
			{ RE::ActorValue::kRestoration, "Restoration"sv },
			{ RE::ActorValue::kEnchanting, "Enchanting"sv },
			{ RE::ActorValue::kHealth, "Health"sv },
			{ RE::ActorValue::kMagicka, "Magicka"sv },
			{ RE::ActorValue::kStamina, "Stamina"sv },
			{ RE::ActorValue::kHealRate, "HealRate"sv },
			{ RE::ActorValue::kMagickaRate, "MagickaRate"sv },
			{ RE::ActorValue::kStaminaRate, "StaminaRate"sv },
			{ RE::ActorValue::kSpeedMult, "SpeedMult"sv },
			{ RE::ActorValue::kInventoryWeight, "InventoryWeight"sv },
			{ RE::ActorValue::kCarryWeight, "CarryWeight"sv },
			{ RE::ActorValue::kCriticalChance, "CritChance"sv },
			{ RE::ActorValue::kMeleeDamage, "MeleeDamage"sv },
			{ RE::ActorValue::kUnarmedDamage, "UnarmedDamage"sv },
			{ RE::ActorValue::kMass, "Mass"sv },
			{ RE::ActorValue::kVoicePoints, "VoicePoints"sv },
			{ RE::ActorValue::kVoiceRate, "VoiceRate"sv },
			{ RE::ActorValue::kDamageResist, "DamageResist"sv },
			{ RE::ActorValue::kPoisonResist, "PoisonResist"sv },
			{ RE::ActorValue::kResistFire, "FireResist"sv },
			{ RE::ActorValue::kResistShock, "ElectricResist"sv },
			{ RE::ActorValue::kResistFrost, "FrostResist"sv },
			{ RE::ActorValue::kResistMagic, "MagicResist"sv },
			{ RE::ActorValue::kResistDisease, "DiseaseResist"sv },
			{ RE::ActorValue::kPerceptionCondition, "PerceptionCondition"sv },
			{ RE::ActorValue::kEnduranceCondition, "EnduranceCondition"sv },
			{ RE::ActorValue::kLeftAttackCondition, "LeftAttackCondition"sv },
			{ RE::ActorValue::kRightAttackCondition, "RightAttackCondition"sv },
			{ RE::ActorValue::kLeftMobilityCondition, "LeftMobilityCondition"sv },
			{ RE::ActorValue::kRightMobilityCondition, "RightMobilityCondition"sv },
			{ RE::ActorValue::kBrainCondition, "BrainCondition"sv },
			{ RE::ActorValue::kParalysis, "Paralysis"sv },
			{ RE::ActorValue::kInvisibility, "Invisibility"sv },
			{ RE::ActorValue::kNightEye, "NightEye"sv },
			{ RE::ActorValue::kDetectLifeRange, "DetectLifeRange"sv },
			{ RE::ActorValue::kWaterBreathing, "WaterBreathing"sv },
			{ RE::ActorValue::kWaterWalking, "WaterWalking"sv },
			{ RE::ActorValue::kIgnoreCrippledLimbs, "IgnoreCrippledLimbs"sv },
			{ RE::ActorValue::kFame, "Fame"sv },
			{ RE::ActorValue::kInfamy, "Infamy"sv },
			{ RE::ActorValue::kJumpingBonus, "JumpingBonus"sv },
			{ RE::ActorValue::kWardPower, "WardPower"sv },
			{ RE::ActorValue::kRightItemCharge, "RightItemCharge"sv },
			{ RE::ActorValue::kArmorPerks, "ArmorPerks"sv },
			{ RE::ActorValue::kShieldPerks, "ShieldPerks"sv },
			{ RE::ActorValue::kWardDeflection, "WardDeflection"sv },
			{ RE::ActorValue::kVariable01, "Variable01"sv },
			{ RE::ActorValue::kVariable02, "Variable02"sv },
			{ RE::ActorValue::kVariable03, "Variable03"sv },
			{ RE::ActorValue::kVariable04, "Variable04"sv },
			{ RE::ActorValue::kVariable05, "Variable05"sv },
			{ RE::ActorValue::kVariable06, "Variable06"sv },
			{ RE::ActorValue::kVariable07, "Variable07"sv },
			{ RE::ActorValue::kVariable08, "Variable08"sv },
			{ RE::ActorValue::kVariable09, "Variable09"sv },
			{ RE::ActorValue::kVariable10, "Variable10"sv },
			{ RE::ActorValue::kBowSpeedBonus, "BowSpeedBonus"sv },
			{ RE::ActorValue::kFavorActive, "FavorActive"sv },
			{ RE::ActorValue::kFavorsPerDay, "FavorsPerDay"sv },
			{ RE::ActorValue::kFavorsPerDayTimer, "FavorsPerDayTimer"sv },
			{ RE::ActorValue::kLeftItemCharge, "LeftItemCharge"sv },
			{ RE::ActorValue::kAbsorbChance, "AbsorbChance"sv },
			{ RE::ActorValue::kBlindness, "Blindness"sv },
			{ RE::ActorValue::kWeaponSpeedMult, "WeaponSpeedMult"sv },
			{ RE::ActorValue::kShoutRecoveryMult, "ShoutRecoveryMult"sv },
			{ RE::ActorValue::kBowStaggerBonus, "BowStaggerBonus"sv },
			{ RE::ActorValue::kTelekinesis, "Telekinesis"sv },
			{ RE::ActorValue::kFavorPointsBonus, "FavorPointsBonus"sv },
			{ RE::ActorValue::kLastBribedIntimidated, "LastBribedIntimidated"sv },
			{ RE::ActorValue::kLastFlattered, "LastFlattered"sv },
			{ RE::ActorValue::kMovementNoiseMult, "MovementNoiseMult"sv },
			{ RE::ActorValue::kBypassVendorStolenCheck, "BypassVendorStolenCheck"sv },
			{ RE::ActorValue::kBypassVendorKeywordCheck, "BypassVendorKeywordCheck"sv },
			{ RE::ActorValue::kWaitingForPlayer, "WaitingForPlayer"sv },
			{ RE::ActorValue::kOneHandedModifier, "OneHandedMod"sv },
			{ RE::ActorValue::kTwoHandedModifier, "TwoHandedMod"sv },
			{ RE::ActorValue::kMarksmanModifier, "MarksmanMod"sv },
			{ RE::ActorValue::kBlockModifier, "BlockMod"sv },
			{ RE::ActorValue::kSmithingModifier, "SmithingMod"sv },
			{ RE::ActorValue::kHeavyArmorModifier, "HeavyArmorMod"sv },
			{ RE::ActorValue::kLightArmorModifier, "LightArmorMod"sv },
			{ RE::ActorValue::kPickpocketModifier, "PickPocketMod"sv },
			{ RE::ActorValue::kLockpickingModifier, "LockpickingMod"sv },
			{ RE::ActorValue::kSneakingModifier, "SneakMod"sv },
			{ RE::ActorValue::kAlchemyModifier, "AlchemyMod"sv },
			{ RE::ActorValue::kSpeechcraftModifier, "SpeechcraftMod"sv },
			{ RE::ActorValue::kAlterationModifier, "AlterationMod"sv },
			{ RE::ActorValue::kConjurationModifier, "ConjurationMod"sv },
			{ RE::ActorValue::kDestructionModifier, "DestructionMod"sv },
			{ RE::ActorValue::kIllusionModifier, "IllusionMod"sv },
			{ RE::ActorValue::kRestorationModifier, "RestorationMod"sv },
			{ RE::ActorValue::kEnchantingModifier, "EnchantingMod"sv },
			{ RE::ActorValue::kOneHandedSkillAdvance, "OneHandedSkillAdvance"sv },
			{ RE::ActorValue::kTwoHandedSkillAdvance, "TwoHandedSkillAdvance"sv },
			{ RE::ActorValue::kMarksmanSkillAdvance, "MarksmanSkillAdvance"sv },
			{ RE::ActorValue::kBlockSkillAdvance, "BlockSkillAdvance"sv },
			{ RE::ActorValue::kSmithingSkillAdvance, "SmithingSkillAdvance"sv },
			{ RE::ActorValue::kHeavyArmorSkillAdvance, "HeavyArmorSkillAdvance"sv },
			{ RE::ActorValue::kLightArmorSkillAdvance, "LightArmorSkillAdvance"sv },
			{ RE::ActorValue::kPickpocketSkillAdvance, "PickPocketSkillAdvance"sv },
			{ RE::ActorValue::kLockpickingSkillAdvance, "LockpickingSkillAdvance"sv },
			{ RE::ActorValue::kSneakingSkillAdvance, "SneakSkillAdvance"sv },
			{ RE::ActorValue::kAlchemySkillAdvance, "AlchemySkillAdvance"sv },
			{ RE::ActorValue::kSpeechcraftSkillAdvance, "SpeechcraftSkillAdvance"sv },
			{ RE::ActorValue::kAlterationSkillAdvance, "AlterationSkillAdvance"sv },
			{ RE::ActorValue::kConjurationSkillAdvance, "ConjurationSkillAdvance"sv },
			{ RE::ActorValue::kDestructionSkillAdvance, "DestructionSkillAdvance"sv },
			{ RE::ActorValue::kIllusionSkillAdvance, "IllusionSkillAdvance"sv },
			{ RE::ActorValue::kRestorationSkillAdvance, "RestorationSkillAdvance"sv },
			{ RE::ActorValue::kEnchantingSkillAdvance, "EnchantingSkillAdvance"sv },
			{ RE::ActorValue::kLeftWeaponSpeedMultiply, "LeftWeaponSpeedMult"sv },
			{ RE::ActorValue::kDragonSouls, "DragonSouls"sv },
			{ RE::ActorValue::kCombatHealthRegenMultiply, "CombatHealthRegenMult"sv },
			{ RE::ActorValue::kOneHandedPowerModifier, "OneHandedPowerMod"sv },
			{ RE::ActorValue::kTwoHandedPowerModifier, "TwoHandedPowerMod"sv },
			{ RE::ActorValue::kMarksmanPowerModifier, "MarksmanPowerMod"sv },
			{ RE::ActorValue::kBlockPowerModifier, "BlockPowerMod"sv },
			{ RE::ActorValue::kSmithingPowerModifier, "SmithingPowerMod"sv },
			{ RE::ActorValue::kHeavyArmorPowerModifier, "HeavyArmorPowerMod"sv },
			{ RE::ActorValue::kLightArmorPowerModifier, "LightArmorPowerMod"sv },
			{ RE::ActorValue::kPickpocketPowerModifier, "PickPocketPowerMod"sv },
			{ RE::ActorValue::kLockpickingPowerModifier, "LockpickingPowerMod"sv },
			{ RE::ActorValue::kSneakingPowerModifier, "SneakPowerMod"sv },
			{ RE::ActorValue::kAlchemyPowerModifier, "AlchemyPowerMod"sv },
			{ RE::ActorValue::kSpeechcraftPowerModifier, "SpeechcraftPowerMod"sv },
			{ RE::ActorValue::kAlterationPowerModifier, "AlterationPowerMod"sv },
			{ RE::ActorValue::kConjurationPowerModifier, "ConjurationPowerMod"sv },
			{ RE::ActorValue::kDestructionPowerModifier, "DestructionPowerMod"sv },
			{ RE::ActorValue::kIllusionPowerModifier, "IllusionPowerMod"sv },
			{ RE::ActorValue::kRestorationPowerModifier, "RestorationPowerMod"sv },
			{ RE::ActorValue::kEnchantingPowerModifier, "EnchantingPowerMod"sv },
			{ RE::ActorValue::kDragonRend, "DragonRend"sv },
			{ RE::ActorValue::kAttackDamageMult, "AttackDamageMult"sv },
			{ RE::ActorValue::kHealRateMult, "HealRateMult"sv },
			{ RE::ActorValue::kMagickaRateMult, "MagickaRateMult"sv },
			{ RE::ActorValue::kStaminaRateMult, "StaminaRateMult"sv },
			{ RE::ActorValue::kWerewolfPerks, "WerewolfPerks"sv },
			{ RE::ActorValue::kVampirePerks, "VampirePerks"sv },
			{ RE::ActorValue::kGrabActorOffset, "GrabActorOffset"sv },
			{ RE::ActorValue::kGrabbed, "Grabbed"sv },
			{ RE::ActorValue::kDEPRECATED05, "DEPRECATED05"sv },
			{ RE::ActorValue::kReflectDamage, "ReflectDamage"sv },
		};

		std::string_view GetActorValue(RE::ActorValue a_av);
		RE::ActorValue   GetActorValue(std::string_view a_av);
	}
}
namespace EDID = Cache::EditorID;
namespace ITEM = Cache::Item;
namespace AV = Cache::ActorValue;
namespace ARCHETYPE = Cache::Archetype;
