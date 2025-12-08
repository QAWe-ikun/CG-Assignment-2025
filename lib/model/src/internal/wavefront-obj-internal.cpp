#include "wavefront-obj-internal.hpp"

#include <algorithm>
#include <format>
#include <map>
#include <memory_resource>

namespace model_parser::wavefront_obj::internal
{
	inline namespace
	{
		constexpr auto subrange_to_string_view = [](auto subrange) {
			return std::string_view(subrange.begin(), subrange.end());
		};
	}  // namespace

	std::strong_ordering Face_line::Index::operator<=>(const Index& other) const noexcept
	{
		return std::tie(pos_index, uv_index, normal_index)
			<=> std::tie(other.pos_index, other.uv_index, other.normal_index);
	}

	std::optional<Face_line::Index> parse_face_index(const std::string_view& slice) noexcept
	{
		std::array<std::string_view, 16> buffer;
		std::pmr::monotonic_buffer_resource pool(buffer.data(), buffer.size());

		const auto parameters = slice
			| std::views::split('/')
			| std::views::transform(subrange_to_string_view)
			| std::ranges::to<std::pmr::vector<std::string_view>>(&pool);

		if (parameters.empty() || parameters.size() != 3) return std::nullopt;

		const auto pos_index = to_number<uint32_t>(parameters[0]);
		const auto uv_index = to_number<uint32_t>(parameters[1]);
		const auto normal_index = to_number<uint32_t>(parameters[2]);

		if (!pos_index || !uv_index || !normal_index) return std::nullopt;
		if (*pos_index == 0 || *uv_index == 0 || *normal_index == 0) return std::nullopt;

		return Face_line::Index{
			.pos_index = *pos_index - 1,
			.uv_index = *uv_index - 1,
			.normal_index = *normal_index - 1
		};
	}

	std::expected<Parsed_line, util::Error> parse_pos(
		const std::vector<std::string_view>& parameters
	) noexcept
	{
		if (parameters.size() != 4) return util::Error("Invalid arguments");

		const auto x = to_number<float>(parameters[1]);
		const auto y = to_number<float>(parameters[2]);
		const auto z = to_number<float>(parameters[3]);

		if (!x || !y || !z) return util::Error("Invalid coordinates");

		return Position_line{
			.pos = {*x, *y, *z}
		};
	}

	std::expected<Parsed_line, util::Error> parse_uv(const std::vector<std::string_view>& parameters) noexcept
	{
		if (parameters.size() != 3) return util::Error("Invalid arguments");

		const auto u = to_number<float>(parameters[1]);
		const auto v = to_number<float>(parameters[2]);

		if (!u || !v) return util::Error("Invalid coordinates");

		return Uv_line{
			.uv = {*u, *v}
		};
	}

	std::expected<Parsed_line, util::Error> parse_normal(
		const std::vector<std::string_view>& parameters
	) noexcept
	{
		if (parameters.size() != 4) return util::Error("Invalid arguments");

		const auto x = to_number<float>(parameters[1]);
		const auto y = to_number<float>(parameters[2]);
		const auto z = to_number<float>(parameters[3]);

		if (!x && !y && !z) return util::Error("Invalid coordinates");

		return Normal_line{
			.normal = {*x, *y, *z}
		};
	}

	std::expected<Parsed_line, util::Error> parse_face(
		const std::vector<std::string_view>& parameters
	) noexcept
	{
		if (parameters.size() != 4) util::Error("Invalid arguments");

		const auto v1 = parse_face_index(parameters[1]);
		const auto v2 = parse_face_index(parameters[2]);
		const auto v3 = parse_face_index(parameters[3]);

		if (!v1 || !v2 || !v3) return util::Error("Invalid vertex index");

		return Face_line{.v1 = *v1, .v2 = *v2, .v3 = *v3};
	}

	std::expected<Parsed_line, util::Error> parse_line(const std::string_view& slice) noexcept
	{
		/* Slice */

		if (slice.empty()) return std::monostate();
		if (slice.length() > 4096) return util::Error("Line input too long, should not exceed 4096");

		const auto parameters =
			slice
			| std::views::split(' ')
			| std::views::filter([](auto subrange) {
				  return !subrange.empty();
			  })
			| std::views::transform(subrange_to_string_view)
			| std::ranges::to<std::vector<std::string_view>>();

		if (parameters.empty()) return std::monostate();

		/* Parse */

		using Parse_func = std::expected<Parsed_line, util::Error>(const std::vector<std::string_view>&);

		const std::map<std::string, Parse_func*> parsers = {
			{"v",  parse_pos   },
			{"vt", parse_uv    },
			{"vn", parse_normal},
			{"f",  parse_face  }
		};

		if (const auto it = parsers.find(std::string(parameters[0])); it != parsers.end())
			return it->second(parameters);
		else
			return std::monostate();
	}

	std::expected<std::vector<Parsed_line>, util::Error> parse_tokenize(
		const std::string_view& content
	) noexcept
	{
		std::vector<Parsed_line> result;

		constexpr auto filter_cr = [](char c) {
			return c != '\r';
		};

		constexpr auto subrange_to_string = [](auto subrange) {
			return subrange | std::ranges::to<std::string>();
		};

		constexpr auto subrange_not_empty = [](std::string_view string) {
			return !string.empty() && !std::ranges::all_of(string, [](char c) {
				return isspace(c);
			});
		};

		auto lines = content
			| std::views::filter(filter_cr)
			| std::views::lazy_split('\n')
			| std::views::transform(subrange_to_string)
			| std::views::filter(subrange_not_empty);

		for (const auto [i, line] : lines | std::views::enumerate)
		{
			const auto parsed_line = parse_line(line);
			if (!parsed_line) return parsed_line.error().propagate(std::format("解析第 {} 行失败", i + 1));
			if (!std::holds_alternative<std::monostate>(*parsed_line)) result.push_back(*parsed_line);
		}

		return result;
	}

	std::expected<Object, util::Error> build_object(const std::vector<Parsed_line>& lines) noexcept
	{
		/* 提取 */

		const auto positions = extract_lines<Position_line>(lines) | std::ranges::to<std::vector>();
		const auto uvs = extract_lines<Uv_line>(lines) | std::ranges::to<std::vector>();
		const auto normals = extract_lines<Normal_line>(lines) | std::ranges::to<std::vector>();

		const auto faces =
			extract_lines<Face_line>(lines)
			| std::views::transform([](const Face_line& face) {
				  return std::array<Face_line::Index, 3>{face.v1, face.v2, face.v3};
			  })
			| std::views::join
			| std::ranges::to<std::vector>();

		/* Remove duplicated vertices */

		auto vertice_map =
			faces
			| std::views::transform([](const auto& index_tuple) {
				  return std::pair(index_tuple, uint32_t(0));
			  })
			| std::ranges::to<std::map>();

		std::vector<Vertex> unique_vertices;

		for (auto [i, item] : vertice_map | std::views::enumerate)
		{
			const auto [pos_idx, uv_idx, normal_idx] = item.first;

			if (pos_idx >= positions.size() || uv_idx >= uvs.size() || normal_idx >= normals.size())
				return util::Error("顶点索引超出范围");

			unique_vertices.emplace_back(
				Vertex{
					.pos = positions[pos_idx].pos,
					.normal = normals[normal_idx].normal,
					.uv = uvs[uv_idx].uv
				}
			);
			item.second = i;
		}

		/* Build index */

		auto indices =
			faces
			| std::views::transform([&vertice_map](const auto& index) {
				  return vertice_map.at(index);
			  })
			| std::ranges::to<std::vector>();

		return Object{.vertices = std::move(unique_vertices), .indices = std::move(indices)};
	}
}