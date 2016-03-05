#ifndef vkapi_h
#define vkapi_h

#ifdef HAVE_XCB
#define VK_USE_PLATFORM_XCB_KHR 1
#endif
#ifdef HAVE_WAYLAND
#define VK_USE_PLATFORM_WAYLAND_KHR 1
#endif

#include <vulkan/vulkan.h>

#define DEF_INST_PROC(x) PFN_##x x
#define DEF_DEV_PROC(x) PFN_##x x

struct vkapi {
	VkInstance instance;
	VkDevice device;
	VkPhysicalDevice *physical_devices;
	uint32_t selected_device;
	VkPhysicalDevice physical_device;
	VkPhysicalDeviceProperties device_properties;
	VkPhysicalDeviceFeatures device_features;
	VkPhysicalDeviceMemoryProperties memory_properties;

	uint32_t g_queue_family;
	VkQueue g_queue;
	uint32_t p_queue_family;
	VkQueue p_queue;

	DEF_INST_PROC(vkCreateDevice);
	DEF_INST_PROC(vkCreateInstance);
	DEF_INST_PROC(vkDestroyInstance);
	DEF_INST_PROC(vkDestroySurfaceKHR);
	DEF_INST_PROC(vkEnumerateInstanceExtensionProperties);
	DEF_INST_PROC(vkEnumerateInstanceLayerProperties);
	DEF_INST_PROC(vkEnumeratePhysicalDevices);
	DEF_INST_PROC(vkGetPhysicalDeviceFeatures);
	DEF_INST_PROC(vkGetPhysicalDeviceMemoryProperties);
	DEF_INST_PROC(vkGetPhysicalDeviceProperties);
	DEF_INST_PROC(vkGetPhysicalDeviceQueueFamilyProperties);
	DEF_INST_PROC(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
	DEF_INST_PROC(vkGetPhysicalDeviceSurfaceFormatsKHR);
	DEF_INST_PROC(vkGetPhysicalDeviceSurfacePresentModesKHR);
	DEF_INST_PROC(vkGetPhysicalDeviceSurfaceSupportKHR);

	DEF_DEV_PROC(vkAcquireNextImageKHR);
	DEF_DEV_PROC(vkAllocateCommandBuffers);
	DEF_DEV_PROC(vkAllocateDescriptorSets);
	DEF_DEV_PROC(vkAllocateMemory);
	DEF_DEV_PROC(vkBeginCommandBuffer);
	DEF_DEV_PROC(vkBindBufferMemory);
	DEF_DEV_PROC(vkCmdBeginQuery);
	DEF_DEV_PROC(vkCmdBeginRenderPass);
	DEF_DEV_PROC(vkCmdBindDescriptorSets);
	DEF_DEV_PROC(vkCmdBindPipeline);
	DEF_DEV_PROC(vkCmdBindVertexBuffers);
	DEF_DEV_PROC(vkCmdDraw);
	DEF_DEV_PROC(vkCmdEndQuery);
	DEF_DEV_PROC(vkCmdEndRenderPass);
	DEF_DEV_PROC(vkCmdPipelineBarrier);
	DEF_DEV_PROC(vkCmdResetQueryPool);
	DEF_DEV_PROC(vkCmdSetScissor);
	DEF_DEV_PROC(vkCmdSetViewport);
	DEF_DEV_PROC(vkCreateBuffer);
	DEF_DEV_PROC(vkCreateCommandPool);
	DEF_DEV_PROC(vkCreateDescriptorPool);
	DEF_DEV_PROC(vkCreateDescriptorSetLayout);
	DEF_DEV_PROC(vkCreateFence);
	DEF_DEV_PROC(vkCreateFramebuffer);
	DEF_DEV_PROC(vkCreateGraphicsPipelines);
	DEF_DEV_PROC(vkCreateImageView);
	DEF_DEV_PROC(vkCreatePipelineLayout);
	DEF_DEV_PROC(vkCreateQueryPool);
	DEF_DEV_PROC(vkCreateRenderPass);
	DEF_DEV_PROC(vkCreateSemaphore);
	DEF_DEV_PROC(vkCreateShaderModule);
	DEF_DEV_PROC(vkCreateSwapchainKHR);
	DEF_DEV_PROC(vkDestroyBuffer);
	DEF_DEV_PROC(vkDestroyCommandPool);
	DEF_DEV_PROC(vkDestroyDescriptorPool);
	DEF_DEV_PROC(vkDestroyDescriptorSetLayout);
	DEF_DEV_PROC(vkDestroyDevice);
	DEF_DEV_PROC(vkDestroyFence);
	DEF_DEV_PROC(vkDestroyFramebuffer);
	DEF_DEV_PROC(vkDestroyImageView);
	DEF_DEV_PROC(vkDestroyPipeline);
	DEF_DEV_PROC(vkDestroyPipelineLayout);
	DEF_DEV_PROC(vkDestroyQueryPool);
	DEF_DEV_PROC(vkDestroyRenderPass);
	DEF_DEV_PROC(vkDestroySemaphore);
	DEF_DEV_PROC(vkDestroyShaderModule);
	DEF_DEV_PROC(vkDestroySwapchainKHR);
	DEF_DEV_PROC(vkDeviceWaitIdle);
	DEF_DEV_PROC(vkEndCommandBuffer);
	DEF_DEV_PROC(vkFreeDescriptorSets);
	DEF_DEV_PROC(vkFreeMemory);
	DEF_DEV_PROC(vkGetBufferMemoryRequirements);
	DEF_DEV_PROC(vkGetDeviceQueue);
	DEF_DEV_PROC(vkGetQueryPoolResults);
	DEF_DEV_PROC(vkGetSwapchainImagesKHR);
	DEF_DEV_PROC(vkMapMemory);
	DEF_DEV_PROC(vkQueuePresentKHR);
	DEF_DEV_PROC(vkQueueSubmit);
	DEF_DEV_PROC(vkResetCommandBuffer);
	DEF_DEV_PROC(vkResetCommandPool);
	DEF_DEV_PROC(vkResetDescriptorPool);
	DEF_DEV_PROC(vkResetFences);
	DEF_DEV_PROC(vkUnmapMemory);
	DEF_DEV_PROC(vkUpdateDescriptorSets);
	DEF_DEV_PROC(vkWaitForFences);

#ifdef HAVE_XCB
	DEF_INST_PROC(vkCreateXcbSurfaceKHR);
#endif
#ifdef HAVE_WAYLAND
	DEF_INST_PROC(vkCreateWaylandSurfaceKHR);
#endif
};

extern struct vkapi vkapi;

struct plat_surface;

/* create Vulkan API instance */
int vkapi_init_instance(const char * app_name);

/* create Vulkan device, capable displaying to the surface */
int vkapi_init_device(struct plat_surface * surface);

/* destroy Vulkan device */
void vkapi_finish_device(void);

/* destroy the remaining Vulkan API objects */
void vkapi_finish(void);

#endif
