#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <functional>
#include <ranges>

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#include <ClibUtil/distribution.hpp>
#include <ClibUtil/rng.hpp>
#include <ClibUtil/simpleINI.hpp>
#include <ClibUtil/numeric.hpp>
#include <MergeMapperPluginAPI.h>
#include <ankerl/unordered_dense.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <srell.hpp>

#include "LogBuffer.h"

#define DLLEXPORT __declspec(dllexport)

namespace logger = SKSE::log;
namespace buffered_logger = LogBuffer;

using namespace clib_util;
using namespace std::literals;
using namespace string::literals;

template <class T>
using nullable = std::optional<T>;

template <class K, class D>
using Map = ankerl::unordered_dense::segmented_map<K, D>;
template <class K>
using Set = ankerl::unordered_dense::segmented_set<K>;

struct string_hash
{
	using is_transparent = void;  // enable heterogeneous overloads
	using is_avalanching = void;  // mark class as high quality avalanching hash

	[[nodiscard]] std::uint64_t operator()(std::string_view str) const noexcept
	{
		return ankerl::unordered_dense::hash<std::string_view>{}(str);
	}
};

template <class D>
using StringMap = ankerl::unordered_dense::map<std::string_view, D, string_hash, std::equal_to<>>;
using StringSet = ankerl::unordered_dense::set<std::string_view, string_hash, std::equal_to<>>;

namespace stl
{
	using namespace SKSE::stl;
}

#include "Cache.h"
#include "Defs.h"
#include "Version.h"
