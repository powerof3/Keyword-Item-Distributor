#pragma once

#include "Data\EnumDefs.h"
#include "Data\Ranges.h"

class Traits
{
public:
	virtual ~Traits() = default;

	[[nodiscard]] virtual bool PassFilter([[maybe_unused]] RE::TESForm* a_item) const { return false; }

	[[nodiscard]] static std::shared_ptr<Traits> Create(DISTRIBUTION::TYPE a_type, const std::string& a_traits);

protected:
	template <class T>
	static T extract_single_value(const std::string& a_str)
	{
		static const srell::regex re{ R"([-+]?\d*\.?\d+)" };
		if (srell::smatch m; srell::regex_search(a_str, m, re)) {
			return string::to_num<T>(m[0].str());
		}
		return static_cast<T>(-1);
	}
};

class ArmorTraits : public Traits
{
public:
	ArmorTraits(const std::string& a_traits)
	{
		auto traits = distribution::split_entry(a_traits);
		for (const auto& trait : traits) {
			if (trait.contains("AR(")) {
				armorRating = Range<float>(trait);
			} else if (trait.contains("W(")) {
				weight = Range<float>(trait);
			} else if (string::is_only_digit(trait)) {
				slot = static_cast<RE::BIPED_MODEL::BipedObjectSlot>(1 << (string::to_num<std::uint32_t>(trait) - 30));
			} else {
				switch (string::const_hash(trait)) {
				case "HEAVY"_h:
					armorType = RE::BIPED_MODEL::ArmorType::kHeavyArmor;
					break;
				case "LIGHT"_h:
					armorType = RE::BIPED_MODEL::ArmorType::kLightArmor;
					break;
				case "CLOTHING"_h:
					armorType = RE::BIPED_MODEL::ArmorType::kClothing;
					break;
				case "E"_h:
					enchanted = true;
					break;
				case "-E"_h:
					enchanted = false;
					break;
				case "T"_h:
					templated = true;
					break;
				case "-T"_h:
					templated = false;
					break;
				default:
					break;
				}
			}
		}
	}
	~ArmorTraits() override = default;

	[[nodiscard]] bool PassFilter(RE::TESForm* a_item) const override
	{
		const auto armor = a_item->As<RE::TESObjectARMO>();

		if (enchanted && (armor->formEnchanting != nullptr) != *enchanted) {
			return false;
		}
		if (templated && (armor->templateArmor != nullptr) != *templated) {
			return false;
		}
		if (armorType != static_cast<RE::BIPED_MODEL::ArmorType>(-1) && armor->GetArmorType() != armorType) {
			return false;
		}
		if (slot != RE::BIPED_MODEL::BipedObjectSlot::kNone && !armor->HasPartOf(slot)) {
			return false;
		}
		if (weight.IsValid() && !weight.IsInRange(armor->weight)) {
			return false;
		}
		if (armorRating.IsValid() && !armorRating.IsInRange(armor->GetArmorRating())) {
			return false;
		}
		return true;
	}

private:
	// members
	std::optional<bool>              enchanted{};
	std::optional<bool>              templated{};
	Range<float>                     armorRating{};
	RE::BIPED_MODEL::ArmorType       armorType{ static_cast<RE::BIPED_MODEL::ArmorType>(-1) };
	Range<float>                     weight;
	RE::BIPED_MODEL::BipedObjectSlot slot{ RE::BIPED_MODEL::BipedObjectSlot::kNone };
};

class WeaponTraits : public Traits
{
public:
	WeaponTraits(const std::string& a_traits)
	{
		auto traits = distribution::split_entry(a_traits);
		for (const auto& trait : traits) {
			if (trait.contains("W(")) {
				weight = Range<float>(trait);
			} else if (trait.contains("D(")) {
				damage = Range<float>(trait);
			} else {
				switch (string::const_hash(trait)) {
				case "HandToHandMelee"_h:
					animationType = RE::WEAPON_TYPE::kHandToHandMelee;
					break;
				case "OneHandSword"_h:
					animationType = RE::WEAPON_TYPE::kOneHandSword;
					break;
				case "OneHandDagger"_h:
					animationType = RE::WEAPON_TYPE::kOneHandDagger;
					break;
				case "OneHandAxe"_h:
					animationType = RE::WEAPON_TYPE::kOneHandAxe;
					break;
				case "OneHandMace"_h:
					animationType = RE::WEAPON_TYPE::kOneHandMace;
					break;
				case "TwoHandSword"_h:
					animationType = RE::WEAPON_TYPE::kTwoHandSword;
					break;
				case "TwoHandAxe"_h:
					animationType = RE::WEAPON_TYPE::kTwoHandAxe;
					break;
				case "Bow"_h:
					animationType = RE::WEAPON_TYPE::kBow;
					break;
				case "Staff"_h:
					animationType = RE::WEAPON_TYPE::kStaff;
					break;
				case "Crossbow"_h:
					animationType = RE::WEAPON_TYPE::kCrossbow;
					break;
				case "E"_h:
					enchanted = true;
					break;
				case "-E"_h:
					enchanted = false;
					break;
				case "T"_h:
					templated = true;
					break;
				case "-T"_h:
					templated = false;
					break;
				default:
					break;
				}
			}
		}
	}
	~WeaponTraits() override = default;

	bool PassFilter(RE::TESForm* a_item) const override
	{
		const auto weapon = a_item->As<RE::TESObjectWEAP>();

		if (enchanted && (weapon->formEnchanting != nullptr) != *enchanted) {
			return false;
		}
		if (templated && (weapon->templateWeapon != nullptr) != *templated) {
			return false;
		}
		if (animationType != static_cast<RE::WEAPON_TYPE>(-1) && weapon->GetWeaponType() != animationType) {
			return false;
		}
		if (weight.IsValid() && !weight.IsInRange(weapon->weight)) {
			return false;
		}
		if (damage.IsValid() && !damage.IsInRange(weapon->GetAttackDamage())) {
			return false;
		}
		return true;
	}

private:
	// members
	std::optional<bool> enchanted{};
	std::optional<bool> templated{};
	RE::WEAPON_TYPE     animationType{ static_cast<RE::WEAPON_TYPE>(-1) };
	Range<float>        damage{};
	Range<float>        weight{};
};

class AmmoTraits : public Traits
{
public:
	AmmoTraits(const std::string& a_traits)
	{
		auto traits = distribution::split_entry(a_traits);
		for (const auto& trait : traits) {
			if (trait.contains("D(")) {
				damage = Range<float>(trait);
			} else {
				switch (string::const_hash(trait)) {
				case "B"_h:
					isBolt = true;
					break;
				case "-B"_h:
					isBolt = false;
					break;
				default:
					break;
				}
			}
		}
	}

	~AmmoTraits() override = default;

	[[nodiscard]] bool PassFilter(RE::TESForm* a_item) const override
	{
		const auto ammo = a_item->As<RE::TESAmmo>();
		if (isBolt && ammo->IsBolt() != *isBolt) {
			return false;
		}
		if (damage.IsValid() && !damage.IsInRange(ammo->data.damage)) {
			return false;
		}
		return true;
	}

private:
	// members
	Range<float>        damage;
	std::optional<bool> isBolt;
};

class MagicEffectTraits : public Traits
{
public:
	MagicEffectTraits(const std::string& a_traits)
	{
		auto traits = distribution::split_entry(a_traits);
		for (const auto& trait : traits) {
			if (trait == "H") {
				isHostile = true;
			} else if (trait == "-H") {
				isHostile = false;
			} else if (trait == "DISPEL") {
				dispelWithKeywords = true;
			} else if (trait == "-DISPEL") {
				dispelWithKeywords = false;
			} else if (trait.contains("D(")) {
				deliveryType = extract_single_value<RE::MagicSystem::Delivery>(trait);
			} else if (trait.contains("CT(")) {
				castingType = extract_single_value<RE::MagicSystem::CastingType>(trait);
			} else if (trait.contains("R(")) {
				resistance = extract_single_value<RE::ActorValue>(trait);
			} else if (trait.contains('(')) {
				static const srell::regex re{
					R"([^-+.\d]*([-+]?\d*\.?\d+)[^-+.\d]+([-+]?\d*\.?\d+)(?:[^-+.\d]+([-+]?\d*\.?\d+))?.*)"  // 0(0/25)
				};
				if (srell::smatch m; srell::regex_match(trait, m, re)) {
					auto skillType = string::to_num<RE::ActorValue>(m[1].str());
					auto min = string::to_num<std::int32_t>(m[2].str());
					if (m[3].matched) {
						auto max = string::to_num<std::int32_t>(m[3].str());
						skill = { skillType, { min, max } };
					} else {
						skill = { skillType, { min } };
					}
				}
			}
		}
	}
	~MagicEffectTraits() override = default;

	bool PassFilter(RE::TESForm* a_item) const override
	{
		const auto mgef = a_item->As<RE::EffectSetting>();

		if (castingType != static_cast<RE::MagicSystem::CastingType>(-1) && mgef->data.castingType != castingType) {
			return false;
		}
		if (deliveryType != static_cast<RE::MagicSystem::Delivery>(-1) && mgef->data.delivery != deliveryType) {
			return false;
		}
		if (resistance != RE::ActorValue::kNone && mgef->data.resistVariable != resistance) {
			return false;
		}
		if (dispelWithKeywords && mgef->data.flags.all(RE::EffectSetting::EffectSettingData::Flag::kDispelWithKeywords) != *dispelWithKeywords) {
			return false;
		}
		if (isHostile && mgef->IsHostile() != *isHostile) {
			return false;
		}
		if (skill.second.IsValid()) {
			auto& [skillType, minMax] = skill;
			if (skillType != RE::ActorValue::kNone && skillType != mgef->GetMagickSkill()) {
				return false;
			}
			if (!minMax.IsInRange(mgef->GetMinimumSkillLevel())) {
				return false;
			}
		}
		return true;
	}

private:
	// members
	RE::MagicSystem::CastingType                   castingType{ static_cast<RE::MagicSystem::CastingType>(-1) };
	RE::MagicSystem::Delivery                      deliveryType{ static_cast<RE::MagicSystem::Delivery>(-1) };
	std::pair<RE::ActorValue, Range<std::int32_t>> skill{ RE::ActorValue::kNone, {} };
	RE::ActorValue                                 resistance{ RE::ActorValue::kNone };
	std::optional<bool>                            isHostile;
	std::optional<bool>                            dispelWithKeywords;
};

class PotionTraits : public Traits
{
public:
	PotionTraits(const std::string& a_traits)
	{
		const auto traits = distribution::split_entry(a_traits);
		for (const auto& trait : traits) {
			switch (string::const_hash(trait)) {
			case "P"_h:
				isPoison = true;
				break;
			case "-P"_h:
				isPoison = false;
				break;
			case "F"_h:
				isFood = true;
				break;
			case "-F"_h:
				isFood = false;
				break;
			default:
				break;
			}
		}
	}
	~PotionTraits() override = default;

	bool PassFilter(RE::TESForm* a_item) const override
	{
		const auto potion = a_item->As<RE::AlchemyItem>();

		if (isPoison && potion->IsPoison() != *isPoison) {
			return false;
		}
		if (isFood && potion->IsFood() != *isFood) {
			return false;
		}
		return true;
	}

private:
	// members
	std::optional<bool> isPoison;
	std::optional<bool> isFood;
};

class IngredientTraits : public Traits
{
public:
	IngredientTraits(const std::string& a_traits)
	{
		switch (string::const_hash(a_traits)) {
		case "F"_h:
			isFood = true;
			break;
		case "-F"_h:
			isFood = false;
			break;
		default:
			break;
		}
	}
	~IngredientTraits() override = default;

	bool PassFilter(RE::TESForm* a_item) const override
	{
		if (isFood && a_item->As<RE::IngredientItem>()->IsFood() != *isFood) {
			return false;
		}
		return true;
	}

private:
	// members
	std::optional<bool> isFood;
};

class BookTraits : public Traits
{
public:
	BookTraits(const std::string& a_traits)
	{
		const auto traits = distribution::split_entry(a_traits);
		for (const auto& trait : traits) {
			switch (string::const_hash(trait)) {
			case "S"_h:
				teachesSpell = true;
				break;
			case "-S"_h:
				teachesSpell = false;
				break;
			case "AV"_h:
				teachesSkill = true;
				break;
			case "-AV"_h:
				teachesSkill = false;
				break;
			default:
				if (string::is_only_digit(trait)) {
					actorValue = string::to_num<RE::ActorValue>(trait);
				}
				break;
			}
		}
	}
	~BookTraits() override = default;

	bool PassFilter(RE::TESForm* a_item) const override
	{
		const auto book = a_item->As<RE::TESObjectBOOK>();

		if (teachesSpell && book->TeachesSpell() != *teachesSpell) {
			return false;
		}
		if (teachesSkill && book->TeachesSkill() != *teachesSkill) {
			return false;
		}
		if (actorValue != RE::ActorValue::kNone) {
			const auto taughtSpell = book->GetSpell();
			if (book->GetSkill() != actorValue && (taughtSpell && AV::GetAssociatedSkill(taughtSpell) != actorValue)) {
				return false;
			}
		}
		return true;
	}

private:
	// members
	std::optional<bool> teachesSpell;
	std::optional<bool> teachesSkill;
	RE::ActorValue      actorValue{ RE::ActorValue::kNone };
};

class SoulGemTraits : public Traits
{
public:
	SoulGemTraits(const std::string& a_traits)
	{
		auto traits = distribution::split_entry(a_traits);
		for (const auto& trait : traits) {
			if (trait == "BLACK") {
				black = true;
			} else if (trait == "-BLACK") {
				black = false;
			} else if (trait.contains("SOUL(")) {
				soulSize = extract_single_value<RE::SOUL_LEVEL>(trait);
			} else {  // GEM
				gemSize = extract_single_value<RE::SOUL_LEVEL>(trait);
			}
		}
	}
	~SoulGemTraits() override = default;

	bool PassFilter(RE::TESForm* a_item) const override
	{
		const auto soulGem = a_item->As<RE::TESSoulGem>();

		if (black && soulGem->CanHoldNPCSoul() != *black) {
			return false;
		}
		if (soulSize != static_cast<RE::SOUL_LEVEL>(-1) && soulGem->GetContainedSoul() != soulSize) {
			return false;
		}
		if (gemSize != static_cast<RE::SOUL_LEVEL>(-1) && soulGem->GetMaximumCapacity() != gemSize) {
			return false;
		}

		return true;
	}

private:
	// members
	std::optional<bool> black;
	RE::SOUL_LEVEL      soulSize{ static_cast<RE::SOUL_LEVEL>(-1) };
	RE::SOUL_LEVEL      gemSize{ static_cast<RE::SOUL_LEVEL>(-1) };
};

class SpellTraits : public Traits
{
public:
	SpellTraits(const std::string& a_traits)
	{
		auto traits = distribution::split_entry(a_traits);
		for (const auto& trait : traits) {
			if (trait == "H") {
				isHostile = true;
			} else if (trait == "-H") {
				isHostile = false;
			} else if (string::is_only_digit(trait)) {
				skill = extract_single_value<RE::ActorValue>(trait);
			} else if (trait.contains("ST(")) {
				spellType = extract_single_value<RE::MagicSystem::SpellType>(trait);
			} else if (trait.contains("D(")) {
				deliveryType = extract_single_value<RE::MagicSystem::Delivery>(trait);
			} else if (trait.contains("CT(")) {
				castingType = extract_single_value<RE::MagicSystem::CastingType>(trait);
			}
		}
	}
	~SpellTraits() override = default;

	bool PassFilter(RE::TESForm* a_item) const override
	{
		const auto spell = a_item->As<RE::MagicItem>();

		if (spellType != static_cast<RE::MagicSystem::SpellType>(-1) && spell->GetSpellType() != spellType) {
			return false;
		}
		if (castingType != static_cast<RE::MagicSystem::CastingType>(-1) && spell->GetCastingType() != castingType) {
			return false;
		}
		if (deliveryType != static_cast<RE::MagicSystem::Delivery>(-1) && spell->GetDelivery() != deliveryType) {
			return false;
		}
		if (isHostile && spell->IsHostile() != *isHostile) {
			return false;
		}
		if (skill != RE::ActorValue::kNone && AV::GetAssociatedSkill(spell) != skill) {
			return false;
		}

		return true;
	}

private:
	// members
	RE::MagicSystem::SpellType   spellType{ static_cast<RE::MagicSystem::SpellType>(-1) };
	RE::MagicSystem::CastingType castingType{ static_cast<RE::MagicSystem::CastingType>(-1) };
	RE::MagicSystem::Delivery    deliveryType{ static_cast<RE::MagicSystem::Delivery>(-1) };
	RE::ActorValue               skill{ RE::ActorValue::kNone };
	std::optional<bool>          isHostile{};
};

class FurnitureTraits : public Traits
{
public:
	FurnitureTraits(const std::string& a_traits)
	{
		auto traits = distribution::split_entry(a_traits);
		for (const auto& trait : traits) {
			if (trait.contains("BT(")) {
				benchType = extract_single_value<RE::TESFurniture::WorkBenchData::BenchType>(trait);
			} else if (trait.contains("US(")) {
				useSkill = extract_single_value<RE::ActorValue>(trait);
			} else if (trait.contains("T(")) {  // generic — must stay after BT(
				furnitureType = extract_single_value<std::int32_t>(trait);
			}
		}
	}
	~FurnitureTraits() override = default;

	bool PassFilter(RE::TESForm* a_item) const override
	{
		const auto furniture = a_item->As<RE::TESFurniture>();

		if (benchType != static_cast<RE::TESFurniture::WorkBenchData::BenchType>(-1) && furniture->workBenchData.benchType != benchType) {
			return false;
		}
		if (useSkill != RE::ActorValue::kNone && furniture->workBenchData.usesSkill != useSkill) {
			return false;
		}
		if (furnitureType != static_cast<std::int32_t>(-1) && GetFurnitureType(furniture) != furnitureType) {
			return false;
		}

		return true;
	}

private:
	static std::int32_t GetFurnitureType(const RE::TESFurniture* a_furniture)
	{
		using FLAGS = RE::TESFurniture::ActiveMarker;

		const auto flags = a_furniture->furnFlags;
		if (flags.any(FLAGS::kIsPerch)) {
			return 0;
		}
		if (flags.any(FLAGS::kCanLean)) {
			return 1;
		}
		if (flags.any(FLAGS::kCanSit)) {
			return 2;
		}
		if (flags.any(FLAGS::kCanSleep)) {
			return 3;
		}
		return -1;
	}

	// members
	std::int32_t                               furnitureType{ static_cast<std::int32_t>(-1) };
	RE::TESFurniture::WorkBenchData::BenchType benchType{ static_cast<RE::TESFurniture::WorkBenchData::BenchType>(-1) };
	RE::ActorValue                             useSkill{ RE::ActorValue::kNone };
};

inline std::shared_ptr<Traits> Traits::Create(DISTRIBUTION::TYPE a_type, const std::string& a_traits)
{
	switch (a_type) {
	case DISTRIBUTION::TYPE::kArmor:
		return std::make_shared<ArmorTraits>(a_traits);
	case DISTRIBUTION::TYPE::kWeapon:
		return std::make_shared<WeaponTraits>(a_traits);
	case DISTRIBUTION::TYPE::kAmmo:
		return std::make_shared<AmmoTraits>(a_traits);
	case DISTRIBUTION::TYPE::kMagicEffect:
		return std::make_shared<MagicEffectTraits>(a_traits);
	case DISTRIBUTION::TYPE::kPotion:
		return std::make_shared<PotionTraits>(a_traits);
	case DISTRIBUTION::TYPE::kIngredient:
		return std::make_shared<IngredientTraits>(a_traits);
	case DISTRIBUTION::TYPE::kBook:
		return std::make_shared<BookTraits>(a_traits);
	case DISTRIBUTION::TYPE::kSoulGem:
		return std::make_shared<SoulGemTraits>(a_traits);
	case DISTRIBUTION::TYPE::kSpell:
	case DISTRIBUTION::TYPE::kEnchantmentItem:
	case DISTRIBUTION::TYPE::kScroll:
		return std::make_shared<SpellTraits>(a_traits);
	case DISTRIBUTION::TYPE::kFurniture:
		return std::make_shared<FurnitureTraits>(a_traits);
	default:
		return nullptr;
	}
}
