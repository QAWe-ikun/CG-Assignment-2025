#include "graphic/aa/detail/mlaa-smaa-lut.hpp"

#include <array>
#include <ranges>

namespace graphic::aa
{
	namespace
	{
		struct Ortho_silhouette
		{
			double left = 0, right = 0;
		};

		constexpr auto pattern_silhouette_list = std::to_array<Ortho_silhouette>({
			{.left = 0,    .right = 0   },
			{.left = -0.5, .right = 0   },
			{.left = 0,    .right = -0.5},
			{.left = -0.5, .right = -0.5},
			{.left = 0.5,  .right = 0   },
			{.left = 0,    .right = 0   },
			{.left = 0.5,  .right = -0.5},
			{.left = 0.5,  .right = -0.5},
			{.left = 0,    .right = 0.5 },
			{.left = -0.5, .right = 0.5 },
			{.left = 0,    .right = 0   },
			{.left = -0.5, .right = 0.5 },
			{.left = 0.5,  .right = 0.5 },
			{.left = -0.5, .right = 0.5 },
			{.left = 0.5,  .right = -0.5},
			{.left = 0,    .right = 0   }
		});

		std::vector<glm::u8vec2> generate_blend_lut_block(uint8_t pattern, size_t lut_size)
		{
			[[assume(pattern < 16)]];

			const auto& silhouette = pattern_silhouette_list[pattern];

			const auto get_factor_by_ratio = [&silhouette](double ratio) -> double {
				return ratio < 0.5
					? glm::mix(silhouette.left, 0.0, ratio * 2.0)
					: glm::mix(0.0, silhouette.right, (ratio - 0.5) * 2.0);
			};

			const auto compute_area = [&](uint8_t left, uint8_t right) -> glm::vec2 {
				const auto total_edge_length = left + right + 1;

				if (left == right)
				{
					const auto factor = get_factor_by_ratio(double(left) / total_edge_length);
					const auto area = glm::abs(0.25 * factor);

					if (silhouette.left * silhouette.right < 1e-6)
						return {area, area};
					else if (silhouette.left > 0)
						return {area * 2.0f, 0.0f};
					else
						return {0.0f, area * 2.0f};
				}

				const auto factor_left_edge = get_factor_by_ratio(double(left) / total_edge_length);
				const auto factor_right_edge = get_factor_by_ratio((1.0 + left) / total_edge_length);

				const auto signed_area = 0.5 * (factor_left_edge + factor_right_edge);

				return signed_area > 0 ? glm::vec2(signed_area, 0.0f) : glm::vec2(0.0f, -signed_area);
			};

			std::vector<glm::u8vec2> lut(lut_size * lut_size);

			for (const auto [right, left] : std::views::cartesian_product(
					 std::views::iota(0zu, lut_size),
					 std::views::iota(0zu, lut_size)
				 ))
			{
				const auto area = compute_area(left, right);
				const auto top_area = area.x;
				const auto bottom_area = area.y;
				lut[right * lut_size + left] = glm::u8vec2(
					glm::clamp(glm::round(bottom_area * 255.0f), 0.0f, 255.0f),
					glm::clamp(glm::round(top_area * 255.0f), 0.0f, 255.0f)
				);
			}

			return lut;
		}

		std::vector<glm::u8vec2> generate_diagonal_lut_block(uint8_t pattern, size_t lut_size)
		{
			[[assume(pattern < 16)]];

			const bool left_vert = (pattern & 0x01) != 0;
			const bool left_hori = (pattern & 0x02) != 0;
			const bool right_vert = (pattern & 0x04) != 0;
			const bool right_hori = (pattern & 0x08) != 0;

			const auto pop_count = std::popcount(pattern);
			if (pop_count == 0 || pop_count == 4)
				return std::vector<glm::u8vec2>(lut_size * lut_size, glm::u8vec2(0.5, 0.5));
		}
	}

	std::vector<glm::u8vec2> generate_ortho_area_lut_data(size_t lut_size) noexcept
	{
		auto pattern_to_result_subtexture_idx = [](uint8_t pattern) -> std::pair<uint8_t, uint8_t> {
			const auto bit0 = pattern & 0x1;
			const auto bit1 = (pattern >> 1) & 0x1;
			const auto bit2 = (pattern >> 2) & 0x1;
			const auto bit3 = (pattern >> 3) & 0x1;

			const auto x = (bit0 << 1) | bit2;
			const auto y = (bit1 << 1) | bit3;

			return {x >= 2 ? x + 1 : x, y >= 2 ? y + 1 : y};
		};

		std::vector<glm::u8vec2> result(lut_size * lut_size * 25);
		std::ranges::fill(result, glm::u8vec2(0, 0));

		for (const auto [block_y, block_x] :
			 std::views::cartesian_product(std::views::iota(0u, 4u), std::views::iota(0u, 4u)))
		{
			const auto [result_block_x, result_block_y] =
				pattern_to_result_subtexture_idx(block_y * 4 + block_x);

			const auto block_lut = generate_blend_lut_block(block_y * 4 + block_x, lut_size);

			for (const auto [y, x] : std::views::cartesian_product(
					 std::views::iota(0zu, lut_size),
					 std::views::iota(0zu, lut_size)
				 ))
			{
				const auto result_x = result_block_x * lut_size + x;
				const auto result_y = result_block_y * lut_size + y;
				result[result_y * lut_size * 5 + result_x] = block_lut[y * lut_size + x];
			}
		}

		return result;
	}

	std::vector<glm::u8vec2> generate_diagonal_area_lut_data(size_t lut_size) noexcept {}

	std::expected<gpu::Texture, util::Error> generate_ortho_area_lut(
		SDL_GPUDevice* device,
		size_t lut_size
	) noexcept
	{
		constexpr auto lut_texture_format = gpu::Texture::Format{
			.type = SDL_GPU_TEXTURETYPE_2D,
			.format = SDL_GPU_TEXTUREFORMAT_R8G8_UNORM,
			.usage = {.sampler = true}
		};

		const auto lut_data = generate_ortho_area_lut_data(lut_size);

		auto texture = gpu::Texture::create(device, lut_texture_format.create(lut_size * 5, lut_size * 5));
		if (!texture) return texture.error().propagate("Create LUT texture failed");

		auto transfer_buffer =
			gpu::Transfer_buffer::create_from_data(device, std::as_bytes(std::span(lut_data)));
		if (!transfer_buffer) return transfer_buffer.error().propagate("Create transfer buffer failed");

		auto command_buffer = gpu::Command_buffer::acquire_from(device);
		if (!command_buffer) return command_buffer.error().propagate("Acquire command buffer failed");

		const auto copy_result = command_buffer->run_copy_pass([&](const gpu::Copy_pass& copy_pass) {
			const SDL_GPUTextureTransferInfo transfer_info{
				.transfer_buffer = *transfer_buffer,
				.offset = 0,
				.pixels_per_row = uint32_t(lut_size * 5),
				.rows_per_layer = uint32_t(lut_size * 5)
			};

			const SDL_GPUTextureRegion texture_region{
				.texture = *texture,
				.mip_level = 0,
				.layer = 0,
				.x = 0,
				.y = 0,
				.z = 0,
				.w = uint32_t(lut_size * 5),
				.h = uint32_t(lut_size * 5),
				.d = 1
			};

			copy_pass.upload_to_texture(transfer_info, texture_region, false);
		});
		if (!copy_result) return copy_result.error().propagate("Copy to texture failed");

		const auto submit_result = command_buffer->submit();
		if (!submit_result) return submit_result.error().propagate("Submit command buffer failed");

		return texture;
	}
}