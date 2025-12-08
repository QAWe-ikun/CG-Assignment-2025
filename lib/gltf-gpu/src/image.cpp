#include "gltf/image.hpp"

#include <graphics/util/quick-create.hpp>
#include <image/algo/mipmap.hpp>
#include <image/compress.hpp>
#include <util/as-byte.hpp>

namespace gltf
{
	std::vector<Image_refcount> compute_image_refcounts(const tinygltf::Model& model) noexcept
	{
		std::vector<Image_refcount> refcount_list(model.images.size());

		const auto count_color_texture = [&model, &refcount_list](const auto& texture_info) {
			if (const auto index = texture_info.index;
				index >= 0 && std::cmp_less(index, model.textures.size()))
			{
				if (const auto source = model.textures[index].source;
					source >= 0 && std::cmp_less(source, model.images.size()))
				{
					refcount_list[source].color_refcount++;
				}
			}
		};

		const auto count_normal_texture = [&model, &refcount_list](const auto& texture_info) {
			if (const auto index = texture_info.index;
				index >= 0 && std::cmp_less(index, model.textures.size()))
			{
				if (const auto source = model.textures[index].source;
					source >= 0 && std::cmp_less(source, model.images.size()))
				{
					refcount_list[source].normal_refcount++;
				}
			}
		};

		for (const auto& material : model.materials)
		{
			count_color_texture(material.pbrMetallicRoughness.baseColorTexture);
			count_color_texture(material.pbrMetallicRoughness.metallicRoughnessTexture);
			count_color_texture(material.occlusionTexture);
			count_color_texture(material.emissiveTexture);
			count_normal_texture(material.normalTexture);
		}

		return refcount_list;
	}

	template <typename T>
	static std::span<const T> as_span(const tinygltf::Image& image) noexcept
	{
		return std::span<const T>(reinterpret_cast<const T*>(image.image.data()), image.width * image.height);
	}

	static std::expected<image::Image<image::Precision::U8, image::Format::RGBA>, util::Error>
	extract_u8_rgba(const tinygltf::Image& image) noexcept
	{
		const bool is_8bit = image.bits == 8 && image.pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
		const bool is_16bit = image.bits == 16 && image.pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT;

		image::Image<image::Precision::U8, image::Format::RGBA> img{
			.size = glm::u32vec2(uint32_t(image.width), uint32_t(image.height)),
			.pixels = std::vector<image::Pixel_t<image::Precision::U8, image::Format::RGBA>>(
				image.width * image.height
			)
		};

		if (image.component == 4)
		{
			if (is_8bit)
				std::ranges::copy(as_span<glm::u8vec4>(image), img.pixels.begin());
			else if (is_16bit)
				std::ranges::copy(
					as_span<glm::u16vec4>(image) | std::views::transform([](const glm::u16vec4& rgba) {
						return glm::u8vec4(rgba / uint16_t(256));
					}),
					img.pixels.begin()
				);
			else
				return util::Error(
					std::format(
						"Mismatched image bit depth ({}) or pixel type ({})",
						image.bits,
						image.pixel_type
					)
				);
		}
		else if (image.component == 3)
		{
			if (is_8bit)
				std::ranges::copy(
					as_span<glm::u8vec3>(image) | std::views::transform([](const glm::u8vec3& rgb) {
						return glm::u8vec4(rgb, 255);
					}),
					img.pixels.begin()
				);
			else if (is_16bit)
				std::ranges::copy(
					as_span<glm::u16vec3>(image) | std::views::transform([](const glm::u16vec3& rgb) {
						return glm::u8vec4(rgb / uint16_t(256), 255);
					}),
					img.pixels.begin()
				);
			else
				return util::Error(
					std::format(
						"Mismatched image bit depth ({}) or pixel type ({})",
						image.bits,
						image.pixel_type
					)
				);
		}
		else
			return util::Error(
				std::format("Unsupported number of components ({}) for color texture.", image.component)
			);

		return std::move(img);
	}

	static std::expected<image::Image<image::Precision::U16, image::Format::RGBA>, util::Error>
	extract_u16_rgba(const tinygltf::Image& image) noexcept
	{
		if (image.bits != 16 || image.pixel_type != TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
			return util::Error(
				std::format(
					"Mismatched image bit depth ({}) or pixel type ({})",
					image.bits,
					image.pixel_type
				)
			);

		image::Image<image::Precision::U16, image::Format::RGBA> img{
			.size = glm::u32vec2(uint32_t(image.width), uint32_t(image.height)),
			.pixels = std::vector<image::Pixel_t<image::Precision::U16, image::Format::RGBA>>(
				image.width * image.height
			)
		};

		if (image.component == 4)
			std::ranges::copy(as_span<glm::u16vec4>(image), img.pixels.begin());
		else if (image.component == 3)
			std::ranges::copy(
				as_span<glm::u16vec3>(image) | std::views::transform([](const glm::u16vec3& rgb) {
					return glm::u8vec4(rgb, 255);
				}),
				img.pixels.begin()
			);
		else
			return util::Error(
				std::format("Unsupported number of components ({}) for color texture.", image.component)
			);

		return std::move(img);
	}

	static bool dim_power_of_2(uint32_t value) noexcept
	{
		return (value != 0) && ((value & (value - 1)) == 0);
	}

	static bool image_power_of_2(glm::u32vec2 size) noexcept
	{
		return dim_power_of_2(size.x) && dim_power_of_2(size.y);
	}

	static bool image_size_multiple_of_block(glm::u32vec2 size) noexcept
	{
		return (size.x % 4 == 0) && (size.y % 4 == 0);
	}

	static auto create_texture_from_mipmap_fn(SDL_GPUDevice* device, SDL_GPUTextureFormat format) noexcept
	{
		return [device, format](const auto& img) -> std::expected<gpu::Texture, util::Error> {
			return graphics::create_texture_from_mipmap(
				device,
				gpu::Texture::Format{
					.type = SDL_GPU_TEXTURETYPE_2D,
					.format = format,
					.usage = {.sampler = true}
				},
				img
			);
		};
	}

	template <typename T>
	static std::function<std::expected<gpu::Texture, util::Error>(const image::Image_container<T>&)>
	create_texture_from_image_fn(SDL_GPUDevice* device, SDL_GPUTextureFormat format) noexcept
	{
		return [device, format](const image::Image_container<T>& img) {
			return graphics::create_texture_from_image(
				device,
				gpu::Texture::Format{
					.type = SDL_GPU_TEXTURETYPE_2D,
					.format = format,
					.usage = {.sampler = true}
				},
				img
			);
		};
	}

	static std::expected<gpu::Texture, util::Error> create_color_uncompressed(
		SDL_GPUDevice* device,
		const tinygltf::Image& image
	) noexcept
	{
		return extract_u8_rgba(image)
			.transform([](const auto& uncompressed_image) {
				return image::generate_mipmap(uncompressed_image);
			})
			.and_then(create_texture_from_mipmap_fn(device, SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM))
			.transform_error(util::Error::propagate_fn());
	}

	static std::expected<gpu::Texture, util::Error> create_color_bc3(
		SDL_GPUDevice* device,
		const tinygltf::Image& image
	) noexcept
	{
		const auto image_size = glm::u32vec2(uint32_t(image.width), uint32_t(image.height));

		if (!image_power_of_2(image_size))
		{
			if (!image_size_multiple_of_block(image_size))  // No compress, no mipmaps
			{
				return create_color_uncompressed(device, image).transform_error(util::Error::propagate_fn());
			}
			else  // Compress, no mipmaps
			{
				return extract_u8_rgba(image)
					.and_then(image::compress_to_bc3)
					.and_then(
						create_texture_from_image_fn<image::BC_block_8bpp>(
							device,
							SDL_GPU_TEXTUREFORMAT_BC3_RGBA_UNORM
						)
					)
					.transform_error(util::Error::propagate_fn());
			}
		}

		return extract_u8_rgba(image)
			.transform([](const auto& uncompressed_image) {
				return image::generate_mipmap(uncompressed_image, {4, 4});
			})
			.and_then(image::Compress_mipmap(image::compress_to_bc3))
			.and_then(create_texture_from_mipmap_fn(device, SDL_GPU_TEXTUREFORMAT_BC3_RGBA_UNORM))
			.transform_error(util::Error::propagate_fn());
	}

	static std::expected<gpu::Texture, util::Error> create_color_bc7(
		SDL_GPUDevice* device,
		const tinygltf::Image& image
	) noexcept
	{
		const auto image_size = glm::u32vec2(uint32_t(image.width), uint32_t(image.height));

		if (!image_power_of_2(image_size))
		{
			if (!image_size_multiple_of_block(image_size))  // No compress, no mipmaps
			{
				return create_color_uncompressed(device, image);
			}
			else  // Compress, no mipmaps
			{
				return extract_u8_rgba(image)
					.and_then(image::compress_to_bc7)
					.and_then(
						create_texture_from_image_fn<image::BC_block_8bpp>(
							device,
							SDL_GPU_TEXTUREFORMAT_BC7_RGBA_UNORM
						)
					)
					.transform_error(util::Error::propagate_fn());
			}
		}

		return extract_u8_rgba(image)
			.transform([](const auto& uncompressed_image) {
				return image::generate_mipmap(uncompressed_image, {4, 4});
			})
			.and_then(image::Compress_mipmap(image::compress_to_bc7))
			.and_then(create_texture_from_mipmap_fn(device, SDL_GPU_TEXTUREFORMAT_BC7_RGBA_UNORM))
			.transform_error(util::Error::propagate_fn());
	}

	static std::expected<gpu::Texture, util::Error> create_normal_8bit(
		SDL_GPUDevice* device,
		const tinygltf::Image& image,
		bool compress
	) noexcept
	{
		const auto image_size = glm::u32vec2(uint32_t(image.width), uint32_t(image.height));

		// Non Power-of-two textures are not compressed and mipmapped
		if (!image_power_of_2(image_size))
		{
			if (compress && image_size_multiple_of_block(image_size))  // Compress, no mipmaps
			{
				return extract_u8_rgba(image)
					.and_then(image::compress_to_bc5)
					.and_then(
						create_texture_from_image_fn<image::BC_block_8bpp>(
							device,
							SDL_GPU_TEXTUREFORMAT_BC5_RG_UNORM
						)
					)
					.transform_error(util::Error::propagate_fn());
			}
			else  // No compress, no mipmaps
			{
				return extract_u8_rgba(image)
					.transform([](const auto& img) {
						return img.map([](const glm::u8vec4& pixel) -> glm::u8vec2 {
							return {pixel.r, pixel.g};
						});
					})
					.and_then(
						create_texture_from_image_fn<glm::u8vec2>(device, SDL_GPU_TEXTUREFORMAT_R8G8_UNORM)
					)
					.transform_error(util::Error::propagate_fn());
			}
		}

		// Power-of-two textures can be compressed
		if (compress)
		{
			return extract_u8_rgba(image)
				.transform([](const auto& img) { return image::generate_mipmap(img, {4, 4}); })
				.and_then(image::Compress_mipmap(image::compress_to_bc5))
				.and_then(create_texture_from_mipmap_fn(device, SDL_GPU_TEXTUREFORMAT_BC5_RG_UNORM))
				.transform_error(util::Error::propagate_fn());
		}
		else
		{
			return extract_u8_rgba(image)
				.transform([](const auto& img) {
					return image::generate_mipmap(img.map([](const glm::u8vec4& pixel) -> glm::u8vec2 {
						return {pixel.r, pixel.g};
					}));
				})
				.and_then(create_texture_from_mipmap_fn(device, SDL_GPU_TEXTUREFORMAT_R8G8_UNORM))
				.transform_error(util::Error::propagate_fn());
		}
	}

	static std::expected<gpu::Texture, util::Error> create_normal_16bit(
		SDL_GPUDevice* device,
		const tinygltf::Image& image,
		bool compress
	) noexcept
	{
		const auto image_size = glm::u32vec2(uint32_t(image.width), uint32_t(image.height));

		// Non Power-of-two textures
		if (!image_power_of_2(image_size))
		{
			if (compress && image_size_multiple_of_block(image_size))  // Compress, no mipmaps
			{
				return extract_u16_rgba(image)
					.transform([&](const auto& img) {
						return img.map([](const glm::u16vec4& pixel) {
							return glm::u8vec4(pixel / uint16_t(256));
						});
					})
					.and_then(image::compress_to_bc5)
					.and_then(
						create_texture_from_image_fn<image::BC_block_8bpp>(
							device,
							SDL_GPU_TEXTUREFORMAT_BC5_RG_UNORM
						)
					)
					.transform_error(util::Error::propagate_fn());
			}
			else  // No compress, no mipmaps
			{
				return extract_u16_rgba(image)
					.transform([](const auto& image) {
						return image.map([](const glm::u16vec4& pixel) -> glm::u16vec2 {
							return {pixel.r, pixel.g};
						});
					})
					.and_then(
						create_texture_from_image_fn<glm::u16vec2>(device, SDL_GPU_TEXTUREFORMAT_R16G16_UNORM)
					)
					.transform_error(util::Error::propagate_fn());
			}
		}

		// Power-of-two textures can be compressed
		if (compress)
		{
			return extract_u16_rgba(image)
				.transform([&](const auto& img) {
					return image::generate_mipmap(
						img.map([](const glm::u16vec4& pixel) -> glm::u8vec4 {
							return pixel / uint16_t(256);
						}),
						{4, 4}
					);
				})
				.and_then(image::Compress_mipmap(image::compress_to_bc5))
				.and_then(create_texture_from_mipmap_fn(device, SDL_GPU_TEXTUREFORMAT_BC5_RG_UNORM))
				.transform_error(util::Error::propagate_fn());
		}
		else
		{
			return extract_u16_rgba(image)
				.transform([](const auto& img) {
					return image::generate_mipmap(img.map([](const glm::u16vec4& pixel) -> glm::u16vec2 {
						return {pixel.r, pixel.g};
					}));
				})
				.and_then(create_texture_from_mipmap_fn(device, SDL_GPU_TEXTUREFORMAT_R16G16_UNORM))
				.transform_error(util::Error::propagate_fn());
		}
	}

	static std::expected<gpu::Texture, util::Error> create_normal(
		SDL_GPUDevice* device,
		const tinygltf::Image& image,
		Image_compress_mode compress_mode
	) noexcept
	{
		const bool compress_when_8bit =
			(compress_mode == Image_compress_mode::RGn_BC5
			 || compress_mode == Image_compress_mode::RG16_raw_RG8_BC5);
		const bool compress_when_16bit = (compress_mode == Image_compress_mode::RGn_BC5);

		if (image.bits == 8 && image.pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
			return create_normal_8bit(device, image, compress_when_8bit);
		else if (image.bits == 16 && image.pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
			return create_normal_16bit(device, image, compress_when_16bit);
		else
			return util::Error(
				std::format(
					"Unsupported image bit depth ({}) or pixel type ({})",
					image.bits,
					image.pixel_type
				)
			);
	}

	std::expected<gpu::Texture, util::Error> create_texture_from_image(
		SDL_GPUDevice* device,
		const tinygltf::Image& image,
		Image_compress_mode compress_mode
	) noexcept
	{
		switch (compress_mode)
		{
		case Image_compress_mode::RGBA8_raw:
			return create_color_uncompressed(device, image);
		case Image_compress_mode::RGBA8_BC3:
			return create_color_bc3(device, image);
		case Image_compress_mode::RGBA8_BC7:
			return create_color_bc7(device, image);
		case Image_compress_mode::RGn_BC5:
		case Image_compress_mode::RG16_raw_RG8_BC5:
		case Image_compress_mode::RGn_raw:
			return create_normal(device, image, compress_mode);
		}

		std::unreachable();
	}

	std::expected<gpu::Texture, util::Error> create_placeholder_image(
		SDL_GPUDevice* device,
		glm::vec4 color
	) noexcept
	{
		image::Image<image::Precision::U8, image::Format::RGBA> img{
			.size = glm::u32vec2(1, 1),
			.pixels = std::vector<image::Pixel_t<image::Precision::U8, image::Format::RGBA>>(1)
		};

		img.pixels[0] = glm::u8vec4(
			static_cast<std::uint8_t>(glm::clamp(color.r, 0.0f, 1.0f) * 255.0f),
			static_cast<std::uint8_t>(glm::clamp(color.g, 0.0f, 1.0f) * 255.0f),
			static_cast<std::uint8_t>(glm::clamp(color.b, 0.0f, 1.0f) * 255.0f),
			static_cast<std::uint8_t>(glm::clamp(color.a, 0.0f, 1.0f) * 255.0f)
		);

		return graphics::create_texture_from_image(
				   device,
				   {.type = SDL_GPU_TEXTURETYPE_2D,
					.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
					.usage = {.sampler = true}},
				   img
		)
			.transform_error(util::Error::propagate_fn("Create texture from image failed"));
	}
}