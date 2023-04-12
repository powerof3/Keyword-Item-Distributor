#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <ranges>

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#include <ClibUtil/distribution.hpp>
#include <ClibUtil/rng.hpp>
#include <ClibUtil/simpleINI.hpp>
#include <MergeMapperPluginAPI.h>
#include <ankerl/unordered_dense.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <srell.hpp>

#include "LogBuffer.h"

#define DLLEXPORT __declspec(dllexport)

namespace logger = SKSE::log;
namespace buffered_logger = LogBuffer;
namespace string = clib_util::string;
namespace distribution = clib_util::distribution;

using namespace std::literals;
using namespace string::literals;

using RNG = clib_util::RNG;

template <class T>
using nullable = std::optional<T>;

namespace stl
{
	using namespace SKSE::stl;
}

#include "Cache.h"
#include "Defs.h"
#include "Version.h"
