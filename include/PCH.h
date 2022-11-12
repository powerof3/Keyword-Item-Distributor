#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#include <ClibUtil/utils.hpp>
#include <MergeMapperPluginAPI.h>
#include <SimpleIni.h>
#include <frozen/map.h>
#include <frozen/set.h>
#include <ranges>
#include <spdlog/sinks/basic_file_sink.h>
#include <srell.hpp>

#define DLLEXPORT __declspec(dllexport)

namespace logger = SKSE::log;
namespace numeric = clib_util::numeric;
namespace string = clib_util::string;
namespace distribution = clib_util::distribution;

using namespace std::literals;

using RNG = clib_util::RNG;

namespace stl
{
	using namespace SKSE::stl;
}

#include "Defs.h"
#include "Version.h"
