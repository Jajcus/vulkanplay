
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <alloca.h>
#include <pthread.h>
#include <assert.h>
#include <sys/time.h>

#include "printmath.h"

#include "vkapi.h"
#include "renderer.h"
#include "surface.h"
#include "main.h"

#include "scene.h"

struct framebuffer {

	VkImage image;
	VkImageView view;
	VkFramebuffer framebuffer;

	uint32_t width, height;

	VkQueryPool query_pool;
};

#define FRAME_LAG 2

struct renderer {
	struct plat_surface * surface;
	struct scene * scene;

	VkSwapchainKHR swapchain;
	VkImage* swapchain_images;
	uint32_t swapchain_image_count;

	VkSemaphore image_acquired_sem, rendering_complete_sem;
	VkFence frame_fences[FRAME_LAG];
	int frame_fences_ready[FRAME_LAG];

	VkExtent2D fb_extent;
	uint32_t fb_count;
	struct framebuffer * framebuffers;

	VkRenderPass render_pass;
	VkDescriptorPool descriptor_pool;

	VkPipeline pipeline;

	uint32_t materials_offset;
	uint32_t vertex_offset;
	uint32_t instance_offset;

	VkDeviceMemory memory;
	VkBuffer buffer;
	VkDescriptorSet descriptor_set;

	VkFence cmd_buf_fence;
	int cmd_buf_fence_ready;
	VkCommandPool command_pool;
	VkCommandBuffer command_buffer;

	VkShaderModule vs_module;
	VkShaderModule fs_module;
	VkPipelineLayout pipeline_layout;
	VkDescriptorSetLayout set_layout;

	unsigned char * mapped_memory;

	Mat4 p_matrix;
	Mat4 v_matrix;

	pthread_t thread;
	pthread_mutex_t mutex;
	bool stop; /* request to stop the rendering thread */
};

struct uniform_buffer {
	Vec4 light_pos;
	struct material materials[];
};

struct instance_data {
	Mat4 mv_matrix;
	Mat4 mvp_matrix;
	Mat4 normal_matrix;
};

extern const unsigned char main_frag_spv[];
extern unsigned int main_frag_spv_len;
extern const unsigned char main_vert_spv[];
extern unsigned int main_vert_spv_len;

void create_pipeline(struct renderer * renderer) {

	uint32_t i;

	vkapi.vkResetDescriptorPool(vkapi.device, renderer->descriptor_pool, 0);

	VkDescriptorSetLayoutBinding dsl_b[1] = {
		{
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		}
	};

	VkDescriptorSetLayoutCreateInfo dsl_ci = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = 1,
		.pBindings = dsl_b,
	};

	vkapi.vkCreateDescriptorSetLayout(vkapi.device, &dsl_ci, NULL, &renderer->set_layout);

	VkPipelineLayoutCreateInfo pipeline_layout_ci = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = 1,
		.pSetLayouts = &renderer->set_layout,
	};

	vkapi.vkCreatePipelineLayout(vkapi.device, &pipeline_layout_ci, NULL, &renderer->pipeline_layout);

	VkShaderModuleCreateInfo vs_module_ci = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = main_vert_spv_len,
		.pCode = (uint32_t *)main_vert_spv,
	};

	vkapi.vkCreateShaderModule(vkapi.device, &vs_module_ci, NULL, &renderer->vs_module);

	VkShaderModuleCreateInfo fs_module_ci = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = main_frag_spv_len,
		.pCode = (uint32_t *)main_frag_spv,
	};

	vkapi.vkCreateShaderModule(vkapi.device, &fs_module_ci, NULL, &renderer->fs_module);

	VkPipelineShaderStageCreateInfo shader_stage_ci[] = {
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = renderer->vs_module,
			.pName = "main",
		},
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = renderer->fs_module,
			.pName = "main",
		},
	};

	VkVertexInputBindingDescription vertex_binding_descr[] = {
		{
			.binding = 0,
			.stride = sizeof(struct vertex_data),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
		},
		{
			.binding = 1,
			.stride = sizeof(struct instance_data),
			.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE,
		},
	};

	VkVertexInputAttributeDescription vertex_attr_descr[15] = {
		// in_position
		{ .location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = 0, },
		// in_normal
		{ .location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = sizeof(Vec4), },
		// in_material
		{ .location = 2, .binding = 0, .format = VK_FORMAT_R32G32B32A32_UINT,   .offset = 2 * sizeof(Vec4), },

		// mv_matrix
		{ .location = 4, .binding = 1, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = 0, },
		{ .location = 5, .binding = 1, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = sizeof(Vec4), },
		{ .location = 6, .binding = 1, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = 2 * sizeof(Vec4), },
		{ .location = 7, .binding = 1, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = 3 * sizeof(Vec4), },

		// mvp_matrix
		{ .location = 8, .binding = 1, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = sizeof(Mat4), },
		{ .location = 9, .binding = 1, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = sizeof(Mat4) + sizeof(Vec4), },
		{ .location = 10, .binding = 1, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = sizeof(Mat4) + 2 * sizeof(Vec4), },
		{ .location = 11, .binding = 1, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = sizeof(Mat4) + 3 * sizeof(Vec4), },

		// normal_matrix
		{ .location = 12, .binding = 1, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = 2 * sizeof(Mat4), },
		{ .location = 13, .binding = 1, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = 2 * sizeof(Mat4) + sizeof(Vec4), },
		{ .location = 14, .binding = 1, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = 2 * sizeof(Mat4) + 2 * sizeof(Vec4), },
		{ .location = 15, .binding = 1, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = 2 * sizeof(Mat4) + 3 * sizeof(Vec4), },
	};

	VkPipelineVertexInputStateCreateInfo vis_ci = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 2,
		.pVertexBindingDescriptions = vertex_binding_descr,
		.vertexAttributeDescriptionCount = 15,
		.pVertexAttributeDescriptions = vertex_attr_descr,
	};

	VkPipelineInputAssemblyStateCreateInfo ias_ci = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE,
	};

	VkPipelineViewportStateCreateInfo vs_ci = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.scissorCount = 1,
	};

	VkPipelineRasterizationStateCreateInfo rs_ci = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.polygonMode = VK_POLYGON_MODE_FILL,
		//.polygonMode = VK_POLYGON_MODE_LINE,
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_CLOCKWISE,
	};

	VkPipelineMultisampleStateCreateInfo mss_ci = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
	};

	VkPipelineColorBlendAttachmentState cb_attachments[] = {
		{
			.colorWriteMask = VK_COLOR_COMPONENT_A_BIT | VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
		},
	};

	VkPipelineColorBlendStateCreateInfo cbs_ci = {
		.sType =  VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments = cb_attachments,
	};

	VkDynamicState dynamic_states[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
	};

	VkPipelineDynamicStateCreateInfo ds_ci = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = 2,
		.pDynamicStates = dynamic_states,
	};

	VkGraphicsPipelineCreateInfo pipeline_ci = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = 2,
		.pStages = shader_stage_ci,

		.pVertexInputState = &vis_ci,
		.pInputAssemblyState = &ias_ci,
		.pTessellationState = NULL,
		.pViewportState = &vs_ci,
		.pRasterizationState = &rs_ci,
		.pMultisampleState = &mss_ci,
		.pDepthStencilState = NULL,
		.pColorBlendState = &cbs_ci,
		.pDynamicState = &ds_ci,

		.layout = renderer->pipeline_layout,
		.renderPass = renderer->render_pass,
		.subpass = 0,
	};

	vkapi.vkCreateGraphicsPipelines(vkapi.device, (VkPipelineCache)VK_NULL_HANDLE, 1, &pipeline_ci, NULL, &renderer->pipeline);

	scene_lock(renderer->scene);

	uint32_t vertices_total = 0;
	uint32_t instances_total = 0;
	uint32_t indices_total = 0;
	for(i = 0; i < renderer->scene->objects_len; i++) {
		struct scene_object * obj = &renderer->scene->objects[i];
		struct model * model = obj->model;
		obj->r.vertex_index = vertices_total;
		obj->r.instance_index = instances_total;
		obj->r.index_index = indices_total;
		vertices_total += model->vertices_len;
		instances_total += 1;
		indices_total += model->indices_len;
	}

	assert(indices_total == 0); // FIXME

	renderer->materials_offset = sizeof(struct uniform_buffer);
	renderer->vertex_offset = renderer->materials_offset + sizeof(struct material) * renderer->scene->materials_len;
	renderer->instance_offset = renderer->vertex_offset + sizeof(struct vertex_data) * vertices_total;
	uint32_t mem_size = renderer->instance_offset + sizeof(struct instance_data) * instances_total;

	VkBufferCreateInfo buffer_ci = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = mem_size,
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	};

	vkapi.vkCreateBuffer(vkapi.device, &buffer_ci, NULL, &renderer->buffer);

	VkMemoryRequirements mem_req;

	vkapi.vkGetBufferMemoryRequirements(vkapi.device, renderer->buffer, &mem_req);

	for (i = 0; i < vkapi.memory_properties.memoryTypeCount; i++) {
		if ((vkapi.memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
			break;
		}
	}

	if (i == vkapi.memory_properties.memoryTypeCount) {
		fprintf(stderr, "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT memory not found!\n");
		i = 0;
	}

	VkMemoryAllocateInfo mem_ai = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = mem_req.size,
		.memoryTypeIndex = i,
	};

	vkapi.vkAllocateMemory(vkapi.device, &mem_ai, NULL, &renderer->memory);

	vkapi.vkMapMemory(vkapi.device, renderer->memory, 0, mem_size, 0, (void *)&renderer->mapped_memory);

	memcpy(renderer->mapped_memory + renderer->materials_offset,
		renderer->scene->materials,
		sizeof(struct material) * renderer->scene->materials_len
		);

	for(i = 0; i < renderer->scene->objects_len; i++) {
		struct scene_object * obj = &renderer->scene->objects[i];
		struct model * model = obj->model;

		memcpy(renderer->mapped_memory + renderer->vertex_offset
				+ obj->r.vertex_index * sizeof(struct vertex_data),
			model->vertices,
			model->vertices_len * sizeof(struct vertex_data)
			);
	}

	scene_unlock(renderer->scene);

	vkapi.vkBindBufferMemory(vkapi.device, renderer->buffer, renderer->memory, 0);

	VkDescriptorSetAllocateInfo ds_ai = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = renderer->descriptor_pool,
		.descriptorSetCount = 1,
		.pSetLayouts = &renderer->set_layout,
	};

	vkapi.vkAllocateDescriptorSets(vkapi.device, &ds_ai, &renderer->descriptor_set);

	VkDescriptorBufferInfo d_buffer_infos[] = {
		{
			.buffer = renderer->buffer,
			.offset = 0,
			.range = sizeof(struct uniform_buffer) + renderer->scene->materials_len * sizeof(struct material),
		}
	};

	VkWriteDescriptorSet w_descr_sets[] = {
		{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = renderer->descriptor_set,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.dstBinding = 0,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.pBufferInfo = d_buffer_infos,
		}
	};

	vkapi.vkUpdateDescriptorSets(vkapi.device, 1, w_descr_sets, 0, NULL);

	VkCommandPoolCreateInfo cmd_pool_ci = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = vkapi.g_queue_family,
	};

	vkapi.vkCreateCommandPool(vkapi.device, &cmd_pool_ci, NULL, &renderer->command_pool);

	VkFenceCreateInfo fence_ci = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO
	};

	vkapi.vkCreateFence(vkapi.device, &fence_ci, NULL, &renderer->cmd_buf_fence);

	VkCommandBufferAllocateInfo cmd_buf_ai = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = renderer->command_pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};

	vkapi.vkAllocateCommandBuffers(vkapi.device, &cmd_buf_ai, &renderer->command_buffer);
}

void render_scene(struct renderer * renderer, uint32_t image_index) {

	VkResult result;
	struct uniform_buffer uniform_buffer;
	uint32_t i;
	struct framebuffer * fb = &renderer->framebuffers[image_index];

	Vec3 up = {0.0f, -1.0f, 0.0};

	renderer->p_matrix = mat4_perspective((float)deg_to_rad(45.0f), 1.0f, 1.0f, 100.0f);

	renderer->v_matrix = mat4_view(renderer->scene->eye_pos, renderer->scene->eye_dir, up);

	for(i = 0; i < renderer->scene->objects_len; i++) {
		struct scene_object * obj = &renderer->scene->objects[i];
		struct instance_data * inst = (struct instance_data *)(
			renderer->mapped_memory + renderer->instance_offset +
			obj->r.instance_index * sizeof(struct instance_data));

		inst->mv_matrix = mat4_mul(renderer->v_matrix, obj->model_matrix);
		inst->mvp_matrix = mat4_mul(renderer->p_matrix, inst->mv_matrix);

		Mat4 imv_matrix = mat4_invert(inst->mv_matrix);
		inst->normal_matrix = mat4_transpose(imv_matrix);
	}

	uniform_buffer.light_pos = renderer->scene->light_pos;

	memcpy(renderer->mapped_memory, &uniform_buffer, sizeof(uniform_buffer));

	if (renderer->cmd_buf_fence_ready) {
		vkapi.vkWaitForFences(vkapi.device, 1, &renderer->cmd_buf_fence, VK_TRUE, UINT64_MAX);
		vkapi.vkResetFences(vkapi.device, 1, &renderer->cmd_buf_fence);
	}

	VkCommandBuffer cmd_buffer = renderer->command_buffer;
	vkapi.vkResetCommandBuffer(cmd_buffer, 0);

	VkCommandBufferBeginInfo cmd_buf_bi = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	};

	vkapi.vkBeginCommandBuffer(cmd_buffer, &cmd_buf_bi);

	int same_queue = vkapi.p_queue_family == vkapi.g_queue_family;
	const VkImageMemoryBarrier acquire_image_b = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.srcQueueFamilyIndex = same_queue ? VK_QUEUE_FAMILY_IGNORED : vkapi.p_queue_family,
		.dstQueueFamilyIndex = same_queue ? VK_QUEUE_FAMILY_IGNORED : vkapi.g_queue_family,
		.image = renderer->swapchain_images[image_index],
		.subresourceRange =  {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.levelCount = 1,
			.layerCount = 1,
			},
	};

	vkapi.vkCmdPipelineBarrier(cmd_buffer,
					VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					VK_DEPENDENCY_BY_REGION_BIT,
					0, NULL, 0, NULL,
					1, &acquire_image_b);

	if (fb->query_pool) {
		vkapi.vkCmdResetQueryPool(cmd_buffer, fb->query_pool, 0, 1);
		vkapi.vkCmdBeginQuery(cmd_buffer, fb->query_pool, 0, 0);
	}

	VkViewport viewports[] = {
		{
			.x = 0,
			.y = 0,
			.width = fb->width,
			.height = fb->height,
			.minDepth = 0.0f,
			.maxDepth = 1.0f,
		}
	};

	vkapi.vkCmdSetViewport(cmd_buffer, 0, 1, viewports);

	VkRect2D scissors[] = {
		{
			.offset = { .x = 0, .y = 0 },
			.extent = { .width = fb->width, .height = fb->height },
		}
	};

	vkapi.vkCmdSetScissor(cmd_buffer, 0, 1, scissors);

	VkClearValue clear_values[] = {
		{ .color = { .float32 = { 0.0f, 0.0f, 0.5f, 1.0f } } }
	};

	VkRenderPassBeginInfo render_pass_bi = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = renderer->render_pass,
		.framebuffer = fb->framebuffer,
		.renderArea = { { 0, 0 }, { fb->width, fb->height } },
		.clearValueCount = 1,
		.pClearValues = clear_values,
	};

	vkapi.vkCmdBeginRenderPass(cmd_buffer, &render_pass_bi, VK_SUBPASS_CONTENTS_INLINE);

	const const VkPipelineStageFlags dst_s_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	const VkSubmitInfo submits[] = {
		{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &renderer->image_acquired_sem,
		.pWaitDstStageMask = &dst_s_mask,
		.commandBufferCount = 1,
		.pCommandBuffers = &cmd_buffer,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &renderer->rendering_complete_sem,
		},
	};

	VkBuffer buffers[] = { renderer->buffer, renderer->buffer };
	VkDeviceSize offsets[] = { renderer->vertex_offset, renderer->instance_offset };

	vkapi.vkCmdBindVertexBuffers(cmd_buffer, 0, 2, buffers, offsets);

	vkapi.vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipeline);

	vkapi.vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipeline_layout,
				0, 1, &renderer->descriptor_set, 0, NULL);

	for(i = 0; i < renderer->scene->objects_len; i++) {
		struct scene_object * obj = &renderer->scene->objects[i];
		struct model * mod = obj->model;

		vkapi.vkCmdDraw(cmd_buffer, mod->vertices_len, 1, obj->r.vertex_index, obj->r.instance_index);
	}

	vkapi.vkCmdEndRenderPass(cmd_buffer);

	if (fb->query_pool) {
		vkapi.vkCmdEndQuery(cmd_buffer, fb->query_pool, 0);
	}

	const VkImageMemoryBarrier release_image_b = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		.srcQueueFamilyIndex = same_queue ? VK_QUEUE_FAMILY_IGNORED : vkapi.g_queue_family,
		.dstQueueFamilyIndex = same_queue ? VK_QUEUE_FAMILY_IGNORED : vkapi.p_queue_family,
		.image = renderer->swapchain_images[image_index],
		.subresourceRange =  {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.levelCount = 1,
			.layerCount = 1,
			},
	};

	vkapi.vkCmdPipelineBarrier(cmd_buffer,
					VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
					VK_DEPENDENCY_BY_REGION_BIT,
					0, NULL, 0, NULL,
					1, &release_image_b);

	vkapi.vkEndCommandBuffer(cmd_buffer);

	result = vkapi.vkQueueSubmit(vkapi.g_queue, 1, submits, renderer->cmd_buf_fence);
	if (result != VK_SUCCESS) {
		fprintf(stderr, "vkQueueSubmit failed: %i\n", result);
	}
	renderer->cmd_buf_fence_ready = 1;
}

void destroy_pipeline(struct renderer * renderer) {

	if (renderer->command_pool) vkapi.vkDestroyCommandPool(vkapi.device, renderer->command_pool, NULL);
	renderer->command_pool = NULL;
	if (renderer->buffer) vkapi.vkDestroyBuffer(vkapi.device, renderer->buffer, NULL);
	renderer->buffer = NULL;
	if (renderer->memory) {
		vkapi.vkUnmapMemory(vkapi.device, renderer->memory);
		vkapi.vkFreeMemory(vkapi.device, renderer->memory, NULL);
	}
	renderer->memory = NULL;
	renderer->mapped_memory = NULL;
	if (renderer->pipeline) vkapi.vkDestroyPipeline(vkapi.device, renderer->pipeline, NULL);
	renderer->pipeline = NULL;
	if (renderer->vs_module) vkapi.vkDestroyShaderModule(vkapi.device, renderer->vs_module, NULL);
	renderer->vs_module = NULL;
	if (renderer->fs_module) vkapi.vkDestroyShaderModule(vkapi.device, renderer->fs_module, NULL);
	renderer->fs_module = NULL;
	if (renderer->pipeline_layout) vkapi.vkDestroyPipelineLayout(vkapi.device, renderer->pipeline_layout, NULL);
	renderer->pipeline_layout = NULL;
	if (renderer->set_layout) vkapi.vkDestroyDescriptorSetLayout(vkapi.device, renderer->set_layout, NULL);
	renderer->set_layout = NULL;
	if (renderer->cmd_buf_fence) vkapi.vkDestroyFence(vkapi.device, renderer->cmd_buf_fence, NULL);
	renderer->cmd_buf_fence = NULL;
}

VkResult render_init(struct renderer * renderer) {

	VkResult result;

	VkAttachmentDescription attachments[] = {
		{
			.format = renderer->surface->s_format,
			.samples = 1,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		},
	};

	VkAttachmentReference color_attachments[] = {
		{
			.attachment = 0,
			.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		},
	};

	VkAttachmentReference resolve_attachments[] = {
		{
			.attachment = VK_ATTACHMENT_UNUSED,
			.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		},
	};

	uint32_t preserved_attachments[1] = { 0 };

	VkSubpassDescription subpasses[] = {
		{
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.inputAttachmentCount = 0,
			.colorAttachmentCount = 1,
			.pColorAttachments = color_attachments,
			.pResolveAttachments = resolve_attachments,
			.preserveAttachmentCount = 1,
			.pPreserveAttachments = preserved_attachments,
		},
	};

	VkRenderPassCreateInfo render_pass_ci = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments = attachments,
		.subpassCount = 1,
		.pSubpasses = subpasses,
	};

	result = vkapi.vkCreateRenderPass(vkapi.device, &render_pass_ci, NULL, &renderer->render_pass);
	if (result != VK_SUCCESS) {
		printf("vkCreateRenderPass failed: %i", result);
		goto error;
	}

	VkDescriptorPoolSize dpool_sizes[] = {
		{
			.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 5,
		},
	};

	VkDescriptorPoolCreateInfo dpool_ci = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.maxSets = 10,
		.poolSizeCount = 1,
		.pPoolSizes = dpool_sizes,
	};

	result = vkapi.vkCreateDescriptorPool(vkapi.device, &dpool_ci, NULL, &renderer->descriptor_pool);
	if (result != VK_SUCCESS) {
		printf("vkCreateDescriptorPool failed: %i", result);
		goto error;
	}
	return VK_SUCCESS;
error:
	if (renderer->descriptor_pool) vkapi.vkDestroyDescriptorPool(vkapi.device, renderer->descriptor_pool, NULL);
	if (renderer->render_pass) vkapi.vkDestroyRenderPass(vkapi.device, renderer->render_pass, NULL);

	return VK_ERROR_INITIALIZATION_FAILED;
}

void render_deinit(struct renderer * renderer) {

	if (renderer->descriptor_pool) vkapi.vkDestroyDescriptorPool(vkapi.device, renderer->descriptor_pool, NULL);
	renderer->descriptor_pool = NULL;
	if (renderer->render_pass) vkapi.vkDestroyRenderPass(vkapi.device, renderer->render_pass, NULL);
	renderer->render_pass = NULL;
}

static uint32_t create_swapchain(struct renderer *renderer) {

	VkResult result;
	int i;
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	VkExtent2D extent = {0};
	VkImage* swapchain_images = NULL;
	VkSurfaceCapabilitiesKHR s_caps;

	struct plat_surface * surface = renderer->surface;

	// get current surface size
	result = vkapi.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkapi.physical_device, surface->vk_surface, &s_caps);
	if (result != VK_SUCCESS) {
		fprintf(stderr, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed: %i\n", result);
		goto error;
	}

        if (s_caps.currentExtent.width == -1) {
            extent.width = surface->width;
            extent.height = surface->height;

            if (extent.width < s_caps.minImageExtent.width)
                extent.width = s_caps.minImageExtent.width;
            else if (extent.width > s_caps.maxImageExtent.width)
                extent.width = s_caps.maxImageExtent.width;

            if (extent.height < s_caps.minImageExtent.height)
                extent.height = s_caps.minImageExtent.height;
            else if (extent.height > s_caps.maxImageExtent.height)
                extent.height = s_caps.maxImageExtent.height;
        }
        else {
            extent = s_caps.currentExtent;
        }

	VkPresentModeKHR mode = options.pres_mode;
	if (mode == -1) {
		mode = VK_PRESENT_MODE_FIFO_KHR;
		for (i = 0; i < surface->s_modes_count; i++) {
			if (surface->s_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
				mode = VK_PRESENT_MODE_MAILBOX_KHR;
				break;
			}
		}
	}
	printf("Using present mode: %i\n", mode);

	VkSwapchainCreateInfoKHR swapchain_ci = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = surface->vk_surface,
		.minImageCount = s_caps.minImageCount + 2,
		.imageFormat = surface->s_format,
		.imageColorSpace = surface->s_colorspace,
		.imageExtent = extent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.preTransform = s_caps.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = mode,
		.oldSwapchain = renderer->swapchain,
	};

	if (vkapi.g_queue_family != vkapi.p_queue_family)
		swapchain_ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;

	result = vkapi.vkCreateSwapchainKHR(vkapi.device, &swapchain_ci, NULL, &swapchain);
	if (result != VK_SUCCESS) {
		fprintf(stderr, "vkCreateSwapchainKHR failed: %i\n", result);
		goto error;
	}

	if (renderer->swapchain) vkapi.vkDestroySwapchainKHR(vkapi.device, renderer->swapchain, NULL);
	renderer->swapchain = VK_NULL_HANDLE;
	if (renderer->swapchain_images) free(renderer->swapchain_images);
	renderer->swapchain_images = NULL;

	uint32_t image_count = 0;
	vkapi.vkGetSwapchainImagesKHR(vkapi.device, swapchain, &image_count, NULL);

	fprintf(stderr, "Available %u images\n", image_count);

	swapchain_images = calloc(image_count, sizeof(VkImage));
	vkapi.vkGetSwapchainImagesKHR(vkapi.device, swapchain, &image_count, swapchain_images);

	renderer->swapchain_images = swapchain_images;
	renderer->fb_extent = extent;
	renderer->swapchain = swapchain;
	renderer->swapchain_image_count = image_count;

	const VkSemaphoreCreateInfo sem_ci = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	};

	if (renderer->image_acquired_sem) {
		vkapi.vkDestroySemaphore(vkapi.device, renderer->image_acquired_sem, NULL);
		renderer->image_acquired_sem = VK_NULL_HANDLE;
	}
	result = vkapi.vkCreateSemaphore(vkapi.device, &sem_ci, NULL, &renderer->image_acquired_sem);
	if (result != VK_SUCCESS) {
		fprintf(stderr, "vkCreateSemaphore failed: %i\n", result);
		goto error;
	}
	if (renderer->rendering_complete_sem) {
		vkapi.vkDestroySemaphore(vkapi.device, renderer->rendering_complete_sem, NULL);
		renderer->rendering_complete_sem = VK_NULL_HANDLE;
	}
	result = vkapi.vkCreateSemaphore(vkapi.device, &sem_ci, NULL, &renderer->rendering_complete_sem);
	if (result != VK_SUCCESS) {
		fprintf(stderr, "vkCreateSemaphore failed: %i\n", result);
		goto error;
	}

	return image_count;
error:
	if (renderer->image_acquired_sem) {
		vkapi.vkDestroySemaphore(vkapi.device, renderer->image_acquired_sem, NULL);
		renderer->image_acquired_sem = VK_NULL_HANDLE;
	}
	if (renderer->rendering_complete_sem) {
		vkapi.vkDestroySemaphore(vkapi.device, renderer->rendering_complete_sem, NULL);
		renderer->rendering_complete_sem = VK_NULL_HANDLE;
	}
	if (swapchain) vkapi.vkDestroySwapchainKHR(vkapi.device, swapchain, NULL);
	if (renderer->swapchain) {
		vkapi.vkDestroySwapchainKHR(vkapi.device, renderer->swapchain, NULL);
		renderer->swapchain = VK_NULL_HANDLE;
	}
	if (swapchain_images) free(swapchain_images);
	if (renderer->swapchain_images) {
		free(renderer->swapchain_images);
		renderer->swapchain_images = NULL;
		renderer->swapchain_image_count = 0;
	}
	return 0;
}

static void destroy_swapchain(struct renderer * renderer) {

	if (renderer->image_acquired_sem) {
		vkapi.vkDestroySemaphore(vkapi.device, renderer->image_acquired_sem, NULL);
		renderer->image_acquired_sem = VK_NULL_HANDLE;
	}
	if (renderer->rendering_complete_sem) {
		vkapi.vkDestroySemaphore(vkapi.device, renderer->rendering_complete_sem, NULL);
		renderer->rendering_complete_sem = VK_NULL_HANDLE;
	}
	if (renderer->swapchain) {
		vkapi.vkDestroySwapchainKHR(vkapi.device, renderer->swapchain, NULL);
		renderer->swapchain = VK_NULL_HANDLE;
	}
	if (renderer->swapchain_images) {
		free(renderer->swapchain_images);
		renderer->swapchain_images = NULL;
		renderer->swapchain_image_count = 0;
	}
}

static void destroy_framebuffers(struct renderer * renderer) {

	uint32_t i;
	struct framebuffer * framebuffers = renderer->framebuffers;
	if (framebuffers) {
		for(i = 0; i < renderer->fb_count; i++) {
			if (framebuffers[i].framebuffer) {
				vkapi.vkDestroyFramebuffer(vkapi.device, framebuffers[i].framebuffer, NULL);
			}
			if (framebuffers[i].view) {
				vkapi.vkDestroyImageView(vkapi.device, framebuffers[i].view, NULL);
			}
			if (framebuffers[i].query_pool) {
				vkapi.vkDestroyQueryPool(vkapi.device, framebuffers[i].query_pool, NULL);
			}
		}
		free(framebuffers);
	}
	renderer->framebuffers = NULL;
}

static struct framebuffer * create_framebuffers(struct renderer * renderer) {

	uint32_t i;
	VkResult result;

	struct framebuffer * framebuffers = calloc(renderer->swapchain_image_count, sizeof(struct framebuffer));
	struct plat_surface * surface = renderer->surface;

	renderer->framebuffers = framebuffers;

	struct VkImageViewCreateInfo iv_ci = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = surface->s_format,
		.components = {0},
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.levelCount = 1,
			.layerCount = 1,
			},
	};
	struct VkFramebufferCreateInfo fb_ci = {
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		.renderPass = renderer->render_pass,
		.attachmentCount = 1,
		.width = renderer->fb_extent.width,
		.height = renderer->fb_extent.height,
		.layers = 1,
	};
	VkQueryPoolCreateInfo qp_ci = {
		.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
		.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS,
		.queryCount = 1,
		.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT
				| VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT
				| VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT
				| VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT
				| VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT
				| VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT,
	};
	for(i = 0; i < renderer->swapchain_image_count; i++) {
		iv_ci.image = renderer->swapchain_images[i];
		result = vkapi.vkCreateImageView(vkapi.device, &iv_ci, NULL, &framebuffers[i].view);
		if (result != VK_SUCCESS) {
			fprintf(stderr, "vkCreateImageView failed: %i\n", result);
			goto error;
		}
		fb_ci.pAttachments = &framebuffers[i].view;
		result = vkapi.vkCreateFramebuffer(vkapi.device, &fb_ci, NULL, &framebuffers[i].framebuffer);
		if (result != VK_SUCCESS) {
			fprintf(stderr, "vkCreateFramebuffer failed: %i\n", result);
			goto error;
		}
		framebuffers[i].width = renderer->fb_extent.width;
		framebuffers[i].height = renderer->fb_extent.height;
		printf("framebuffers[%li] width: %llu\n", (long)i, (long long)framebuffers[i].width);
		if (options.stats) {
			result = vkapi.vkCreateQueryPool(vkapi.device, &qp_ci, NULL, &framebuffers[i].query_pool);
			if (result != VK_SUCCESS) framebuffers[i].query_pool = VK_NULL_HANDLE;
		}

	}
	renderer->fb_count = renderer->swapchain_image_count;
	return framebuffers;
error:
	destroy_framebuffers(renderer);
	return NULL;
}

void * render_loop(void * arg) {

	struct renderer * renderer = (struct renderer *) arg;

	VkResult result;
	uint32_t image_index, frame_index;
	int i;

	VkFenceCreateInfo fence_ci = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO
	};
	for(i = 0; i < FRAME_LAG; i++) {
		result = vkapi.vkCreateFence(vkapi.device, &fence_ci, NULL, &renderer->frame_fences[i]);
		if (result != VK_SUCCESS) {
			fprintf(stderr, "vkCreateFence failed: %i\n", result);
			goto finish;
		}
	}

	result = render_init(renderer);
	if (result != VK_SUCCESS) {
		goto finish;
	}

	int frames = 0;

	struct timeval last_tv, last_fps_tv, tv;
	gettimeofday(&last_tv, NULL);
	last_fps_tv = last_tv;

	float fps_cap_frame_time;

	if (options.fps_cap) {
		fps_cap_frame_time = 1.0f / (float)options.fps_cap;
	}
	else {
		fps_cap_frame_time = 0.0f;
	}

	frame_index = 0;

	create_pipeline(renderer);

	while(!exit_requested()) {
		pthread_mutex_lock(&renderer->mutex);
		bool stop = renderer->stop;
		pthread_mutex_unlock(&renderer->mutex);
		if (stop) break;

		if (!create_swapchain(renderer)) goto finish;

		if (!create_framebuffers(renderer)) goto finish;

		while(!exit_requested()) {
			pthread_mutex_lock(&renderer->mutex);
			bool stop = renderer->stop;
			pthread_mutex_unlock(&renderer->mutex);
			if (stop) break;

			if (renderer->frame_fences_ready[frame_index]) {
				// Ensure no more than FRAME_LAG presentations are outstanding
				vkapi.vkWaitForFences(vkapi.device, 1, &renderer->frame_fences[frame_index], VK_TRUE, UINT64_MAX);
				vkapi.vkResetFences(vkapi.device, 1, &renderer->frame_fences[frame_index]);
			}

			result = vkapi.vkAcquireNextImageKHR(vkapi.device,
							     renderer->swapchain,
							     50000000,
							     renderer->image_acquired_sem,
							     renderer->frame_fences[frame_index],
							     &image_index);
			renderer->frame_fences_ready[frame_index] = 1;
			if (result == VK_ERROR_OUT_OF_DATE_KHR) {
				fprintf(stderr, "swapchain out of date, breaking\n");
				break;
			}
			else if (result == VK_SUBOPTIMAL_KHR) {
				fprintf(stderr, "swapchain suboptimal, continuing\n");
			}
			else if (result == VK_TIMEOUT) {
				fprintf(stderr, "vkAcquireNextImageKHR timed out\n");
			}
			else if (result != VK_SUCCESS) {
				fprintf(stderr, "vkAcquireNextImageKHR failed: %i\n", result);
				goto finish;
			}
			render_scene(renderer, image_index);
			VkPresentInfoKHR pi = {
				.sType =  VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
				.swapchainCount = 1,
				.pSwapchains = &renderer->swapchain,
				.pImageIndices = &image_index,
				.waitSemaphoreCount = 1,
				.pWaitSemaphores = &renderer->rendering_complete_sem,
			};
			result = vkapi.vkQueuePresentKHR(vkapi.p_queue, &pi);
			if (result == VK_ERROR_OUT_OF_DATE_KHR) {
				fprintf(stderr, "swapchain out of date\n");
				break;
			}
			else if (result == VK_SUBOPTIMAL_KHR) {
				fprintf(stderr, "swapchain suboptimal, breaking\n");
				break;
			}
			else if (result != VK_SUCCESS) {
				fprintf(stderr, "vkQueuePresentKHR failed: %i\n", result);
				goto finish;
			}
			VkQueryPool qp = renderer->framebuffers[image_index].query_pool;
			if (qp) {
				uint64_t data[6];
				vkapi.vkGetQueryPoolResults(vkapi.device, qp, 0, 1, sizeof(data), data, sizeof(data),
								VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
				printf("input assembly vertices:    %5lli\n", (long long) data[0]);
				printf("input assembly primitives:  %5lli\n", (long long) data[1]);
				printf("vertex shader invocations:  %5lli\n", (long long) data[2]);
				printf("clipping invocations:       %5lli\n", (long long) data[3]);
				printf("clipping primitives:        %5lli\n", (long long) data[4]);
				printf("fragment shader invocations:%5lli\n", (long long) data[5]);
			}
			frame_index += 1;
			frame_index %= FRAME_LAG;
			frames++;
			gettimeofday(&tv, NULL);
			long seconds = tv.tv_sec - last_fps_tv.tv_sec;
			if (seconds > 10 || (frames > 50 && seconds > 1)) {
				float timedelta = tv.tv_sec - last_fps_tv.tv_sec;
				timedelta += (float)((int32_t)tv.tv_usec - (int32_t)last_fps_tv.tv_usec) / 1000000.0;
				printf("%5i frames in %5.2f s - %5.1f FPS\n", frames, timedelta, (float)frames / timedelta);
				last_fps_tv = tv;
				frames = 0;
			}
			if (fps_cap_frame_time) {
				float timedelta = tv.tv_sec - last_tv.tv_sec;
				timedelta += (float)((int32_t)tv.tv_usec - (int32_t)last_tv.tv_usec) / 1000000.0;
				if (timedelta < fps_cap_frame_time) {
					usleep((useconds_t)((fps_cap_frame_time - timedelta) * 1000000));
					gettimeofday(&last_tv, NULL);
				}
				else {
					last_tv = tv;
				}
			}
		}
		vkapi.vkDeviceWaitIdle(vkapi.device);
		destroy_framebuffers(renderer);
	}
finish:
	fprintf(stderr, "render thread cleaning up...\n");
	vkapi.vkDeviceWaitIdle(vkapi.device);
	destroy_framebuffers(renderer);
	destroy_swapchain(renderer);
	destroy_pipeline(renderer);
	render_deinit(renderer);
	if (renderer->image_acquired_sem) vkapi.vkDestroySemaphore(vkapi.device, renderer->image_acquired_sem, NULL);
	if (renderer->rendering_complete_sem) vkapi.vkDestroySemaphore(vkapi.device, renderer->rendering_complete_sem, NULL);
	for(i = 0; i < FRAME_LAG; i++) {
		if (renderer->frame_fences[i]) {
			vkapi.vkDestroyFence(vkapi.device, renderer->frame_fences[i], NULL);
		}
	}
	request_exit();
	return NULL;
}

struct renderer * start_renderer(struct plat_surface * surface, struct scene * scene) {

	struct renderer * renderer = calloc(1, sizeof(struct renderer));

	renderer->surface = surface;
	renderer->scene = scene;

	pthread_mutex_init(&renderer->mutex, NULL);
	pthread_create(&renderer->thread, NULL, render_loop, renderer);

	return renderer;
}

void stop_renderer(struct renderer * renderer) {

	if (!renderer) return;

	pthread_mutex_lock(&renderer->mutex);
	renderer->stop = true;
	pthread_mutex_unlock(&renderer->mutex);

	pthread_join(renderer->thread, NULL);

	free(renderer);
}
