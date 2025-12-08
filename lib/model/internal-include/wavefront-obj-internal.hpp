#pragma once

#include <charconv>
#include <expected>
#include <glm/glm.hpp>
#include <optional>
#include <ranges>
#include <string_view>
#include <system_error>
#include <variant>
#include <vector>

#include "model/wavefront-obj.hpp"

namespace model_parser::wavefront_obj::internal
{
	struct Position_line
	{
		glm::vec3 pos;
	};

	struct Uv_line
	{
		glm::vec2 uv;
	};

	struct Normal_line
	{
		glm::vec3 normal;
	};

	struct Face_line
	{
		struct Index
		{
			uint32_t pos_index, uv_index, normal_index;

			std::strong_ordering operator<=>(const Index& other) const noexcept;
		};

		Index v1, v2, v3;
	};

	using Parsed_line = std::variant<std::monostate, Position_line, Uv_line, Normal_line, Face_line>;

	template <typename T>
	std::optional<T> to_number(const std::string_view& str) noexcept
	{
		T value;

		const auto result = std::from_chars(std::to_address(str.begin()), std::to_address(str.end()), value);

		if (result.ptr != std::to_address(str.end())
			|| result.ec == std::errc::result_out_of_range
			|| result.ec == std::errc::invalid_argument)
			return std::nullopt;

		return value;
	}

	// Extract lines of a specific type from parsed lines
	template <typename T>
	auto extract_lines(const std::vector<Parsed_line>& lines) noexcept
	{
		return lines
			| std::views::filter([](const auto& line) {
				   return std::holds_alternative<T>(line);
			   })
			| std::views::transform([](const auto& line) {
				   return std::get<T>(line);
			   });
	}

	std::optional<Face_line::Index> parse_face_index(const std::string_view& slice) noexcept;

	///
	/// @brief Parse one line
	///
	/// @param slice Line content
	/// @retval std::monostate Empty line or comment, should be ignored
	/// @retval util::Error Error information on failure
	/// @return Parsed line
	///
	std::expected<Parsed_line, util::Error> parse_line(const std::string_view& slice) noexcept;

	// Parse all lines
	std::expected<std::vector<Parsed_line>, util::Error> parse_tokenize(
		const std::string_view& content
	) noexcept;

	// Build Object from parsed lines
	std::expected<Object, util::Error> build_object(const std::vector<Parsed_line>& lines) noexcept;

	std::expected<Parsed_line, util::Error> parse_pos(
		const std::vector<std::string_view>& parameters
	) noexcept;

	std::expected<Parsed_line, util::Error> parse_uv(
		const std::vector<std::string_view>& parameters
	) noexcept;

	std::expected<Parsed_line, util::Error> parse_normal(
		const std::vector<std::string_view>& parameters
	) noexcept;

	std::expected<Parsed_line, util::Error> parse_face(
		const std::vector<std::string_view>& parameters
	) noexcept;

}