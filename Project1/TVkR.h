#pragma once

#include "libs.h"
#include "VkApplication.h"

#define ENGINE_NAME TVkR
#define ENGINE_NAME_STR std::string(STRINGIFY(ENGINE_NAME))

#define ENGINE_VERSION_MAJOR 0
#define ENGINE_VERSION_MINOR 1
#define ENGINE_VERSION_PATCH 0
#define ENGINE_VERSION_FULL ENGINE_VERSION_MAJOR.ENGINE_VERSION_MINOR.ENGINE_VERSION_PATCH
#define ENGINE_VERSION_FULL_STR std::string(STRINGIFY(ENGINE_VERSION_FULL))
#define ENGINE_VERSION VK_MAKE_VERSION(ENGINE_VERSION_MAJOR,ENGINE_VERSION_MINOR,ENGINE_VERSION_PATCH);

#define ENGINE_FULL_NAME ENGINE_NAME ENGINE_VERSION_FULL
#define ENGINE_FULL_NAME_STR std::string(STRINGIFY(ENGINE_FULL_NAME))

#define VERSION_TO_VK_VER(vers) { VK_MAKE_VERSION(vers.major,vers.minor,vers.patch) }

// so fe, this:
/*

Version ver(5, 7, 2);

uint32_t vkver = VERSION_TO_VK_VER(ver);

*/
// turns into this:
/*

Version ver(5, 7, 2);

uint32_t vkver = { { __typeof(ver) *__tmp; __tmp = (Version *)NULL; }; (((ver.major) << 22) | ((ver.minor) << 12) | (ver.patch)) };

*/

/*

VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) {
	auto func = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pCallback);
	} else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

*/
/*
#define Vk_EXTENSION_FUNC(name) 

#define _Vk_EXTENSION_FUNC(name, ...) VkResult name (VkInstance instance) {\
	auto func = (PFN_vk##name) vkGetInstanceProcAddr(instance, "vk##name##");\
	if (func != nullptr) {\
		return func(instance, __VA_ARGS__);\
	} else {\
		return VK_ERROR_EXTENSION_NOT_PRESENT;\
	}\
}
*/
//Vk_EXTENSION_FUNCTION(CreateDebugReportCallbackEXT);

typedef struct Version {

	Version() {}
	Version(int maj, int min, int pat) {
		major = maj;
		minor = min;
		patch = pat;
	};

	uint32_t major : 10;
	uint32_t minor : 10;
	uint32_t patch : 12;
} Version;