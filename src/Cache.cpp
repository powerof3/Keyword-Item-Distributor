#include "Cache.h"

namespace Cache
{
	bool FormType::IsFilter(RE::FormType a_type)
	{
		return std::ranges::find(set, a_type) != set.end();
	}

	Item::TYPE Item::GetType(const std::string& a_type)
	{
		const auto it = std::ranges::find_if(itemTypes,
			[&](const auto& element) { return element.first == a_type; });
		return it != itemTypes.end() ?
		           static_cast<TYPE>(it - std::begin(itemTypes)) :
		           kNone;
	}

	std::string_view Item::GetType(TYPE a_type)
	{
		return itemTypes[a_type].first;
	}

	std::string_view ActorValue::GetActorValue(RE::ActorValue a_av)
	{
		const auto it = r_map.find(a_av);
		return it != r_map.end() ? it->second : "None";
	}

	RE::ActorValue ActorValue::GetActorValue(std::string_view a_av)
	{
		const auto it = map.find(a_av);
		return it != map.end() ? it->second : RE::ActorValue::kNone;
	}
}
