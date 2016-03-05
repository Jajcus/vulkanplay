#include "vkapi.h"
#include "surface.h"
#include "main.h"

#include <stdio.h>
#include <string.h>
#include <alloca.h>
#include <malloc.h>

#define GET_INST_PROC(x) { \
	vkapi.x = (PFN_##x) vkGetInstanceProcAddr(vkapi.instance, #x); \
	if (!vkapi.x) { \
		fprintf(stderr, "Failed to retrieve '" #x "' address\n"); \
		goto error; \
	} \
}

#define GET_DEV_PROC(x) { \
	vkapi.x = (PFN_##x) vkGetDeviceProcAddr(vkapi.device, #x); \
	if (!vkapi.x) { \
		fprintf(stderr, "Failed to retrieve '" #x "' address\n"); \
		goto error; \
	} \
}

struct vkapi vkapi = {};

const char * instance_extensions[] = {
	"VK_KHR_surface",
#ifdef HAVE_XCB
	"VK_KHR_xcb_surface",
#endif
#ifdef HAVE_WAYLAND
	"VK_KHR_wayland_surface",
#endif
	NULL
};

const char * device_extensions[] = {
	"VK_KHR_swapchain",
	NULL
};

int vkapi_init_instance(const char * app_name) {

	VkResult result;

	GET_INST_PROC(vkCreateInstance);
	GET_INST_PROC(vkEnumerateInstanceExtensionProperties);
	GET_INST_PROC(vkEnumerateInstanceLayerProperties);

	uint32_t ext_count;
	for(ext_count=0; instance_extensions[ext_count]; ext_count++);

	VkApplicationInfo app_info = {
		.sType=VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName="vulkan play",
		.apiVersion=VK_MAKE_VERSION(1, 0, 3),
	};

	VkInstanceCreateInfo inst_info = {
		.sType=VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.enabledExtensionCount=ext_count,
		.ppEnabledExtensionNames=instance_extensions,
		.pApplicationInfo=&app_info,
	};

	result = vkapi.vkCreateInstance(&inst_info, NULL, &vkapi.instance);

	if (result != VK_SUCCESS) {
		printf("vkCreateInstance failed: %i\n", result);
		goto error;
	}

	GET_INST_PROC(vkCreateDevice);
	GET_INST_PROC(vkDestroyInstance);
	GET_INST_PROC(vkDestroySurfaceKHR);
	GET_INST_PROC(vkEnumeratePhysicalDevices);
	GET_INST_PROC(vkGetPhysicalDeviceFeatures);
	GET_INST_PROC(vkGetPhysicalDeviceMemoryProperties);
	GET_INST_PROC(vkGetPhysicalDeviceProperties);
	GET_INST_PROC(vkGetPhysicalDeviceQueueFamilyProperties);
	GET_INST_PROC(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
	GET_INST_PROC(vkGetPhysicalDeviceSurfaceFormatsKHR);
	GET_INST_PROC(vkGetPhysicalDeviceSurfacePresentModesKHR);
	GET_INST_PROC(vkGetPhysicalDeviceSurfaceSupportKHR);
#ifdef HAVE_XCB
	GET_INST_PROC(vkCreateXcbSurfaceKHR);
#endif
#ifdef HAVE_WAYLAND
	GET_INST_PROC(vkCreateWaylandSurfaceKHR);
#endif

	return VK_SUCCESS;
error:
	if (vkapi.instance) vkapi.vkDestroyInstance(vkapi.instance, NULL);
	memset(&vkapi, 0, sizeof(vkapi));
	return VK_ERROR_INITIALIZATION_FAILED;
}

static int _vkapi_init_device_procs(void) {

	GET_DEV_PROC(vkAcquireNextImageKHR);
	GET_DEV_PROC(vkAllocateCommandBuffers);
	GET_DEV_PROC(vkAllocateDescriptorSets);
	GET_DEV_PROC(vkAllocateMemory);
	GET_DEV_PROC(vkBeginCommandBuffer);
	GET_DEV_PROC(vkBindBufferMemory);
	GET_DEV_PROC(vkCmdBeginRenderPass);
	GET_DEV_PROC(vkCmdBindDescriptorSets);
	GET_DEV_PROC(vkCmdBindPipeline);
	GET_DEV_PROC(vkCmdBindVertexBuffers);
	GET_DEV_PROC(vkCmdDraw);
	GET_DEV_PROC(vkCmdEndRenderPass);
	GET_DEV_PROC(vkCmdResetQueryPool);
	GET_DEV_PROC(vkCreateBuffer);
	GET_DEV_PROC(vkCreateCommandPool);
	GET_DEV_PROC(vkCreateDescriptorPool);
	GET_DEV_PROC(vkCreateDescriptorSetLayout);
	GET_DEV_PROC(vkCreateFramebuffer);
	GET_DEV_PROC(vkCreateGraphicsPipelines);
	GET_DEV_PROC(vkCreateImageView);
	GET_DEV_PROC(vkCreatePipelineLayout);
	GET_DEV_PROC(vkCreateQueryPool);
	GET_DEV_PROC(vkCreateRenderPass);
	GET_DEV_PROC(vkCreateSemaphore);
	GET_DEV_PROC(vkCreateShaderModule);
	GET_DEV_PROC(vkCreateSwapchainKHR);
	GET_DEV_PROC(vkDestroyBuffer);
	GET_DEV_PROC(vkDestroyCommandPool);
	GET_DEV_PROC(vkDestroyDescriptorPool);
	GET_DEV_PROC(vkDestroyDescriptorSetLayout);
	GET_DEV_PROC(vkDestroyDevice);
	GET_DEV_PROC(vkDestroyFramebuffer);
	GET_DEV_PROC(vkDestroyImageView);
	GET_DEV_PROC(vkDestroyPipeline);
	GET_DEV_PROC(vkDestroyPipelineLayout);
	GET_DEV_PROC(vkDestroyQueryPool);
	GET_DEV_PROC(vkDestroyRenderPass);
	GET_DEV_PROC(vkDestroySemaphore);
	GET_DEV_PROC(vkDestroyShaderModule);
	GET_DEV_PROC(vkDestroySwapchainKHR);
	GET_DEV_PROC(vkDeviceWaitIdle);
	GET_DEV_PROC(vkEndCommandBuffer);
	GET_DEV_PROC(vkFreeDescriptorSets);
	GET_DEV_PROC(vkFreeMemory);
	GET_DEV_PROC(vkGetBufferMemoryRequirements);
	GET_DEV_PROC(vkGetDeviceQueue);
	GET_DEV_PROC(vkGetQueryPoolResults);
	GET_DEV_PROC(vkGetSwapchainImagesKHR);
	GET_DEV_PROC(vkMapMemory);
	GET_DEV_PROC(vkQueuePresentKHR);
	GET_DEV_PROC(vkQueueSubmit);
	GET_DEV_PROC(vkResetCommandPool);
	GET_DEV_PROC(vkResetDescriptorPool);
	GET_DEV_PROC(vkUnmapMemory);
	GET_DEV_PROC(vkUpdateDescriptorSets);

	if (options.stats) {
		GET_DEV_PROC(vkCmdBeginQuery);
		GET_DEV_PROC(vkCmdEndQuery);
	}

	return VK_SUCCESS;
error:
	return VK_ERROR_INITIALIZATION_FAILED;
}

int vkapi_init_device(struct plat_surface * surface) {

	VkResult result;
	uint32_t i, j;
	uint32_t pd_count = 0;
	result = vkapi.vkEnumeratePhysicalDevices(vkapi.instance, &pd_count, NULL);

	VkSurfaceKHR vk_surface = surface->vk_surface;

	if (result != VK_SUCCESS) {
		fprintf(stderr, "vkEnumeratePhysicalDevices failed: %i\n", result);
		goto error;
	}
	if (!pd_count) {
		fprintf(stderr, "vkEnumeratePhysicalDevices returned 0 devices\n");
		goto error;
	}

	printf("%i physical devices found\n", pd_count);

	vkapi.physical_devices = (VkPhysicalDevice *)calloc(pd_count, sizeof(VkPhysicalDevice));

	result = vkapi.vkEnumeratePhysicalDevices(vkapi.instance, &pd_count, vkapi.physical_devices);

	if (result != VK_SUCCESS) {
		fprintf(stderr, "vkEnumeratePhysicalDevices failed: %i\n", result);
		goto error;
	}

	uint32_t selected_dev = UINT32_MAX, selected_g_qf, selected_p_qf;

	VkPhysicalDeviceProperties dev_props;
	for(i = 0; i < pd_count; i++) {
		vkapi.vkGetPhysicalDeviceProperties(vkapi.physical_devices[i], &dev_props);
		printf("device #%i: %04x:%04x %s (ver: %x, API ver: %i.%i.%i, type: %i)\n",
				i, dev_props.vendorID, dev_props.deviceID, dev_props.deviceName,
				dev_props.driverVersion,
				VK_VERSION_MAJOR(dev_props.apiVersion),
				VK_VERSION_MINOR(dev_props.apiVersion),
				VK_VERSION_PATCH(dev_props.apiVersion),
			       	dev_props.deviceType);
		uint32_t qfp_count = 0;
		vkapi.vkGetPhysicalDeviceQueueFamilyProperties(vkapi.physical_devices[i], &qfp_count, NULL);
		VkQueueFamilyProperties * qf_props = calloc(qfp_count, sizeof(VkQueueFamilyProperties));
		vkapi.vkGetPhysicalDeviceQueueFamilyProperties(vkapi.physical_devices[i], &qfp_count, qf_props);
		if (selected_dev == UINT32_MAX) {
			selected_g_qf = UINT32_MAX;
			selected_p_qf = UINT32_MAX;
		}
		for(j = 0; j < qfp_count; j++) {
			printf("    queue family #%i: %i queues, flags: %x, ts bits: %i, mitg: (%i, %i, %i)\n",
					j, qf_props[j].queueCount, qf_props[j].queueFlags, qf_props[j].timestampValidBits,
					qf_props[j].minImageTransferGranularity.width,
					qf_props[j].minImageTransferGranularity.height,
					qf_props[j].minImageTransferGranularity.depth);
			if (selected_dev == UINT32_MAX) {
				if (selected_g_qf == UINT32_MAX && (qf_props[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
					selected_g_qf = j;
					printf("        good for graphics\n");
				}
				if (vk_surface) {
					VkBool32 supported;
					result = vkapi.vkGetPhysicalDeviceSurfaceSupportKHR(vkapi.physical_devices[i], j, vk_surface, &supported);
					if (result != VK_SUCCESS) {
						printf("vkGetPhysicalDeviceSurfaceSupportKHR failed");
						continue;
					}
					if (selected_p_qf == UINT32_MAX && supported) {
						selected_p_qf = j;
						printf("        good for presentation\n");
					}
				}
			}
		}
		if (selected_dev == UINT32_MAX && selected_g_qf != UINT32_MAX && (!vk_surface || selected_p_qf != UINT32_MAX)) {
			selected_dev = i;
			vkapi.device_properties = dev_props;
			printf("    you are the chosen one!\n");
		}
		free(qf_props);
	}

	if (selected_dev == UINT32_MAX) {
		fprintf(stderr, "No suitable physical device found\n");
		goto error;
	}
	else if (vk_surface) {
		fprintf(stderr, "Using device #%u, queue #%u for graphics and queue #%u for presentation\n",
				selected_dev, selected_g_qf, selected_p_qf);
	}
	else {
		fprintf(stderr, "Using device #%u, queue #%u for graphics\n",
				selected_dev, selected_g_qf);
	}

	vkapi.vkGetPhysicalDeviceFeatures(vkapi.physical_devices[selected_dev], &vkapi.device_features);

	if (options.stats && !vkapi.device_features.pipelineStatisticsQuery) {
		fprintf(stderr, "Statistics queries not available on this device!\n");
		goto error;
	}
	vkapi.vkGetPhysicalDeviceMemoryProperties(vkapi.physical_devices[selected_dev], &vkapi.memory_properties);

	vkapi.g_queue_family = selected_p_qf;
	vkapi.p_queue_family = selected_p_qf;

	float q_priority = 0.0;
	struct VkDeviceQueueCreateInfo queue_ci[2] = {
		{
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueCount = 1,
		.pQueuePriorities = &q_priority,
		.queueFamilyIndex = selected_g_qf,
		},
		{
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueCount = 1,
		.pQueuePriorities = &q_priority,
		.queueFamilyIndex = selected_p_qf,
		}
	};

	/* require no optional features */
	VkPhysicalDeviceFeatures features = {};

	uint32_t ext_count;
	for(ext_count=0; device_extensions[ext_count]; ext_count++);

	struct VkDeviceCreateInfo dev_ci = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = (!vk_surface || selected_g_qf == selected_p_qf) ? 1 : 2,
		.pQueueCreateInfos = queue_ci,
		.pEnabledFeatures = &features,
		.enabledExtensionCount=ext_count,
		.ppEnabledExtensionNames=device_extensions,
	};

	result = vkapi.vkCreateDevice(vkapi.physical_devices[selected_dev], &dev_ci, NULL, &vkapi.device);

	if (result != VK_SUCCESS) {
		fprintf(stderr, "vkCreateDevice failed: %i\n", result);
		goto error;
	}

	vkapi.selected_device = selected_dev;
	result = _vkapi_init_device_procs();
	if (result != VK_SUCCESS) goto error;

	vkapi.vkGetDeviceQueue(vkapi.device, selected_g_qf, 0, &vkapi.g_queue);
	if (!vk_surface) {
		vkapi.p_queue = VK_NULL_HANDLE;
	}
	else if (selected_g_qf == selected_p_qf) {
		vkapi.p_queue = vkapi.g_queue;
	}
	else {
		vkapi.vkGetDeviceQueue(vkapi.device, selected_p_qf, 0, &vkapi.p_queue);
	}

	vkapi.physical_device = vkapi.physical_devices[vkapi.selected_device];

	result = vkapi.vkGetPhysicalDeviceSurfaceFormatsKHR(vkapi.physical_device, vk_surface, &surface->s_formats_count, NULL);
	if (result != VK_SUCCESS) {
		fprintf(stderr, "vkGetPhysicalDeviceSurfaceFormatsKHR failed: %i\n", result);
		goto error;
	}
	surface->s_formats = (VkSurfaceFormatKHR *)calloc(surface->s_formats_count, sizeof(VkSurfaceFormatKHR));
	result = vkapi.vkGetPhysicalDeviceSurfaceFormatsKHR(vkapi.physical_device, vk_surface, &surface->s_formats_count, surface->s_formats);
	if (result != VK_SUCCESS) {
		fprintf(stderr, "vkGetPhysicalDeviceSurfaceFormatsKHR failed: %i\n", result);
		goto error;
	}
	result = vkapi.vkGetPhysicalDeviceSurfacePresentModesKHR(vkapi.physical_device, vk_surface, &surface->s_modes_count, NULL);
	if (result != VK_SUCCESS) {
		fprintf(stderr, "vkGetPhysicalDeviceSurfacePresentModesKHR failed: %i\n", result);
		goto error;
	}
	surface->s_modes = (VkPresentModeKHR *)calloc(surface->s_modes_count, sizeof(VkPresentModeKHR));
	result = vkapi.vkGetPhysicalDeviceSurfacePresentModesKHR(vkapi.physical_device, vk_surface, &surface->s_modes_count, surface->s_modes);
	if (result != VK_SUCCESS) {
		fprintf(stderr, "vkGetPhysicalDeviceSurfacePresentModesKHR failed: %i\n", result);
		goto error;
	}

	if ((surface->s_formats_count == 1) && (surface->s_formats[0].format == VK_FORMAT_UNDEFINED)) {
		/* vk_surface doesn't care, choose what we like */
		printf("Surface format is undefined, using VK_FORMAT_B8G8R8A8_SRGB\n");
		surface->s_format = VK_FORMAT_B8G8R8A8_SRGB;
		surface->s_colorspace = surface->s_formats[0].colorSpace;
	}
	else {
		for(i = 0; i < surface->s_formats_count; i++) {
			if (surface->s_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB) {
				printf("Surface supports VK_FORMAT_B8G8R8A8_SRGB, using it\n");
				surface->s_format = VK_FORMAT_B8G8R8A8_SRGB;
				surface->s_colorspace = surface->s_formats[i].colorSpace;
				break;
			}
		}
		if (i == surface->s_formats_count) {
			surface->s_format = surface->s_formats[0].format;
			surface->s_colorspace = surface->s_formats[0].colorSpace;
			printf("Using vk_surface preferred format: %i\n", surface->s_format);
		}
	}

	return VK_SUCCESS;
error:
	return VK_ERROR_INITIALIZATION_FAILED;
}

void vkapi_finish_device(void) {

	if (vkapi.device) {
		vkapi.vkDeviceWaitIdle(vkapi.device);
		vkapi.vkDestroyDevice(vkapi.device, NULL);
	}
	vkapi.device = VK_NULL_HANDLE;
	vkapi.g_queue_family = UINT32_MAX;
	vkapi.p_queue_family = UINT32_MAX;
	vkapi.g_queue = VK_NULL_HANDLE;
	vkapi.p_queue = VK_NULL_HANDLE;
}

void vkapi_finish(void) {

	if (vkapi.device) vkapi_finish_device();
	if (vkapi.instance) vkapi.vkDestroyInstance(vkapi.instance, NULL);
	if (vkapi.physical_devices) free(vkapi.physical_devices);
	vkapi.physical_device = VK_NULL_HANDLE;
	vkapi.instance = VK_NULL_HANDLE;
}
