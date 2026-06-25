#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <functional>
#include <ranges>
#include <unordered_set>

#include "RE/Skyrim.h"
#include "REX/REX/Singleton.h"
#include "SKSE/SKSE.h"

#include <MergeMapperPluginAPI.h>
#include <boost/unordered/unordered_flat_map.hpp>
#include <boost/unordered/unordered_flat_set.hpp>
#include <boost/unordered/concurrent_flat_map.hpp>
#include <frozen/bits/elsa_std.h>
#include <frozen/unordered_map.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <srell.hpp>

#include <ClibUtil/distribution.hpp>
#include <ClibUtil/numeric.hpp>
#include <ClibUtil/rng.hpp>
#include <ClibUtil/simpleINI.hpp>
#include <ClibUtil/timer.hpp>

#include <ClibUtil/editorID.hpp>

#include "LogBuffer.h"

#define DLLEXPORT __declspec(dllexport)

namespace logger = SKSE::log;
namespace buffered_logger = LogBuffer;
namespace distribution = clib_util::distribution;
namespace string = clib_util::string;
namespace hash = clib_util::hash;
namespace EDID = clib_util::editorID;

using namespace std::literals;
using namespace string::literals;

// for visting variants
template <class... Ts>
struct overload : Ts...
{
	using Ts::operator()...;
};

template <class K, class D, class H = boost::hash<K>, class KEqual = std::equal_to<K>>
using Map = boost::unordered_flat_map<K, D, H, KEqual>;

template <class K, class H = boost::hash<K>, class KEqual = std::equal_to<K>>
using Set = boost::unordered_flat_set<K, H, KEqual>;

template <class K, class D, class H = boost::hash<K>, class KEqual = std::equal_to<K>>
using ConcurrentMap = boost::concurrent_flat_map<K, D, H, KEqual>;

struct string_hash
{
	using is_transparent = void;  // enable heterogeneous overloads

	std::size_t operator()(const std::string& str) const
	{
		return boost::hash<std::string>()(str);
	}

	std::size_t operator()(std::string_view str) const
	{
		return boost::hash<std::string_view>()(str);
	}
};

struct istring_hash
{
	using is_transparent = void;  // enable heterogeneous overloads

	std::size_t operator()(std::string_view str) const
	{
		std::size_t seed = 0;
		for (auto it = str.begin(); it != str.end(); ++it) {
			boost::hash_combine(seed, std::tolower(*it));
		}
		return seed;
	}
};

struct istring_cmp
{
	using is_transparent = void;  // enable heterogeneous overloads

	bool operator()(const std::string& str1, const std::string& str2) const
	{
		return string::iequals(str1, str2);
	}
	bool operator()(std::string_view str1, std::string_view str2) const
	{
		return string::iequals(str1, str2);
	}
};

template <class D>
using StringMap = Map<std::string, D, string_hash>;
using StringSet = Set<std::string, string_hash>;
template <class D>
using IStringMap = Map<std::string, D, istring_hash, istring_cmp>;
using IStringSet = Set<std::string, istring_hash, istring_cmp>;

namespace stl
{
	using namespace SKSE::stl;

	template <class F, class T>
	void write_vfunc()
	{
		REL::Relocation<std::uintptr_t> vtbl{ F::VTABLE[0] };
		T::func = vtbl.write_vfunc(T::idx, T::thunk);
	}
}

/// A standardized way of converting any object to string.
///
///	<p>
///	Overload `operator<<` to provide custom formatting for your value.
///	Alternatively, specialize this method and provide your own implementation.
///	</p>
template <typename Value>
std::string describe(Value value)
{
	std::ostringstream os;
	os << value;
	return os.str();
}

inline std::ostream& operator<<(std::ostream& os, const RE::TESFile* file)
{
	os << file->fileName;
	return os;
}

inline std::ostream& operator<<(std::ostream& os, const RE::TESForm* form)
{
	if (const auto& edid = EDID::get_editorID(form); !edid.empty()) {
		os << edid << " ";
	}
	os << "["
	   << std::to_string(form->GetFormType())
	   << ":"
	   << std::setfill('0')
	   << std::setw(sizeof(RE::FormID) * 2)
	   << std::uppercase
	   << std::hex
	   << form->GetFormID()
	   << "]";

	return os;
}

#include "Cache.h"
#include "Defs.h"
#include "RE.h"
#include "Version.h"
