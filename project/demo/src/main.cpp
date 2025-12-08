#include <SDL3/SDL_events.h>
#include <algorithm>
#include <array>
#include <future>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <iostream>
#include <limits>
#include <print>

#include "asset/shader/simple.frag.hpp"
#include "asset/shader/simple.vert.hpp"

#include "config/general.hpp"
#include "config/texture.hpp"
#include "gltf/sampler.hpp"
#include "render/aa.hpp"

#include <backend/imgui.hpp>
#include <backend/loop.hpp>
#include <backend/sdl.hpp>
#include <gltf.hpp>
#include <gpu.hpp>
#include <graphics/aa/empty.hpp>
#include <graphics/aa/fxaa.hpp>
#include <graphics/aa/mlaa.hpp>
#include <graphics/aa/smaa.hpp>
#include <graphics/camera/projection/ortho.hpp>
#include <graphics/camera/projection/perspective.hpp>
#include <graphics/camera/view/orbit.hpp>
#include <graphics/corner.hpp>
#include <graphics/culling.hpp>
#include <graphics/smallest-bound.hpp>
#include <graphics/util/quick-create.hpp>
#include <graphics/util/smart-texture.hpp>
#include <image/algo/mipmap.hpp>
#include <image/compress.hpp>
#include <image/io.hpp>
#include <tiny_gltf.h>
#include <util/asset.hpp>
#include <util/file.hpp>
#include <util/unwrap.hpp>
#include <zip/zip.hpp>

static std::pair<gpu::Graphic_shader, gpu::Graphic_shader> create_shaders(SDL_GPUDevice* device)
{
	auto vertex_shader =
		gpu::Graphic_shader::create(
			device,
			std::as_bytes(shader_asset::simple_vert),
			gpu::Graphic_shader::Stage::Vertex,
			0,
			0,
			0,
			1
		)
		| util::unwrap("创建顶点着色器失败");

	auto fragment_shader =
		gpu::Graphic_shader::create(
			device,
			std::as_bytes(shader_asset::simple_frag),
			gpu::Graphic_shader::Stage::Fragment,
			1,
			0,
			0,
			0
		)
		| util::unwrap("创建片段着色器失败");

	return {std::move(vertex_shader), std::move(fragment_shader)};
}

using Vertex = gltf::Vertex;

static gpu::Graphics_pipeline create_pipeline(
	SDL_GPUDevice* device,
	SDL_Window* window [[maybe_unused]],
	const gpu::Graphic_shader& vertex_shader,
	const gpu::Graphic_shader& fragment_shader
)
{
	SDL_GPURasterizerState rasterizer_state;
	rasterizer_state.cull_mode = SDL_GPU_CULLMODE_BACK;
	rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
	rasterizer_state.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;
	rasterizer_state.depth_bias_clamp = 0;
	rasterizer_state.depth_bias_constant_factor = 0;
	rasterizer_state.depth_bias_slope_factor = 0;
	rasterizer_state.enable_depth_bias = false;
	rasterizer_state.enable_depth_clip = true;

	const auto vertex_attributes = std::to_array<SDL_GPUVertexAttribute>({
		SDL_GPUVertexAttribute{
							   .location = 0,
							   .buffer_slot = 0,
							   .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
							   .offset = offsetof(Vertex, position)
		},
		SDL_GPUVertexAttribute{
							   .location = 1,
							   .buffer_slot = 0,
							   .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
							   .offset = offsetof(Vertex, normal)
		},
		SDL_GPUVertexAttribute{
							   .location = 2,
							   .buffer_slot = 0,
							   .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
							   .offset = offsetof(Vertex, tangent)
		},
		SDL_GPUVertexAttribute{
							   .location = 3,
							   .buffer_slot = 0,
							   .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
							   .offset = offsetof(Vertex, texcoord)
		},
	});

	const auto vertex_buffer_descs = std::to_array<SDL_GPUVertexBufferDescription>({
		{.slot = 0,
		 .pitch = sizeof(Vertex),
		 .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
		 .instance_step_rate = 0},
	});

	const auto primary_color_target = [&] {
		SDL_GPUColorTargetDescription desc;
		desc.format = config::texture::color_texture_format.format;
		desc.blend_state = SDL_GPUColorTargetBlendState{
			.src_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
			.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ZERO,
			.color_blend_op = SDL_GPU_BLENDOP_ADD,
			.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
			.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ZERO,
			.alpha_blend_op = SDL_GPU_BLENDOP_ADD,
			.color_write_mask = SDL_GPU_COLORCOMPONENT_R
				| SDL_GPU_COLORCOMPONENT_G
				| SDL_GPU_COLORCOMPONENT_B
				| SDL_GPU_COLORCOMPONENT_A,
			.enable_blend = false,
			.enable_color_write_mask = false,
			.padding1 = 0,
			.padding2 = 0
		};
		return desc;
	}();

	const auto depth_stencil_state = gpu::Graphics_pipeline::Depth_stencil_state{
		.format = config::texture::depth_texture_format.format,
		.compare_op = SDL_GPU_COMPAREOP_GREATER,
		.back_stencil_state = {},
		.front_stencil_state = {},
		.compare_mask = 0xFF,
		.write_mask = 0xFF,
		.enable_depth_test = true,
		.enable_depth_write = true,
		.enable_stencil_test = false
	};

	const auto color_target_descs = std::to_array<SDL_GPUColorTargetDescription>({primary_color_target});

	return gpu::Graphics_pipeline::create(
			   device,
			   vertex_shader,
			   fragment_shader,
			   SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
			   SDL_GPU_SAMPLECOUNT_1,
			   rasterizer_state,
			   vertex_attributes,
			   vertex_buffer_descs,
			   color_target_descs,
			   depth_stencil_state
		   )
		| util::unwrap();
}

static gpu::Sampler create_sampler(SDL_GPUDevice* device)
{
	return gpu::Sampler::create(device, gpu::Sampler::Create_info{.max_anisotropy = 4.0}) | util::unwrap();
}

static std::pair<SDL_GPUColorTargetInfo, SDL_GPUDepthStencilTargetInfo> gen_color_target_info(
	SDL_GPUTexture* swapchain,
	SDL_GPUTexture* depth
)
{
	SDL_GPUColorTargetInfo swapchain_target;
	swapchain_target.texture = swapchain;
	swapchain_target.load_op = SDL_GPU_LOADOP_CLEAR;
	swapchain_target.store_op = SDL_GPU_STOREOP_STORE;
	swapchain_target.clear_color = SDL_FColor{.r = 0, .g = 0, .b = 0, .a = 1};
	swapchain_target.resolve_texture = nullptr;
	swapchain_target.cycle = false;
	swapchain_target.mip_level = 0;
	swapchain_target.layer_or_depth_plane = 0;

	SDL_GPUDepthStencilTargetInfo depth_target;
	depth_target.texture = depth;
	depth_target.clear_depth = 0.0f;
	depth_target.load_op = SDL_GPU_LOADOP_CLEAR;
	depth_target.store_op = SDL_GPU_STOREOP_STORE;
	depth_target.stencil_load_op = SDL_GPU_LOADOP_DONT_CARE;
	depth_target.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;
	depth_target.cycle = true;
	depth_target.clear_stencil = 0;

	return {swapchain_target, depth_target};
}

static SDL_GPUColorTargetInfo gen_swapchain_target_info(SDL_GPUTexture* swapchain)
{
	SDL_GPUColorTargetInfo swapchain_target;
	swapchain_target.texture = swapchain;
	swapchain_target.load_op = SDL_GPU_LOADOP_LOAD;
	swapchain_target.store_op = SDL_GPU_STOREOP_STORE;
	swapchain_target.clear_color = SDL_FColor{.r = 0, .g = 0, .b = 0, .a = 1};
	swapchain_target.resolve_texture = nullptr;
	swapchain_target.cycle = false;
	swapchain_target.mip_level = 0;
	swapchain_target.layer_or_depth_plane = 0;

	return swapchain_target;
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
			ImGui::Text("正在加载模型...");
			ImGui::ProgressBar(-ImGui::GetTime(), ImVec2(300.0f, 0.0f));
		}
	);
	if (!gltf_load_result) return gltf_load_result.error().propagate("加载 tinygltf 模型失败");

	std::atomic<gltf::Model::Load_progress> load_progress;

	auto future = std::async(std::launch::async, [&context, &gltf_load_result, &load_progress]() {
		return gltf::Model::load_model(
			context.device,
			*gltf_load_result,
			gltf::Sampler_config{.anisotropy = 4.0f},
			{.color_mode = gltf::Image_compress_mode::RGBA8_BC7,
			 .normal_mode = gltf::Image_compress_mode::RGn_BC5},
			std::ref(load_progress)
		);
	});

	auto gltf_result = backend::display_until_task_done(context, std::move(future), [&load_progress] {
		const auto current = load_progress.load();

		switch (current.stage)
		{
		case gltf::Model::Load_stage::Node:
			ImGui::Text("正在解析节点...");
			break;
		case gltf::Model::Load_stage::Mesh:
			ImGui::Text("正在解析网格...");
			break;
		case gltf::Model::Load_stage::Material:
			ImGui::Text("正在解析材质...");
			break;
		case gltf::Model::Load_stage::Animation:
			ImGui::Text("正在解析动画...");
			break;
		case gltf::Model::Load_stage::Postprocess:
			ImGui::Text("处理中...");
			break;
		}

		ImGui::ProgressBar(current.progress.value_or(-ImGui::GetTime()), ImVec2(300.0f, 0.0f));
	});
	if (!gltf_result) return gltf_result.error().propagate("加载 glTF 模型失败");

	return gltf_result;
}

#ifdef NDEBUG
constexpr bool is_debug_build = false;
#else
constexpr bool is_debug_build = true;
#endif

int main(int argc, const char** argv)
try
{
	std::span<const char*> args(argv, argc);
	if (args.size() != 2)
	{
		std::println("用法: {} <模型文件>", args[0]);
		return EXIT_FAILURE;
	}

	/* 初始化 */

	backend::initialize_sdl() | util::unwrap("初始化 SDL 失败");
	const auto sdl_context =
		backend::SDL_context::create(
			config::general::initial_window_width,
			config::general::initial_window_height,
			"图形学大作业技术Demo",
			SDL_WINDOW_RESIZABLE,
			is_debug_build
		)
		| util::unwrap("创建 SDL 上下文失败");

	const auto window = sdl_context->window;
	const auto gpu_device = sdl_context->device;
	const auto swapchain_format = sdl_context->get_swapchain_texture_format();

	backend::initialize_imgui(*sdl_context) | util::unwrap();

	/* 加载模型和贴图 */

	const auto scene =
		util::read_file(args[1], 1024 * 1048576)
			.and_then([&sdl_context](const std::vector<std::byte>& model) {
				return create_scene_from_model(*sdl_context, model);
			})
		| util::unwrap("加载模型失败");

	/* 创建管线 */

	const auto [vertex_shader, fragment_shader] = create_shaders(gpu_device);
	const auto graphics_pipeline = create_pipeline(gpu_device, window, vertex_shader, fragment_shader);
	const auto sampler = create_sampler(gpu_device);

	Antialias_module aa_module =
		Antialias_module::create(gpu_device, swapchain_format) | util::unwrap("创建抗锯齿模块失败");

	graphics::Smart_texture depth_texture(config::texture::depth_texture_format);
	graphics::Smart_texture color_texture(config::texture::color_texture_format);

	graphics::camera::view::Orbit
		camera_orbit(3, 0, 0, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	const graphics::camera::view::Orbit::Pan_controller pan_controller{0.5f};
	const graphics::camera::view::Orbit::Rotate_controller rotate_controller{
		.azimuth_per_width = glm::radians(360.0f),
		.pitch_per_height = glm::radians(180.0f)
	};

	graphics::camera::projection::Perspective camera_projection(glm::radians(45.0f), 0.1f, std::nullopt);

	Antialias_module::Mode aa_mode = Antialias_module::Mode::MLAA;

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

		const auto [width, height] = ImGui::GetIO().DisplaySize;

		const auto begin_time = std::chrono::high_resolution_clock::now();

		const glm::mat4 camera_matrix =
			camera_projection.matrix_reverse_z(float(width) / float(height)) * camera_orbit.matrix();

		const auto planes = graphics::compute_frustum_planes(camera_matrix);
		auto [drawcalls, material_cache] = scene.generate_drawdata(glm::mat4(1.0f));

		float min_z = std::numeric_limits<float>::max();

		std::vector<gltf::Drawcall> visible_drawcalls;
		visible_drawcalls.reserve(drawcalls.size());
		for (const auto& drawcall : drawcalls)
		{
			const auto [world_min, world_max] = graphics::local_bound_to_world(
				drawcall.primitive.position_min,
				drawcall.primitive.position_max,
				drawcall.transform
			);

			if (graphics::box_in_frustum(world_min, world_max, planes))
			{
				const auto projected_bound = graphics::transform_corner_points(
					graphics::get_corner_points(world_min, world_max),
					camera_matrix
				);
				const auto local_min_z = std::ranges::min(
					projected_bound | std::views::transform(&glm::vec3::z) | std::views::filter([](float z) {
						return z > 0.0;
					})
				);
				min_z = std::min(min_z, local_min_z);

				visible_drawcalls.push_back(drawcall);
			}
		}

		std::ranges::sort(visible_drawcalls, [](const gltf::Drawcall& a, const gltf::Drawcall& b) {
			return a.material_index.value_or(std::numeric_limits<uint32_t>::max())
				< b.material_index.value_or(std::numeric_limits<uint32_t>::max());
		});

		const auto light_dir = glm::normalize(glm::vec3(0.0, 1.0, 0.0));
		const auto corners = graphics::transform_corner_points(
			graphics::get_corner_points(glm::vec3(-1.0, -1.0, min_z), glm::vec3(1.0)),
			glm::inverse(camera_matrix)
		);
		const auto bounds = graphics::find_smallest_bound(corners, light_dir);
		const auto mat = glm::ortho(bounds.left, bounds.right, bounds.bottom, bounds.top, -10.0f, 10.0f);

		const auto end_time = std::chrono::high_resolution_clock::now();
		const auto load_duration = end_time - begin_time;
		const double load_ms =
			std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(load_duration).count();

		if (!ImGui::GetIO().WantCaptureMouse)
		{
			const auto delta = ImGui::GetIO().MouseDelta;
			if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
			{
				rotate_controller.rotate(camera_orbit, {float(width), float(height)}, {delta.x, delta.y});
			}

			if (ImGui::IsMouseDown(ImGuiMouseButton_Middle))
			{
				pan_controller.pan(camera_orbit, {float(width), float(height)}, {delta.x, delta.y});
			}

			const auto mouse_wheel = ImGui::GetIO().MouseWheel;
			if (mouse_wheel != 0.0f) camera_orbit.distance *= glm::pow(0.8f, mouse_wheel);
		}

		if (ImGui::Begin("设置", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (ImGui::RadioButton("无抗锯齿", aa_mode == Antialias_module::Mode::None))
				aa_mode = Antialias_module::Mode::None;
			if (ImGui::RadioButton("FXAA", aa_mode == Antialias_module::Mode::FXAA))
				aa_mode = Antialias_module::Mode::FXAA;
			if (ImGui::RadioButton("MLAA", aa_mode == Antialias_module::Mode::MLAA))
				aa_mode = Antialias_module::Mode::MLAA;
			if (ImGui::RadioButton("SMAA", aa_mode == Antialias_module::Mode::SMAA))
				aa_mode = Antialias_module::Mode::SMAA;

			ImGui::Separator();

			ImGui::Text("Drawdata Gen: %.2f ms", load_ms);
			ImGui::Text("Drawcall count: %zu", visible_drawcalls.size());

			const auto tri_count = std::ranges::fold_left(
				visible_drawcalls,
				size_t(0),
				[](size_t acc, const gltf::Drawcall& drawcall) {
					return acc + (drawcall.primitive.index_count / 3);
				}
			);
			ImGui::Text("Triangle count: %zu", tri_count);
			ImGui::Text("Smallest Z: %.4f", min_z);
		}
		ImGui::End();

		auto command_buffer = gpu::Command_buffer::acquire_from(gpu_device) | util::unwrap();
		const auto [swapchain_texture, swapchain_width, swapchain_height] =
			command_buffer.wait_and_acquire_swapchain_texture(window) | util::unwrap();

		backend::imgui_upload_data(command_buffer);

		// 渲染
		if (swapchain_texture != nullptr)
		{
			depth_texture.resize(gpu_device, {swapchain_width, swapchain_height}) | util::unwrap();
			color_texture.resize(gpu_device, {swapchain_width, swapchain_height}) | util::unwrap();

			const auto [color_target, depth_target] = gen_color_target_info(*color_texture, *depth_texture);
			const auto swapchain_color_target = gen_swapchain_target_info(swapchain_texture);

			command_buffer.run_render_pass(
				{&color_target, 1},
				depth_target,
				[&](const gpu::Render_pass& render_pass) {
					render_pass.set_viewport(
						SDL_GPUViewport{
							.x = 0,
							.y = 0,
							.w = float(swapchain_width),
							.h = float(swapchain_height),
							.min_depth = 0.0f,
							.max_depth = 1.0f
						}
					);

					render_pass.bind_pipeline(graphics_pipeline);

					for (const auto subrange :
						 visible_drawcalls
							 | std::views::chunk_by([](const gltf::Drawcall& a, const gltf::Drawcall& b) {
								   return a.material_index == b.material_index;
							   }))
					{
						if (subrange.empty()) continue;
						const auto material_bind =
							material_cache.get_material_bind(subrange.front().material_index);

						render_pass.bind_fragment_samplers(0, {&material_bind.base_color, 1});

						for (const auto& drawcall : subrange)
						{
							const glm::mat4 model_matrix = drawcall.transform;
							const glm::mat4 MVP = camera_matrix * model_matrix;
							command_buffer.push_uniform_to_vertex(0, util::as_bytes(MVP));

							render_pass
								.bind_vertex_buffers(0, {&drawcall.primitive.vertex_buffer_binding, 1});
							render_pass.bind_index_buffer(
								drawcall.primitive.index_buffer_binding,
								SDL_GPU_INDEXELEMENTSIZE_32BIT
							);
							render_pass.draw_indexed(drawcall.primitive.index_count, 0, 1, 0, 0);
						}
					}
				}
			) | util::unwrap();

			aa_module.run_antialiasing(
				gpu_device,
				command_buffer,
				*color_texture,
				swapchain_texture,
				{swapchain_width, swapchain_height},
				aa_mode
			) | util::unwrap();

			command_buffer.run_render_pass(
				{&swapchain_color_target, 1},
				{},
				[&command_buffer](const gpu::Render_pass& render_pass) {
					backend::imgui_draw_to_renderpass(command_buffer, render_pass);
				}
			) | util::unwrap();
		}

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