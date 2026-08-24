#pragma once

#define MAKE_BUFFERED_LOG(a_func, a_level)                                    \
                                                                              \
	template <class... Args>                                                  \
	struct [[maybe_unused]] a_func                                            \
	{                                                                         \
		a_func() = delete;                                                    \
                                                                              \
		explicit a_func(                                                      \
			std::format_string<Args...> a_fmt,                                \
			Args&&... a_args,                                                 \
			std::source_location a_loc = std::source_location::current())     \
		{                                                                     \
			auto message = std::format(a_fmt, std::forward<Args>(a_args)...); \
			if (const auto [it, inserted] =                                   \
					buffer.insert(Entry{ a_loc, std::move(message) });        \
				inserted) {                                                   \
				REX::Impl::Log(a_loc, REX::ELogLevel::a_level,                \
					std::string_view{ it->message });                         \
			}                                                                 \
		}                                                                     \
	};                                                                        \
                                                                              \
	template <class... Args>                                                  \
	a_func(std::format_string<Args...>, Args&&...) -> a_func<Args...>;

namespace LogBuffer
{
	struct Entry
	{
		std::source_location loc;
		std::string          message;

		bool operator==(const Entry& other) const
		{
			return std::strcmp(loc.file_name(), other.loc.file_name()) == 0 &&
			       loc.line() == other.loc.line() &&
			       message == other.message;
		}
	};
}

/// Add hashing for custom log entries.
template <>
struct boost::hash<LogBuffer::Entry>
{
	using is_transparent = void;

	[[nodiscard]] std::uint64_t operator()(const LogBuffer::Entry& entry) const noexcept
	{
		return boost::hash<std::string>()(entry.message);
	}
};

/// LogBuffer proxies typical logging calls and buffers received entries to avoid duplication.
namespace LogBuffer
{
	inline boost::unordered_flat_set<Entry> buffer{};

	/// Clears already buffered messages to allow them to be logged once again.
	inline void clear()
	{
		buffer.clear();
	}

	MAKE_BUFFERED_LOG(trace, Trace);
	MAKE_BUFFERED_LOG(debug, Debug);
	MAKE_BUFFERED_LOG(info, Info);
	MAKE_BUFFERED_LOG(warn, Warning);
	MAKE_BUFFERED_LOG(error, Error);
	MAKE_BUFFERED_LOG(critical, Critical);
}

#undef MAKE_BUFFERED_LOG
