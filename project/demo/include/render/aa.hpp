#pragma once

#include <graphics/aa/empty.hpp>
#include <graphics/aa/fxaa.hpp>
#include <graphics/aa/mlaa.hpp>
#include <graphics/aa/smaa.hpp>

// Antialiasing Processor Module
class Antialias_module
{
	graphics::aa::Empty empty_processor;
	graphics::aa::FXAA fxaa_processor;
	graphics::aa::MLAA mlaa_processor;
	graphics::aa::SMAA smaa_processor;

	Antialias_module(
		graphics::aa::Empty empty_processor,
		graphics::aa::FXAA fxaa_processor,
		graphics::aa::MLAA mlaa_processor,
		graphics::aa::SMAA smaa_processor
	) noexcept;

  public:

	Antialias_module(const Antialias_module&) = delete;
	Antialias_module(Antialias_module&&) = default;
	Antialias_module& operator=(const Antialias_module&) = delete;
	Antialias_module& operator=(Antialias_module&&) = default;

	enum class Mode
	{
		None,
		FXAA,
		MLAA,
		SMAA
	};

	static std::expected<Antialias_module, util::Error> create(
		SDL_GPUDevice* device,
		SDL_GPUTextureFormat format
	) noexcept;

	std::expected<void, util::Error> run_antialiasing(
		SDL_GPUDevice* device,
		const gpu::Command_buffer& command_buffer,
		SDL_GPUTexture* source,
		SDL_GPUTexture* target,
		glm::u32vec2 size,
		Mode mode
	) noexcept;
};