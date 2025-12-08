#pragma once

#include <expected>
#include <iostream>
#include <source_location>
#include <string>
#include <tuple>
#include <vector>

namespace util
{
	///
	/// @brief Error class containing an error trace
	///
	///
	class Error
	{
	  public:

		///
		/// @brief Error trace entry
		///
		///
		struct Trace_entry
		{
			std::source_location location;  // Source location of the error
			std::string message;            // Information about the error
		};

	  private:

		std::vector<Trace_entry> entries;

	  public:

		Error(const Error&) = default;
		Error(Error&&) = default;
		Error& operator=(const Error&) = default;
		Error& operator=(Error&&) = default;

		///
		/// @brief Constructs and error
		///
		/// @param message Error information
		/// @param location Location where the error occurred, default to current location
		///
		Error(
			std::string message,
			const std::source_location& location = std::source_location::current()
		) noexcept;

		///
		/// @brief Propagate the error by adding a new trace entry
		///
		/// @param message Additional error information
		/// @param location Location where the error is propagated, default to current location
		/// @return
		///
		Error propagate(
			std::string message,
			const std::source_location& location = std::source_location::current()
		) const noexcept;

		template <typename V>
		operator std::expected<V, Error>() const noexcept
		{
			return std::unexpected<Error>(*this);
		}

		///
		/// @brief Dumps the error trace to an output stream
		///
		/// @param os Output stream, default to std::cerr
		/// @param color Whether to use colored output, default to true
		///
		void dump_trace(std::ostream& os = std::cerr, bool color = true) const noexcept;

		const std::vector<Trace_entry>& operator*() const noexcept { return entries; }
		const std::vector<Trace_entry>* operator->() const noexcept { return &(**this); }
	};

	struct Unwrap
	{
		std::source_location location;
		std::string message;
	};

	///
	/// @brief Error unwrapper
	/// @details Usage: `value = expected_value_or_error | util::unwrap("Error message")`
	///
	/// @param message Message for unwrapping error
	/// @param location Source location, defaults to current location
	/// @return Unwrapper object
	///
	Unwrap unwrap(
		const std::string& message = "",
		const std::source_location& location = std::source_location::current()
	) noexcept;

	template <typename T>
		requires(!std::is_same_v<T, void>)
	T operator|(std::expected<T, Error> expected, const Unwrap& unwrap)
	{
		if (!expected) throw expected.error().propagate(unwrap.message, unwrap.location);
		return std::move(expected.value());
	}

	void operator|(std::expected<void, Error> expected, const Unwrap& unwrap);

	///
	/// @brief Pack multiple expected results into a tuple
	/// @return Packed tuple of results, or the first error encountered
	///
	template <typename E, typename V>
	std::expected<std::tuple<V>, E> pack_results(std::expected<V, E> result)
	{
		return std::move(result).transform([](V&& value) {
			return std::make_tuple(std::move(value));
		});
	}

	///
	/// @brief Pack multiple expected results into a tuple
	/// @return Packed tuple of results, or the first error encountered
	///
	template <typename E, typename Vf, typename... V>
	std::expected<std::tuple<Vf, V...>, E> pack_results(
		std::expected<Vf, E> result_first,
		std::expected<V, E>... results
	)
	{
		if (!result_first.has_value()) return std::unexpected(std::move(result_first.error()));

		auto results_expanded = pack_results(std::move(results)...);
		if (!results_expanded.has_value()) return std::unexpected(std::move(results_expanded.error()));

		return std::tuple_cat(
			std::make_tuple(std::move(result_first.value())),
			std::move(results_expanded.value())
		);
	}
}