#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_video.h>
#include <future>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/matrix.hpp>
#include <imgui.h>
#include <iostream>
#include <print>

#include "backend/imgui.hpp"
#include "backend/loop.hpp"
#include "backend/sdl.hpp"
#include "gltf/model.hpp"
#include "graphics/camera/projection/ortho.hpp"
#include "graphics/util/renderpass-copy.hpp"
#include "logic.hpp"
#include "pipeline/ambient-light.hpp"
#include "pipeline/ao.hpp"
#include "pipeline/debug.hpp"
#include "pipeline/directional-light.hpp"
#include "pipeline/sky-preetham.hpp"
#include "pipeline/tonemapping.hpp"
#include "renderer/aa.hpp"
#include "renderer/gbuffer.hpp"
#include "renderer/shadow.hpp"
#include "target.hpp"
#include "target/ao.hpp"
#include "target/composite.hpp"
#include "target/gbuffer.hpp"
#include "target/light.hpp"
#include "target/shadow.hpp"
#include "tiny_gltf.h"
#include "util/file.hpp"
#include "util/unwrap.hpp"

struct Render_resource
{
	renderer::Antialias aa_module;

	target::Gbuffer gbuffer_target;
	target::Shadow shadow_target;
	target::Light_buffer light_buffer_target;
	target::Composite composite_target;
	target::AO ao_target;

	renderer::Gbuffer gbuffer_renderer;
	renderer::Shadow shadow_renderer;

	pipeline::Directional_light directional_light_pipeline;
	pipeline::Ambient_light ambient_light_pipeline;
	pipeline::AO ao_pipeline;
	pipeline::Tonemapping tonemapping_pipeline;
	pipeline::Debug_pipeline debug_pipeline;
	pipeline::Sky_preetham sky_pipeline;

	graphics::Renderpass_copy depth_to_color_copier;
};

static std::expected<Render_resource, util::Error> create_render_resource(
	const backend::SDL_context& context
) noexcept
{
	auto aa_module = renderer::Antialias::create(context.device, context.get_swapchain_texture_format());
	if (!aa_module) return aa_module.error().forward("Create antialias module failed");

	auto gbuffer_renderer = renderer::Gbuffer::create(context.device);
	if (!gbuffer_renderer) return gbuffer_renderer.error().forward("Create G-buffer renderer failed");

	auto shadow_renderer = renderer::Shadow::create(context.device);
	if (!shadow_renderer) return shadow_renderer.error().forward("Create shadow renderer failed");

	auto directional_light_pipeline = pipeline::Directional_light::create(context.device);
	if (!directional_light_pipeline)
		return directional_light_pipeline.error().forward("Create directional light pipeline failed");

	auto ambient_light_pipeline = pipeline::Ambient_light::create(context.device);
	if (!ambient_light_pipeline)
		return ambient_light_pipeline.error().forward("Create environment light pipeline failed");

	auto tonemapping_pipeline =
		pipeline::Tonemapping::create(context.device, target::Composite::composite_format.format);
	if (!tonemapping_pipeline)
		return tonemapping_pipeline.error().forward("Create tonemapping pipeline failed");

	auto ao_pipeline = pipeline::AO::create(context.device);
	if (!ao_pipeline) return ao_pipeline.error().forward("Create AO pipeline failed");

	auto debug_pipeline = pipeline::Debug_pipeline::create(
		context.device,
		gpu::Texture::Format{
			.type = SDL_GPU_TEXTURETYPE_2D,
			.format = context.get_swapchain_texture_format(),
			.usage = {.color_target = true}
		}
	);
	if (!debug_pipeline) return debug_pipeline.error().forward("Create debug pipeline failed");

	auto sky_pipeline = pipeline::Sky_preetham::create(context.device);
	if (!sky_pipeline) return sky_pipeline.error().forward("Create sky pipeline failed");

	auto depth_to_color_copier =
		graphics::Renderpass_copy::create(context.device, 1, target::Gbuffer::depth_value_format);
	if (!depth_to_color_copier)
		return depth_to_color_copier.error().forward("Create depth to color copier failed");

	return Render_resource{
		.aa_module = std::move(*aa_module),
		.gbuffer_target = target::Gbuffer{},
		.shadow_target = target::Shadow{},
		.light_buffer_target = target::Light_buffer{},
		.composite_target = target::Composite{},
		.ao_target = target::AO{},

		.gbuffer_renderer = std::move(*gbuffer_renderer),
		.shadow_renderer = std::move(*shadow_renderer),

		.directional_light_pipeline = std::move(*directional_light_pipeline),
		.ambient_light_pipeline = std::move(*ambient_light_pipeline),
		.ao_pipeline = std::move(*ao_pipeline),
		.tonemapping_pipeline = std::move(*tonemapping_pipeline),
		.debug_pipeline = std::move(*debug_pipeline),
		.sky_pipeline = std::move(*sky_pipeline),

		.depth_to_color_copier = std::move(*depth_to_color_copier)
	};
}

static std::expected<gltf::Model, util::Error> create_scene_from_model(
	const backend::SDL_context& context,
	const std::vector<std::byte>& model_data
) noexcept
{
	auto gltf_load_result = backend::display_until_task_done(
		context,
		std::async(std::launch::async, gltf::load_tinygltf_model, std::ref(model_data)),
		[] {
			ImGui::Text("加载模型...");
			ImGui::ProgressBar(-ImGui::GetTime(), ImVec2(300.0f, 0.0f));
		}
	);
	if (!gltf_load_result) return gltf_load_result.error().forward("Load tinygltf model failed");

	std::atomic<gltf::Model::Load_progress> load_progress;

	auto future = std::async(std::launch::async, [&context, &gltf_load_result, &load_progress]() {
		return gltf::Model::load_model(
			context.device,
			*gltf_load_result,
			gltf::Sampler_config{.anisotropy = 4.0f},
			{.color_mode = gltf::Color_compress_mode::RGBA8_BC7,
			 .normal_mode = gltf::Normal_compress_mode::RGn_BC5},
			std::ref(load_progress)
		);
	});

	auto gltf_result = backend::display_until_task_done(context, std::move(future), [&load_progress] {
		const auto current = load_progress.load();

		switch (current.stage)
		{
		case gltf::Model::Load_stage::Node:
			ImGui::Text("解析节点树...");
			break;
		case gltf::Model::Load_stage::Mesh:
			ImGui::Text("分析并优化网格...");
			break;
		case gltf::Model::Load_stage::Material:
			ImGui::Text("压缩材质...");
			break;
		case gltf::Model::Load_stage::Animation:
			ImGui::Text("解析动画...");
			break;
		case gltf::Model::Load_stage::Postprocess:
			ImGui::Text("处理中...");
			break;
		}

		ImGui::ProgressBar(current.progress.value_or(-ImGui::GetTime()), ImVec2(300.0f, 0.0f));
	});
	if (!gltf_result) return gltf_result.error().forward("Load gltf model failed");

	return gltf_result;
}

int main(int argc, const char** argv)
try
{
	std::span<const char*> args(argv, argc);
	if (args.size() != 2)
	{
		argv[1] = "/mnt/Dev/Dev/Art/glTF-Sample-Models-main/2.0/Sponza/sponza-tangent.glb";
		args = std::span<const char*>(argv, 2);
	}

	/* 初始化 */

#ifdef NDEBUG
	constexpr bool enable_debug_layer = false;
#else
	constexpr bool enable_debug_layer = true;
#endif

	backend::initialize_sdl() | util::unwrap("Initialize SDL failed");
	const auto sdl_context =
		backend::SDL_context::create(1280, 720, "Demo", SDL_WINDOW_RESIZABLE, enable_debug_layer)
		| util::unwrap("Initialize SDL Backend failed");

	const auto window = sdl_context->window;
	const auto gpu_device = sdl_context->device;

	SDL_SetWindowMinimumSize(window, 640, 480);

	backend::initialize_imgui(*sdl_context) | util::unwrap();

	/* 加载模型和贴图 */

	auto scene =
		util::read_file(args[1], 1024 * 1048576)
			.and_then([&sdl_context](const std::vector<std::byte>& model) {
				return create_scene_from_model(*sdl_context, model);
			})
		| util::unwrap("Load 3D model failed");

	/* 创建管线 */

	auto render_resource =
		backend::display_until_task_done(
			*sdl_context,
			std::async(std::launch::async, create_render_resource, std::ref(*sdl_context)),
			[] {
				ImGui::Text("创建渲染管线...");
				ImGui::ProgressBar(-ImGui::GetTime(), ImVec2(300.0f, 0.0f));
			}
		)
		| util::unwrap("Create render resource failed");

	Logic logic;

	/* 主循环 */

	bool quit = false;
	while (!quit)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			backend::imgui_handle_event(&event);
			if (event.type == SDL_EVENT_QUIT) quit = true;
		}

		backend::imgui_new_frame();

		const auto logic_result = logic.logic();
		const auto camera_matrix = logic_result.camera.proj_matrix * logic_result.camera.view_matrix;

		const auto drawdata = scene.generate_drawdata(glm::mat4(1.0f));

		renderer::Gbuffer::Drawdata gbuffer_drawdata(camera_matrix, logic_result.camera.eye_position);
		gbuffer_drawdata.append(drawdata);
		renderer::Shadow::Drawdata shadow_drawdata(
			camera_matrix,
			logic_result.light_direction,
			gbuffer_drawdata.get_min_z(),
			logic_result.csm_linear_blend
		);
		shadow_drawdata.append(drawdata);

		auto command_buffer = gpu::Command_buffer::acquire_from(gpu_device) | util::unwrap();
		const auto [swapchain_texture, swapchain_width, swapchain_height] =
			command_buffer.wait_and_acquire_swapchain_texture(window) | util::unwrap();
		const glm::u32vec2 swapchain_size{swapchain_width, swapchain_height};

		// 渲染
		if (swapchain_texture == nullptr)
		{
			command_buffer.submit() | util::unwrap();
			continue;
		}

		render_resource.gbuffer_target.cycle(gpu_device, swapchain_size) | util::unwrap();
		render_resource.light_buffer_target.cycle(gpu_device, swapchain_size) | util::unwrap();
		render_resource.composite_target.resize(gpu_device, swapchain_size) | util::unwrap();
		render_resource.shadow_target.resize(gpu_device, {3072, 3072}) | util::unwrap();
		render_resource.ao_target.cycle(gpu_device, swapchain_size) | util::unwrap();

		backend::imgui_upload_data(command_buffer);

		command_buffer.push_debug_group("GBuffer Pass");
		auto gbuffer_pass =
			acquire_gbuffer_pass(
				command_buffer,
				render_resource.gbuffer_target,
				render_resource.light_buffer_target
			)
			| util::unwrap("获取 G-buffer Pass失败");
		render_resource.gbuffer_renderer.render(command_buffer, gbuffer_pass, gbuffer_drawdata);
		gbuffer_pass.end();
		command_buffer.pop_debug_group();

		render_resource.depth_to_color_copier.copy(
			command_buffer,
			*render_resource.gbuffer_target.depth_texture,
			render_resource.gbuffer_target.depth_value_texture.current()
		) | util::unwrap("Copy depth to color failed");

		command_buffer.push_debug_group("Shadow Pass");
		render_resource.shadow_renderer.render(command_buffer, render_resource.shadow_target, shadow_drawdata)
			| util::unwrap("渲染阴影失败");
		command_buffer.pop_debug_group();

		command_buffer.push_debug_group("AO Pass");
		auto ao_pass = target::acquire_ao_pass(command_buffer, render_resource.ao_target)
			| util::unwrap("获取 AO Pass失败");
		render_resource.ao_pipeline.render(
			command_buffer,
			ao_pass,
			render_resource.ao_target,
			render_resource.gbuffer_target,
			pipeline::AO::Params{
				.camera_mat_inv = glm::inverse(camera_matrix),
				.view_mat = logic_result.camera.view_matrix,
				.proj_mat_inv = glm::inverse(logic_result.camera.proj_matrix),
				.prev_camera_mat = logic_result.camera.prev_camera_matrix,
				.radius = logic_result.ambient_lighting.ao_radius,
				.blend_alpha = logic_result.ambient_lighting.ao_blend_ratio
			}
		);
		ao_pass.end();
		command_buffer.pop_debug_group();

		command_buffer.push_debug_group("Lighting Pass");
		auto lighting_pass =
			acquire_lighting_pass(
				command_buffer,
				render_resource.light_buffer_target,
				render_resource.gbuffer_target
			)
			| util::unwrap("获取光照Pass失败");
		{
			render_resource.directional_light_pipeline.render(
				command_buffer,
				lighting_pass,
				render_resource.gbuffer_target,
				render_resource.shadow_target,
				pipeline::Directional_light::Params{
					.camera_matrix_inv = glm::inverse(camera_matrix),
					.shadow_matrix_level0 = shadow_drawdata.get_vp_matrix(0),
					.shadow_matrix_level1 = shadow_drawdata.get_vp_matrix(1),
					.shadow_matrix_level2 = shadow_drawdata.get_vp_matrix(2),
					.eye_position = logic_result.camera.eye_position,
					.light_direction = logic_result.light_direction,
					.light_color = logic_result.light_color
				}
			);

			render_resource.ambient_light_pipeline.render(
				command_buffer,
				lighting_pass,
				render_resource.gbuffer_target,
				render_resource.ao_target,
				pipeline::Ambient_light::Param{
					.ambient_intensity = glm::vec3(logic_result.ambient_lighting.ambient_intensity),
					.ao_strength = logic_result.ambient_lighting.ao_strength
				}
			);

			render_resource.sky_pipeline.render(
				command_buffer,
				lighting_pass,
				pipeline::Sky_preetham::Params{
					.camera_mat_inv = glm::inverse(camera_matrix),
					.screen_size = swapchain_size,
					.eye_position = logic_result.camera.eye_position,
					.sun_direction = logic_result.light_direction,
					.sun_intensity = logic_result.light_color * logic_result.sky_brightness_mult,
					.turbidity = logic_result.turbidity
				}
			);
		}
		lighting_pass.end();
		command_buffer.pop_debug_group();

		if (logic_result.debug_mode == Logic::Debug_mode::No_debug)
		{
			command_buffer.push_debug_group("Tonemapping Pass");
			render_resource.tonemapping_pipeline.render(
				command_buffer,
				render_resource.light_buffer_target.light_texture.current(),
				*render_resource.composite_target.composite_texture,
				{.exposure = 1.0f}
			) | util::unwrap();
			command_buffer.pop_debug_group();

			command_buffer.push_debug_group("Antialiasing Pass");
			render_resource.aa_module.run_antialiasing(
				gpu_device,
				command_buffer,
				*render_resource.composite_target.composite_texture,
				swapchain_texture,
				swapchain_size,
				logic_result.aa_mode
			) | util::unwrap();
			command_buffer.pop_debug_group();
		}

		auto swapchain_pass =
			target::acquire_swapchain_pass(
				command_buffer,
				swapchain_texture,
				logic_result.debug_mode != Logic::Debug_mode::No_debug
			)
			| util::unwrap("获取交换链Pass通道失败");
		{

			if (logic_result.debug_mode == Logic::Debug_mode::Show_AO)
			{
				render_resource.debug_pipeline.render_single_channel(
					command_buffer,
					swapchain_pass,
					render_resource.ao_target.halfres_ao_texture.current(),
					{swapchain_width, swapchain_height}
				);
			}

			backend::imgui_draw_to_renderpass(command_buffer, swapchain_pass);
		}
		swapchain_pass.end();

		command_buffer.submit() | util::unwrap();
	}

	return EXIT_SUCCESS;
}
catch (const util::Error& e)
{
	std::println(std::cerr, "\033[91m[错误]\033[0m {}", e->front().message);
	e.dump_trace();
	std::terminate();
}
catch (const std::exception& e)
{
	std::println(std::cerr, "\033[91m[异常]\033[0m {}", e.what());
	std::terminate();
}
catch (...)
{
	std::println(std::cerr, "\033[91m[异常]\033[0m 未知异常发生");
	std::terminate();
}
