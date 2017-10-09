#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "libs.h"
#include "TVkR.h"

#include <vector>

#ifndef NDEBUG
#define USE_VALIDATION
#endif

class VkApplication
{
public:
	VkApplication(int width, int height, std::string app_name, Version version);
	~VkApplication();

	void run();

public:
#ifdef USE_VALIDATION
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objType,
		uint64_t obj,
		size_t location,
		int32_t code,
		const char* layerPrefix,
		const char* msg,
		void* userData);
#endif
	
	VkSurfaceKHR getSurface() { return surface; }

	int getWidth() { return width; }
	int getHeight() { return height; }

private:

	int width;
	int height;
	std::string app_name;
	Version version;

	GLFWwindow* window;
	VkInstance inst;
	VkSurfaceKHR surface;

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentQueue;

	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;
	VkFormat imageFormat;
	VkExtent2D swapChainExtent;

#ifdef USE_VALIDATION
	VkDebugReportCallbackEXT callback;
#endif

	void initWindow();

	void initVulkan();
	void createInstance();
	void createSurface();
	void pickDevice();
	void createSwapChain();
	void createImageViews();
	void createLogicalDevice();
	void createGFXPipleine();

	void mainLoop();
	void cleanup();

	bool destructed;

};

