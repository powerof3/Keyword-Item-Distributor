#pragma once

namespace TRAITS
{
	namespace detail
	{
		template <class T>
		nullable<T> get_single_value(std::string& a_str)
		{
			if (const auto values = string::split(string::remove_non_numeric(a_str), " "); !values.empty()) {
				return string::to_num<T>(values[0]);
			}
			return std::nullopt;
		}
	}

	template <typename T>
	struct Range
	{
		Range(T a_min) :
			min(a_min)
		{}
		Range(T a_min, T a_max) :
			min(a_min),
			max(a_max)
		{}
		Range(std::string& a_str)
		{
			if (const auto values = string::split(string::remove_non_numeric(a_str), " "); !values.empty()) {
				if (values.size() > 1) {
					min = string::to_num<T>(values[0]);
					max = string::to_num<T>(values[1]);
				} else {
					min = string::to_num<T>(values[0]);
				}
			}
		}

		[[nodiscard]] bool IsInRange(T value) const
		{
			return value >= min && value <= max;
		}

		// members
		T min{ std::numeric_limits<T>::min() };
		T max{ std::numeric_limits<T>::max() };
	};

	class Traits
	{
	public:
		virtual ~Traits() = default;

		[[nodiscard]] virtual bool PassFilter([[maybe_unused]] RE::TESForm* a_item) const
		{
			return false;
		}
	};

	class ArmorTraits : public Traits
	{
	public:
		ArmorTraits(const std::string& a_traits)
		{
			auto traits = distribution::split_entry(a_traits);
			for (auto& trait : traits) {
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
			if (armorRating && !armorRating->IsInRange(armor->GetArmorRating())) {
				return false;
			}
			if (armorType && armor->GetArmorType() != *armorType) {
				return false;
			}
			if (weight && !weight->IsInRange(armor->weight)) {
				return false;
			}
			if (slot && !armor->HasPartOf(*slot)) {
				return false;
			}
			return true;
		}

	private:
		// members
		nullable<bool>                             enchanted{};
		nullable<bool>                             templated{};
		nullable<Range<float>>                     armorRating{};
		nullable<RE::BIPED_MODEL::ArmorType>       armorType{};
		nullable<Range<float>>                     weight;
		nullable<RE::BIPED_MODEL::BipedObjectSlot> slot{};
	};

	class WeaponTraits : public Traits
	{
	public:
		WeaponTraits(const std::string& a_traits)
		{
			auto traits = distribution::split_entry(a_traits);
			for (auto& trait : traits) {
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
			if (weight && !weight->IsInRange(weapon->weight)) {
				return false;
			}
			if (animationType && weapon->GetWeaponType() != *animationType) {
				return false;
			}
			if (damage && !damage->IsInRange(weapon->GetAttackDamage())) {
				return false;
			}
			return true;
		}

	private:
		// members
		nullable<bool>            enchanted{};
		nullable<bool>            templated{};
		nullable<RE::WEAPON_TYPE> animationType{};
		nullable<Range<float>>    damage{};
		nullable<Range<float>>    weight{};
	};

	class AmmoTraits : public Traits
	{
	public:
		AmmoTraits(const std::string& a_traits)
		{
			auto traits = distribution::split_entry(a_traits);
			for (auto& trait : traits) {
				if (trait.contains("D(")) {
					damage = Range<float>(trait);
				} else {
					switch (string::const_hash(a_traits)) {
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
			if (damage && !damage->IsInRange(ammo->data.damage)) {
				return false;
			}
			return true;
		}

	private:
		// members
		nullable<bool>         isBolt;
		nullable<Range<float>> damage;
	};

	class MagicEffectTraits : public Traits
	{
	public:
		MagicEffectTraits(const std::string& a_traits)
		{
			auto traits = distribution::split_entry(a_traits);
			for (auto& trait : traits) {
				if (trait.contains("D(")) {
					deliveryType = detail::get_single_value<RE::MagicSystem::Delivery>(trait);
				} else if (trait.contains("CT(")) {
					castingType = detail::get_single_value<RE::MagicSystem::CastingType>(trait);
				} else if (trait.contains("R(")) {
					resistance = detail::get_single_value<RE::ActorValue>(trait);
				} else if (trait.contains('(')) {
					if (auto value = string::split(string::remove_non_numeric(trait), " "); !value.empty()) {
						auto skillType = string::to_num<RE::ActorValue>(value[0]);
						auto min = string::to_num<std::int32_t>(value[1]);
						if (value.size() > 2) {
							auto max = string::to_num<std::int32_t>(value[2]);
							skill = { skillType, { min, max } };
						} else {
							skill = { skillType, { min } };
						}
					}
				} else if (trait == "H") {
					isHostile = true;
				} else if (trait == "-H") {
					isHostile = false;
				} else if (trait == "DISPEL") {
					dispelWithKeywords = true;
				} else if (trait == "-DISPEL") {
					dispelWithKeywords = false;
				}
			}
		}

		~MagicEffectTraits() override = default;

		bool PassFilter(RE::TESForm* a_item) const override
		{
			const auto mgef = a_item->As<RE::EffectSetting>();

			if (isHostile && mgef->IsHostile() != *isHostile) {
				return false;
			}
			if (castingType && mgef->data.castingType != *castingType) {
				return false;
			}
			if (deliveryType && mgef->data.delivery != *deliveryType) {
				return false;
			}
			if (skill) {
				auto& [skillType, minMax] = *skill;
				if (skillType != mgef->GetMagickSkill()) {
					return false;
				}
				if (!minMax.IsInRange(mgef->GetMinimumSkillLevel())) {
					return false;
				}
			}
			if (resistance && mgef->data.resistVariable != *resistance) {
				return false;
			}
			if (dispelWithKeywords && mgef->data.flags.all(RE::EffectSetting::EffectSettingData::Flag::kDispelWithKeywords) != *dispelWithKeywords) {
				return false;
			}
			return true;
		}

	private:
		// members
		nullable<bool>                                           isHostile;
		nullable<RE::MagicSystem::CastingType>                   castingType;
		nullable<RE::MagicSystem::Delivery>                      deliveryType;
		nullable<std::pair<RE::ActorValue, Range<std::int32_t>>> skill;
		nullable<RE::ActorValue>                                 resistance;
		nullable<bool>                                           dispelWithKeywords;
	};

	class PotionTraits : public Traits
	{
	public:
		PotionTraits(const std::string& a_traits)
		{
			const auto traits = distribution::split_entry(a_traits);
			for (auto& trait : traits) {
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
		nullable<bool> isPoison;
		nullable<bool> isFood;
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
		nullable<bool> isFood;
	};

	class BookTraits : public Traits
	{
	public:
		BookTraits(const std::string& a_traits)
		{
			const auto traits = distribution::split_entry(a_traits);
			for (auto& trait : traits) {
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
					actorValue = string::to_num<RE::ActorValue>(trait);
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
			if (actorValue) {
				const auto taughtSpell = book->GetSpell();
				if (book->GetSkill() != *actorValue && (taughtSpell && taughtSpell->GetAssociatedSkill() != *actorValue)) {
					return false;
				}
			}
			return true;
		}

	private:
		// members
		nullable<bool>           teachesSpell;
		nullable<bool>           teachesSkill;
		nullable<RE::ActorValue> actorValue;
	};

	class SoulGemTraits : public Traits
	{
	public:
		SoulGemTraits(const std::string& a_traits)
		{
			auto traits = distribution::split_entry(a_traits);
			for (auto& trait : traits) {
				if (trait == "BLACK") {
					black = true;
				} else if (trait == "-BLACK") {
					black = false;
				} else if (trait.contains("SOUL(")) {
					soulSize = detail::get_single_value<RE::SOUL_LEVEL>(trait);
				} else {  // GEM
					gemSize = detail::get_single_value<RE::SOUL_LEVEL>(trait);
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
			if (soulSize && soulGem->GetContainedSoul() != *soulSize) {
				return false;
			}
			if (gemSize && soulGem->GetMaximumCapacity() != *gemSize) {
				return false;
			}

			return true;
		}

	private:
		// members
		nullable<bool>           black;
		nullable<RE::SOUL_LEVEL> soulSize;
		nullable<RE::SOUL_LEVEL> gemSize;
	};

	class SpellTraits : public Traits
	{
	public:
		SpellTraits(const std::string& a_traits)
		{
			auto traits = distribution::split_entry(a_traits);
			for (auto& trait : traits) {
				if (trait.contains("ST(")) {
					spellType = detail::get_single_value<RE::MagicSystem::SpellType>(trait);
				} else if (trait.contains("D(")) {
					deliveryType = detail::get_single_value<RE::MagicSystem::Delivery>(trait);
				} else if (trait.contains("CT(")) {
					castingType = detail::get_single_value<RE::MagicSystem::CastingType>(trait);
				} else if (trait == "H") {
					isHostile = true;
				} else if (trait == "-H") {
					isHostile = false;
				} else {
					skill = detail::get_single_value<RE::ActorValue>(trait);
				}
			}
		}

		~SpellTraits() override = default;

		bool PassFilter(RE::TESForm* a_item) const override
		{
			const auto spell = a_item->As<RE::SpellItem>();

			if (spellType && spell->GetSpellType() != *spellType) {
				return false;
			}
			if (castingType && spell->GetCastingType() != *castingType) {
				return false;
			}
			if (deliveryType && spell->GetDelivery() != *deliveryType) {
				return false;
			}
			if (skill && spell->GetAssociatedSkill() != *skill) {
				return false;
			}
			if (isHostile && spell->IsHostile() != *isHostile) {
				return false;
			}
			return true;
		}

	private:
		// members
		nullable<RE::MagicSystem::SpellType>   spellType{};
		nullable<RE::MagicSystem::CastingType> castingType{};
		nullable<RE::MagicSystem::Delivery>    deliveryType{};
		nullable<RE::ActorValue>               skill{};
		nullable<bool>                         isHostile{};
	};

	class FurnitureTraits : public Traits
	{
	public:
		FurnitureTraits(const std::string& a_traits)
		{
			auto traits = distribution::split_entry(a_traits);
			for (auto& trait : traits) {
				if (trait.contains("BT(")) {
					benchType = detail::get_single_value<RE::TESFurniture::WorkBenchData::BenchType>(trait);
				} else if (trait.contains("T(")) {
					furnitureType = detail::get_single_value<std::int32_t>(trait);
				} else if (trait.contains("US(")) {
					useSkill = detail::get_single_value<RE::ActorValue>(trait);
				}
			}
		}

		~FurnitureTraits() override = default;

		bool PassFilter(RE::TESForm* a_item) const override
		{
			const auto furniture = a_item->As<RE::TESFurniture>();

			if (benchType && furniture->workBenchData.benchType != *benchType) {
				return false;
			}

			if (furnitureType && GetFurnitureType(furniture) != *furnitureType) {
				return false;
			}

			if (useSkill && furniture->workBenchData.usesSkill != *useSkill) {
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
		nullable<std::int32_t>                               furnitureType{};
		nullable<RE::TESFurniture::WorkBenchData::BenchType> benchType{};
		nullable<RE::ActorValue>                             useSkill{};
	};
}
using TraitsPtr = std::shared_ptr<TRAITS::Traits>;
