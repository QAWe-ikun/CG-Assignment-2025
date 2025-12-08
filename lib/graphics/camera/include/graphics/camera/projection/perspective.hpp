#pragma once

#include "graphics/camera/projection.hpp"

#include <optional>

namespace graphics::camera::projection
{
	///
	/// @brief Perspective Projection
	///
	///
	struct Perspective : public Projection
	{
		Perspective(const Perspective&) = default;
		Perspective(Perspective&&) = default;
		Perspective& operator=(const Perspective&) = default;
		Perspective& operator=(Perspective&&) = default;

		///
		/// @brief Create a perspective projection
		///
		/// @param fov_y Vertical field of view in radians
		/// @param near_plane Distance to near plane
		/// @param far_plane Distance to far plane if given a number, infinite if given `std::nullopt` (the
		/// default)
		///
		Perspective(float fov_y, float near_plane, std::optional<float> far_plane) noexcept;

		virtual ~Perspective() = default;

		glm::dmat4 matrix(float aspect_ratio) noexcept override;

		float fov_y;
		float near_plane;
		std::optional<float> far_plane;  // See class constructor for details
	};
}