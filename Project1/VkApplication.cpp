#include "VkApplication.h"
#include "TVkR.h"

#define LOAD_DEBUG_REPORT
#include "VkExtensions.h"

#include "utils.h"

#include <algorithm>
#include <iostream>
#include <set>

#ifdef USE_VALIDATION

const std::vector<const char*> validationLayers = {
	"VK_LAYER_LUNARG_standard_validation"
};

#endif // !NDEBUG

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

VkApplication::VkApplication(int w, int h, std::string nam, Version ver)
{
	destructed = false;

	width = w;
	height = h;

	app_name = nam;

	version = ver;
}

VkApplication::~VkApplication()
{
	if (!destructed) cleanup();
}

void VkApplication::run()
{
	initWindow();
	initVulkan();
	mainLoop();
	cleanup();
}

void VkApplication::initWindow()
{
	glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(width, height, this->app_name.c_str(), nullptr, nullptr);
}

#ifdef USE_VALIDATION
void setupDebugCallback(VkInstance inst, VkDebugReportCallbackEXT* callback) {
	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	createInfo.pfnCallback = VkApplication::debugCallback;

	if (CreateDebugReportCallbackEXT(inst, &createInfo, nullptr, callback) != VK_SUCCESS) {
		ERROR("Failed to set up debug callback!");
	}
}
#endif


#ifdef USE_VALIDATION
bool checkValidationSupport() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}
		if (!layerFound) {
			return false;
		}
	}

	return true;
}
#endif

std::vector<const char*> getNeededExtensions() {
	std::vector<const char*> extensions;

	unsigned int glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	for (unsigned int i = 0; i < glfwExtensionCount; i++) {
		extensions.push_back(glfwExtensions[i]);
	}

#ifdef USE_VALIDATION
	extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif

	return extensions;
}

void VkApplication::createInstance()
{
#ifdef USE_VALIDATION
	if (!checkValidationSupport()) {
		ERROR("Validation layers requested, but not available!");
	}
#endif

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = app_name.c_str();
	appInfo.applicationVersion = VERSION_TO_VK_VER(version);
	appInfo.pEngineName = ENGINE_NAME_STR.c_str();
	appInfo.engineVersion = ENGINE_VERSION;
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	auto extensions = getNeededExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

#ifdef USE_VALIDATION
	createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
	createInfo.ppEnabledLayerNames = validationLayers.data();
#else
	createInfo.enabledLayerCount = 0;
#endif

	if (vkCreateInstance(&createInfo, nullptr, &inst) != VK_SUCCESS) {
		ERROR("Failed to create instance!");
	}
}

void VkApplication::createSurface()
{
	if (glfwCreateWindowSurface(inst, window, nullptr, &surface) != VK_SUCCESS) {
		ERROR("Failed to create window surface!");
	}
}

// Create device
#if 1
// Queue families block
#if 1
struct QueueFamilies
{
	int GRAPHICS = -1;
	int COMPUTE = -1;
	int TRANSFER = -1;
	int SPARSE_BINDING = -1;

	int presenter = -1;

	bool hasAll() {
		return (presenter + 1) && (GRAPHICS + 1) && (COMPUTE + 1) && (TRANSFER + 1) && (SPARSE_BINDING + 1);
	}
};

QueueFamilies findQueueFamilies(VkApplication* app, VkPhysicalDevice device) {
	QueueFamilies families;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

#define QUEUEFAMILY_BITCHECK(type) if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_##type##_BIT) families.##type = i;

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		QUEUEFAMILY_BITCHECK(GRAPHICS)
		QUEUEFAMILY_BITCHECK(COMPUTE)
		QUEUEFAMILY_BITCHECK(TRANSFER)
		QUEUEFAMILY_BITCHECK(SPARSE_BINDING)

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, app->getSurface(), &presentSupport);

		if (queueFamily.queueCount > 0 && presentSupport) {
			families.presenter = i;
		}

		if (families.hasAll()) {
			break;
		}

		i++;
	}

#undef QUEUEFAMILY_BITCHECK

	return families;
}
#endif

bool checkExtensionSupport(VkPhysicalDevice device) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

// Swap chain block
#if 1
struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

SwapChainSupportDetails querySwapChainSupport(VkApplication* app, VkPhysicalDevice device) {
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, app->getSurface(), &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, app->getSurface(), &formatCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, app->getSurface(), &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, app->getSurface(), &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, app->getSurface(), &presentModeCount, details.presentModes.data());
	}

	return details;
}

VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) {
	if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
		return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	for (const auto& availableFormat : formats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return formats[0];
}

VkPresentModeKHR chooseSwapMode(const std::vector<VkPresentModeKHR> presentModes) {
	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

	for (const auto& availablePresentMode : presentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
		else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
			bestMode = availablePresentMode;
		}
	}

	return bestMode;
}

VkExtent2D chooseSwapExtent(VkApplication* app, const VkSurfaceCapabilitiesKHR& capabilities) {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else {
		VkExtent2D actualExtent = { app->getWidth(), app->getHeight() };

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

void VkApplication::createSwapChain() {
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(this, physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(this, swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilies indices = findQueueFamilies(this, physicalDevice);
	uint32_t queueFamilyIndices[] = { (uint32_t)indices.GRAPHICS, (uint32_t)indices.presenter };

	if (indices.GRAPHICS != indices.presenter) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
		ERROR("Failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

	imageFormat = surfaceFormat.format;
	swapChainExtent = extent;
}
#endif

bool scoreDevice(VkApplication* app, VkPhysicalDevice device) {
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
	
	QueueFamilies families = findQueueFamilies(app, device);

	bool supportsExtensions = checkExtensionSupport(device);

	bool goodSwapChain = false;
	if (supportsExtensions) {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(app, device);
		goodSwapChain = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	return families.GRAPHICS != -1 && families.presenter != -1 && (supportsExtensions && goodSwapChain);
}

void VkApplication::pickDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(inst, &deviceCount, nullptr);

	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(inst, &deviceCount, devices.data());

	for (const auto& device : devices) {
		if (scoreDevice(this, device)) {
			physicalDevice = device;
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE) {
		ERROR("failed to find a suitable GPU!");
	}
}

void VkApplication::createLogicalDevice()
{
	QueueFamilies indices = findQueueFamilies(this, physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<int> uniqueQueueFamilies = { indices.GRAPHICS, indices.presenter };

	float queuePriority = 1.0f;
	for (int queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures = {};

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

#ifdef USE_VALIDATION
	createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
	createInfo.ppEnabledLayerNames = validationLayers.data();
#else
	createInfo.enabledLayerCount = 0;
#endif

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
		ERROR("Failed to create logical device!");
	}

	vkGetDeviceQueue(device, indices.GRAPHICS, 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presenter, 0, &presentQueue);
}
#endif

void VkApplication::initVulkan()
{
	createInstance();

#ifdef USE_VALIDATION
	setupDebugCallback(inst, &callback);
#endif

	createSurface();
	pickDevice();

	createLogicalDevice();
	createSwapChain();
	createImageViews();

	createGFXPipleine();
}

void VkApplication::createImageViews() {
	swapChainImageViews.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++) {
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = imageFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;
		
		if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
			ERROR("Failed to create image views!");
		}
	}
}

// Shaders
#if 1
VkShaderModule createShaderModule(VkDevice* device, const std::vector<char>& code) {
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(*device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		ERROR("Failed to create shader module!");
	}

	return shaderModule;
}

void VkApplication::createGFXPipleine() {
	auto vertShaderCode = utils::readFile("shaders/vert.spv");
	auto fragShaderCode = utils::readFile("shaders/frag.spv");

	VkShaderModule vertShaderModule = createShaderModule(&device, vertShaderCode);
	VkShaderModule fragShaderModule = createShaderModule(&device, fragShaderCode);



	vkDestroyShaderModule(device, fragShaderModule, nullptr);
	vkDestroyShaderModule(device, vertShaderModule, nullptr);
}
#endif

void VkApplication::mainLoop()
{
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}
}

void VkApplication::cleanup()
{
	destructed = true;

	for (size_t i = 0; i < swapChainImageViews.size(); i++) {
		vkDestroyImageView(device, swapChainImageViews[i], nullptr);
	}

	vkDestroySwapchainKHR(device, swapChain, nullptr);
	vkDestroySurfaceKHR(inst, surface, nullptr);

#ifdef USE_VALIDATION
	DestroyDebugReportCallbackEXT(inst, callback, nullptr);
#endif

	vkDestroyDevice(device, nullptr);
	vkDestroyInstance(inst, nullptr);

	glfwDestroyWindow(window);
	glfwTerminate();
}

#ifdef USE_VALIDATION
VKAPI_ATTR VkBool32 VKAPI_CALL VkApplication::debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char * layerPrefix, const char * msg, void * userData)
{
	std::cerr << "[" << layerPrefix << "]: " << msg << std::endl;

	return VK_FALSE;
}
#endif
