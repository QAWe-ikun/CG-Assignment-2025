#pragma once

#include <format>
#include <util/error.hpp>

#define RETURN_SDL_ERROR return util::Error(std::format("SDL Error: {}", SDL_GetError()));