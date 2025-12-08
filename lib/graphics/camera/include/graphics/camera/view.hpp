#pragma once

#include <glm/ext/matrix_double4x4.hpp>

namespace graphics::camera
{
	class Projection;

	///
	/// @brief Interface for a camera view
	///
	///
	class View
	{
	  public:

		virtual ~View() = default;

		///
		/// @brief Get the view matrix
		///
		/// @return View matrix
		///
		virtual glm::dmat4 matrix() const noexcept = 0;

		///
		/// @brief Get the eye position in world space
		///
		/// @return Eye position
		///
		virtual glm::vec3 eye_position() const noexcept = 0;
	};
}