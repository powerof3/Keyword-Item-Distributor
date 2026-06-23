#pragma once

template <class T>
concept numeric = std::is_arithmetic_v<T>;

template <class T>
	requires numeric<T>
struct Range
{
	Range() = default;

	Range(T a_min) :
		min(a_min)
	{}
	Range(T a_min, T a_max) :
		min(a_min),
		max(a_max)
	{}
	Range(const std::string& a_str)
	{
		static const srell::regex re{
			R"([^-+.\d]*([-+]?\d*\.?\d+)(?:[^-+.\d]+([-+]?\d*\.?\d+))?.*)" // (min/max)
		};
		
		if (srell::smatch m; srell::regex_match(a_str, m, re)) {
			min = string::to_num<T>(m[1].str());
			max = m[2].matched ? string::to_num<T>(m[2].str()) : min;
		}
	}

	auto operator<=>(const Range&) const = default;

	[[nodiscard]] bool IsValid() const { return min > std::numeric_limits<T>::lowest(); }  // min must always be valid, max is optional
	[[nodiscard]] bool IsInRange(T value) const { return value >= min && value <= max; }
	[[nodiscard]] bool IsExact() const { return min == max; }

	[[nodiscard]] T GetRandom(bool a_fixed, RE::FormID a_seed1, RE::FormID a_seed2) const
	{
		using namespace clib_util;

		return IsExact() ? min :
		       a_fixed   ? RNG(hash::szudzik_pair(a_seed1, a_seed2)).generate<T>(min, max) :
		                   RNG().generate<T>(min, max);
	}

	// members
	T min{ std::numeric_limits<T>::lowest() };
	T max{ std::numeric_limits<T>::max() };
};
