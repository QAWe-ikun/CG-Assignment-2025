#pragma once

#include <fastgltf/types.hpp>
#include <glm/glm.hpp>
#include <gpu.hpp>
#include <util/error.hpp>
#include <vector>

namespace model_parser::gltf
{
	namespace util
	{
		///
		/// @brief 将 fastgltf 的图元类型转换为 SDL_GPU 的图元类型
		///
		/// @param type fastgltf 图元类型
		/// @return SDL_GPU 图元类型
		///
		SDL_GPUPrimitiveType to_sdl_primitive_type(fastgltf::PrimitiveType type) noexcept;
	}

	// GLTF 顶点定义
	struct Vertex
	{
		glm::vec3 position;  // 位置
		glm::vec3 normal;    // 法线
		glm::vec3 binormal;  // 副切线
		glm::vec2 uv;        // 纹理坐标
	};

	// GLTF 网格（CPU侧）
	struct Primitive
	{
		fastgltf::PrimitiveType type;          // 图元类型
		std::vector<Vertex> vertices;          // 顶点数据
		std::vector<uint32_t> indices;         // 索引数据
		std::optional<size_t> material_index;  // 材质索引，若为空则使用白模
	};

	// GLTF 网格（GPU侧）
	struct Primitive_gpu
	{
		SDL_GPUPrimitiveType type;             // 图元类型
		gpu::Buffer vertex_buffer;             // 顶点缓冲区
		gpu::Buffer index_buffer;              // 索引缓冲区
		size_t index_count;                    // 索引数量
		std::optional<size_t> material_index;  // 材质索引，若为空则使用白模

		///
		/// @brief 将 CPU 侧的 Primitive 转换为 GPU 侧的 Primitive_gpu
		///
		/// @param device GPU设备
		/// @param primitive CPU 侧的 Primitive
		/// @return 转换结果，成功则返回 Primitive_gpu，失败则返回错误信息
		///
		static std::expected<Primitive_gpu, ::util::Error> from_primitive_cpu(
			SDL_GPUDevice* device,
			const Primitive& primitive
		) noexcept;
	};

	// GLTF 材料
	struct Material
	{
		glm::vec4 base_color_factor;                             // 基础颜色
		float roughness_factor;                                  // 粗糙度
		float metalness_factor;                                  // 金属度
		float emissive_factor;                                   // 自发光强度
		std::optional<size_t> base_color_texture_index;          // 基础颜色纹理索引
		std::optional<size_t> normal_texture_index;              // 法线纹理索引
		std::optional<size_t> metallic_roughness_texture_index;  // 金属粗糙度纹理索引
		std::optional<size_t> emissive_texture_index;            // 自发光纹理索引
	};

}