#pragma once

#include <vulkan/vulkan.h>

#include <stdexcept>
#include <functional>
#include <string>

#define ERROR_TYPE std::runtime_error
#define ERROR(msg) throw ERROR_TYPE(msg)
#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)