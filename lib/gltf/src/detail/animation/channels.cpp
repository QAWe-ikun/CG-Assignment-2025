#include "gltf/detail/animation/channels.hpp"

namespace gltf::detail::animation
{
	void Translation_channel::apply(std::span<Node::Transform_override> overrides, float time) const noexcept
	{
		overrides[target_node].translation = sampler[time];
	}

	void Rotation_channel::apply(std::span<Node::Transform_override> overrides, float time) const noexcept
	{
		overrides[target_node].rotation = sampler[time];
	}

	void Scale_channel::apply(std::span<Node::Transform_override> overrides, float time) const noexcept
	{
		overrides[target_node].scale = sampler[time];
	}
}