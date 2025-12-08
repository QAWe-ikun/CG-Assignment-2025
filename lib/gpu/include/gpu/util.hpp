#pragma once

#include "util/error.hpp"
#include <format>

#define RETURN_SDL_ERROR return util::Error(std::format("SDL Error: {}", SDL_GetError()));