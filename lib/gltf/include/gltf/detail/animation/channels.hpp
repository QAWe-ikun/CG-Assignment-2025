#pragma once

#include "channel-def.hpp"
#include "sampler.hpp"

namespace gltf::detail::animation
{
	class Translation_channel : public Channel
	{
		uint32_t target_node;
		Sampler<glm::vec3> sampler;

	  public:

		// Note: No out-of-bound check for `target_node`
		Translation_channel(uint32_t target_node, Sampler<glm::vec3> sampler) :
			target_node(target_node),
			sampler(std::move(sampler))
		{}

		void apply(std::span<Node::Transform_override> overrides, float time) const noexcept override;
	};

	class Rotation_channel : public Channel
	{
		uint32_t target_node;
		Sampler<glm::quat> sampler;

	  public:

		// Note: No out-of-bound check for `target_node`
		Rotation_channel(uint32_t target_node, Sampler<glm::quat> sampler) :
			target_node(target_node),
			sampler(std::move(sampler))
		{}

		void apply(std::span<Node::Transform_override> overrides, float time) const noexcept override;
	};

	class Scale_channel : public Channel
	{
		uint32_t target_node;
		Sampler<glm::vec3> sampler;

	  public:

		// Note: No out-of-bound check for `target_node`
		Scale_channel(uint32_t target_node, Sampler<glm::vec3> sampler) :
			target_node(target_node),
			sampler(std::move(sampler))
		{}

		void apply(std::span<Node::Transform_override> overrides, float time) const noexcept override;
	};
}