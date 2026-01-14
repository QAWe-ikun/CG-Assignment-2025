#pragma once

#include "backend/sdl.hpp"
#include "graphics/util/renderpass-copy.hpp"
#include "render/pipeline/aa.hpp"
#include "render/pipeline/ambient-light.hpp"
#include "render/pipeline/ao.hpp"
#include "render/pipeline/auto-exposure.hpp"
#include "render/pipeline/bloom.hpp"
#include "render/pipeline/directional-light.hpp"
#include "render/pipeline/gbuffer-gltf.hpp"
#include "render/pipeline/hiz-generator.hpp"
#include "render/pipeline/light.hpp"
#include "render/pipeline/shadow-gltf.hpp"
#include "render/pipeline/sky-preetham.hpp"
#include "render/pipeline/ssgi.hpp"
#include "render/pipeline/tonemapping.hpp"

namespace render
{
	struct Pipeline
	{
		pipeline::Antialias aa_module;
		pipeline::AmbientLight ambient_light;
		pipeline::AO ao;
		pipeline::AutoExposure auto_exposure;
		pipeline::Bloom bloom;
		pipeline::Directional_light directional_light;
		pipeline::GbufferGLTF gbuffer_gltf;
		pipeline::HizGenerator hiz_generator;
		pipeline::ShadowGLTF shadow_gltf;
		pipeline::SkyPreetham sky_preetham;
		pipeline::SSGI ssgi;
		pipeline::Tonemapping tonemapping;
		pipeline::Light point_light;

		graphics::RenderpassCopy depth_to_color_copier;

		static std::expected<Pipeline, util::Error> create(const backend::SDLcontext& context) noexcept;
	};
}
