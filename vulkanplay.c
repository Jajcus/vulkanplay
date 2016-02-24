
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <alloca.h>
#include <pthread.h>
#include <assert.h>

#include "linmath.h"
#include "printmath.h"

#include "vkapi.h"
#include "plat.h"

#ifdef HAVE_XCB
#include "plat_xcb.h"
#endif

struct framebuffer {

	VkImage image;
	VkImageView view;
	VkFramebuffer framebuffer;

	uint32_t width, height;

	VkQueryPool query_pool;
};

struct render_ctx {
	VkSurfaceKHR surface;
	VkRenderPass render_pass;
	VkDescriptorPool descriptor_pool;
} render_ctx;

/* tetrahedron vertices */
#define V1 { 1.0f,  1.0f,  1.0f, 1.0f}
#define V2 {-1.0f, -1.0f,  1.0f, 1.0f}
#define V3 {-1.0f,  1.0f, -1.0f, 1.0f}
#define V4 { 1.0f, -1.0f, -1.0f, 1.0f}

vec4 V1v = V1;
vec4 V2v = V2;
vec4 V3v = V3;
vec4 V4v = V4;

/* tetrahedron colors */
#define C1  {0.8f,  0.5f,  0.5f, 1.0f}
#define C2  {0.5f,  0.8f,  0.5f, 1.0f}
#define C3  {0.5f,  0.5f,  0.8f, 1.0f}
#define C4  {0.5f,  0.8f,  0.8f, 1.0f}

static const vec4 tetrahedron_vertices[] = {
	V1, V2, V4,
	V1, V3, V2,
	V1, V4, V3,
	V2, V3, V4,
};

static const vec4 tetrahedron_colors[] = {
	C1, C2, C4,
	C1, C3, C2,
	C1, C4, C3,
	C2, C3, C4,
};
const uint32_t tetrahedron_triangle_count = 4;

static const vec4 test_vertices[] = {
	{ 0.1, 0.1, 0.5, 1.0 },
	{ 0.9, 0.1, 0.5, 1.0 },
	{ 0.5, 0.9, 0.5, 1.0 },
};
static const vec4 test_colors[] = {
	{ 1.0, 0.0, 0.0, 1.0 },
	{ 0.0, 1.0, 0.0, 1.0 },
	{ 0.0, 0.0, 1.0, 1.0 },
};
const uint32_t test_triangle_count = 4;

struct uniform_buffer {
	mat4x4 mvp_matrix;
	mat4x4 mv_matrix;
	mat4x4 normal_matrix;
	vec4 light_pos;
};
	
struct model {
	mat4x4 p_matrix;
	mat4x4 v_matrix;
	mat4x4 m_matrix;

	VkPipeline pipeline;

	uint32_t vertex_offset;
	uint32_t normal_offset;
	uint32_t color_offset;

	VkDeviceMemory memory;
	VkBuffer buffer;
	VkDescriptorSet descriptor_set;
	VkCommandPool command_pool;

	VkShaderModule vs_module;
	VkShaderModule fs_module;
	VkPipelineLayout pipeline_layout;
	VkDescriptorSetLayout set_layout;

	void * mapped_memory;
} model = {};

extern const unsigned char shader_frag_spv[];
extern unsigned int shader_frag_spv_len;
extern const unsigned char shader_vert_spv[];
extern unsigned int shader_vert_spv_len;

void normal(vec4 result, const vec4 a, const vec4 b, const vec4 c) {

	vec4 U, V;

	U[0] = b[0] - a[0];
	U[1] = b[1] - a[1];
	U[2] = b[2] - a[2];

	V[0] = c[0] - a[0];
	V[1] = c[1] - a[1];
	V[2] = c[2] - a[2];

	result[0] = U[1]*V[2] - U[2]*V[1];
	result[1] = U[2]*V[0] - U[0]*V[2];
	result[2] = U[0]*V[1] - U[1]*V[0];
	result[3] = 0;
}

void init_model(struct framebuffer * fb) {

	uint32_t i;
	printf("i"); fflush(stdout);	

	vkapi.vkResetDescriptorPool(vkapi.device, render_ctx.descriptor_pool, 0);

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


	vkapi.vkCreateDescriptorSetLayout(vkapi.device, &dsl_ci, NULL, &model.set_layout);

	VkPipelineLayoutCreateInfo pipeline_layout_ci = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = 1,
		.pSetLayouts = &model.set_layout,
	};
	
	vkapi.vkCreatePipelineLayout(vkapi.device, &pipeline_layout_ci, NULL, &model.pipeline_layout);

	VkShaderModuleCreateInfo vs_module_ci = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = shader_vert_spv_len,
		.pCode = (uint32_t *)shader_vert_spv,
	};
	
	vkapi.vkCreateShaderModule(vkapi.device, &vs_module_ci, NULL, &model.vs_module);

	VkShaderModuleCreateInfo fs_module_ci = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = shader_frag_spv_len,
		.pCode = (uint32_t *)shader_frag_spv,
	};
	
	vkapi.vkCreateShaderModule(vkapi.device, &fs_module_ci, NULL, &model.fs_module);

	VkPipelineShaderStageCreateInfo shader_stage_ci[] = {
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = model.vs_module,
			.pName = "main",
		},
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = model.fs_module,
			.pName = "main",
		},
	};

	VkVertexInputBindingDescription vertex_binding_descr[] = {
		{
			.binding = 0,
			.stride = 4 * sizeof(float),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
		},
		{
			.binding = 1,
			.stride = 4 * sizeof(float),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
		},
		{
			.binding = 2,
			.stride = 4 * sizeof(float),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
		},
	};

	VkVertexInputAttributeDescription vertex_attr_descr[] = {
		{
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = 0,
		},
		{
			.location = 1,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = 0,
		},
		{
			.location = 2,
			.binding = 2,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = 0,
		}
	};

	VkPipelineVertexInputStateCreateInfo vis_ci = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 3,
		.pVertexBindingDescriptions = vertex_binding_descr,
		.vertexAttributeDescriptionCount = 3,
		.pVertexAttributeDescriptions = vertex_attr_descr,
	};

	VkPipelineInputAssemblyStateCreateInfo ias_ci = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE,
	};

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

	VkRect2D scissors[] = {
		{
			.offset = { .x = 0, .y = 0 },
			.extent = { .width = fb->width, .height = fb->height },
		}
	};

	VkPipelineViewportStateCreateInfo vs_ci = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.pViewports = viewports,
		.scissorCount = 1,
		.pScissors = scissors,
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
		.pDynamicState = NULL,

		.layout = model.pipeline_layout,
		.renderPass = render_ctx.render_pass,
		.subpass = 0,
	};

	
	vkapi.vkCreateGraphicsPipelines(vkapi.device, (VkPipelineCache)VK_NULL_HANDLE, 1, &pipeline_ci, NULL, &model.pipeline);

	// compute normals
	vec4 * normals = alloca(sizeof(tetrahedron_vertices));
	for(i = 0; i < sizeof(tetrahedron_vertices) / sizeof(vec4); i += 3) {
		vec4 t_normal;
		normal(t_normal,
				tetrahedron_vertices[i],
				tetrahedron_vertices[i + 1],
				tetrahedron_vertices[i + 2]);
		vec4_norm(normals[i], t_normal);
		normals[i][3] = 1.0f;
		memcpy(normals[i + 1], normals[i], sizeof(vec4));
		memcpy(normals[i + 2], normals[i], sizeof(vec4));
	}
	print_vec4("normals[0]", normals[0]);
	print_vec4("normals[1]", normals[1]);
	print_vec4("normals[2]", normals[2]);
	print_vec4("normals[3]", normals[3]);
	print_vec4("normals[4]", normals[4]);

	model.vertex_offset = sizeof(struct uniform_buffer);
	model.normal_offset = model.vertex_offset + sizeof(tetrahedron_vertices);
	model.color_offset = model.color_offset + sizeof(tetrahedron_vertices);
	uint32_t mem_size = model.color_offset + sizeof(tetrahedron_colors);

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
		.allocationSize = mem_size,
		.memoryTypeIndex = i,
	};

	vkapi.vkAllocateMemory(vkapi.device, &mem_ai, NULL, &model.memory);

	vkapi.vkMapMemory(vkapi.device, model.memory, 0, mem_size, 0, &model.mapped_memory);

	memcpy(model.mapped_memory + model.vertex_offset, tetrahedron_vertices, sizeof(tetrahedron_vertices));
	memcpy(model.mapped_memory + model.normal_offset, normals, sizeof(tetrahedron_vertices));
	memcpy(model.mapped_memory + model.color_offset, tetrahedron_colors, sizeof(tetrahedron_colors));

	VkBufferCreateInfo buffer_ci = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = mem_size,
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	};

	vkapi.vkCreateBuffer(vkapi.device, &buffer_ci, NULL, &model.buffer);

	vkBindBufferMemory(vkapi.device, model.buffer, model.memory, 0);

	VkDescriptorSetAllocateInfo ds_ai = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = render_ctx.descriptor_pool,
		.descriptorSetCount = 1,
		.pSetLayouts = &model.set_layout,
	};

	vkapi.vkAllocateDescriptorSets(vkapi.device, &ds_ai, &model.descriptor_set);

	VkDescriptorBufferInfo d_buffer_infos[] = {
		{
			.buffer = model.buffer,
			.offset = 0,
			.range = sizeof(struct uniform_buffer),
		}
	};

	VkWriteDescriptorSet w_descr_sets[] = {
		{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = model.descriptor_set,
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
		.queueFamilyIndex = vkapi.g_queue_family,
	};

	vkapi.vkCreateCommandPool(vkapi.device, &cmd_pool_ci, NULL, &model.command_pool);

	printf("I"); fflush(stdout);	
};

void render_model(struct framebuffer * fb, VkSemaphore wait_sem, VkSemaphore signal_sem) {

	VkResult result;
	struct uniform_buffer uniform_buffer;

	printf("r"); fflush(stdout);	
	vec3 origin = {0, 0, 0};
	vec3 up = {0.0f, -1.0f, 0.0};
	vec3 eye_base = {-0.5f, 1.0f, 6.0f};
	static int view_angle_y = 0, view_angle_x = 0;
	vec4 light_pos = { 2.0f,  2.0f, 10.0f, 1.0f };

	mat4x4_perspective(model.p_matrix, (float)degreesToRadians(45.0f), 1.0f, 1.0f, 100.0f);
	//print_mat4x4("p_matrix:", model.p_matrix);

	vec4 eye;
	mat4x4 identity, rot1, rot2;
	mat4x4_identity(identity);
	
	printf("angle: %i, %i\n", view_angle_y, view_angle_x);	
	mat4x4_rotate_Y(rot1, identity, (float)degreesToRadians((float)view_angle_y));
	mat4x4_rotate_X(rot2, rot1, (float)degreesToRadians((float)view_angle_y));
	print_mat4x4("rot:", rot2);

	mat4x4_mul_vec4(eye, rot2, eye_base);
	print_vec4("eye:", eye);

	mat4x4_look_at(model.v_matrix, eye, origin, up);
	//print_mat4x4("v_matrix:", model.v_matrix);

	mat4x4_identity(model.m_matrix);

	mat4x4_mul(uniform_buffer.mv_matrix, model.v_matrix, model.m_matrix);
	//print_mat4x4("base_mv_matrix:", base_mv_matrix);

	
	/*vec4 v, vv;
	mat4x4_mul_vec4(v, mv_matrix, V1v);
	vec4_scale(vv, v, 1/v[3]);
	print_vec4("V1:", vv);
	mat4x4_mul_vec4(v, mv_matrix, V2v);
	vec4_scale(vv, v, 1/v[3]);
	print_vec4("V2:", vv);
	mat4x4_mul_vec4(v, mv_matrix, V3v);
	vec4_scale(vv, v, 1/v[3]);
	print_vec4("V3:", vv); */

	view_angle_y += 1;
	view_angle_y = view_angle_y % 360;
	view_angle_x += 3;
	view_angle_x = view_angle_x % 360;
	
	mat4x4_mul(uniform_buffer.mvp_matrix, model.p_matrix, uniform_buffer.mv_matrix);
	memcpy(uniform_buffer.light_pos, light_pos, sizeof(vec4));
	//mat4x4_mul_vec4(uniform_buffer.light_pos, uniform_buffer.mvp_matrix, light_pos);
	// normal_matrix = transpose(inverse(mv_matrix));
	mat4x4 imv_matrix;
	mat4x4_invert(imv_matrix, uniform_buffer.mv_matrix);
	mat4x4_transpose(uniform_buffer.normal_matrix, imv_matrix);

	print_mat4x4("normal_matrix:", uniform_buffer.mvp_matrix);

	/*
	mat4x4_mul_vec4(v, uniform_buffer.mvp_matrix, V1v);
	vec4_scale(vv, v, 1/v[3]);
	print_vec4("V1:", vv);
	mat4x4_mul_vec4(v, uniform_buffer.mvp_matrix, V2v);
	vec4_scale(vv, v, 1/v[3]);
	print_vec4("V2:", vv);
	mat4x4_mul_vec4(v, uniform_buffer.mvp_matrix, V3v);
	vec4_scale(vv, v, 1/v[3]);
	print_vec4("V3:", vv);
	*/

	memcpy(model.mapped_memory, &uniform_buffer, sizeof(uniform_buffer));

	vkapi.vkResetCommandPool(vkapi.device, model.command_pool, 0);

	VkCommandBufferAllocateInfo cmd_buf_ai = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = model.command_pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};

	VkCommandBuffer cmd_buffer;

	vkapi.vkAllocateCommandBuffers(vkapi.device, &cmd_buf_ai, &cmd_buffer);

	VkCommandBufferBeginInfo cmd_buf_bi = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	};

	vkapi.vkBeginCommandBuffer(cmd_buffer, &cmd_buf_bi);

	if (fb->query_pool) {
		vkapi.vkCmdResetQueryPool(cmd_buffer, fb->query_pool, 0, 1);
		vkapi.vkCmdBeginQuery(cmd_buffer, fb->query_pool, 0, 0);
	}

	VkClearValue clear_values[] = {
		{ .color = { .float32 = { 0.0f, 0.0f, 0.5f, 1.0f } } }
	};

	VkRenderPassBeginInfo render_pass_bi = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = render_ctx.render_pass,
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
		.pWaitSemaphores = &wait_sem,
		.pWaitDstStageMask = &dst_s_mask,
		.commandBufferCount = 1,
		.pCommandBuffers = &cmd_buffer,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &signal_sem,
		},
	};

	VkBuffer buffers[] = { model.buffer, model.buffer, model.buffer };
	VkDeviceSize offsets[] = { model.vertex_offset, model.normal_offset, model.color_offset };

	vkapi.vkCmdBindVertexBuffers(cmd_buffer, 0, 3, buffers, offsets);

	vkapi.vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, model.pipeline);

	vkapi.vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, model.pipeline_layout,
				0, 1, &model.descriptor_set, 0, NULL);

	vkapi.vkCmdDraw(cmd_buffer, 3 * tetrahedron_triangle_count, 1, 0, 0);
	vkapi.vkCmdEndRenderPass(cmd_buffer);
	
	if (fb->query_pool) {
		vkapi.vkCmdEndQuery(cmd_buffer, fb->query_pool, 0);
	}

	vkapi.vkEndCommandBuffer(cmd_buffer);

	result = vkapi.vkQueueSubmit(vkapi.g_queue, 1, submits, VK_NULL_HANDLE);
	if (result != VK_SUCCESS) {
		fprintf(stderr, "vkQueueSubmit failed: %i\n", result);
	}

	printf("R"); fflush(stdout);	
};

void destroy_model(void) {

	printf("d"); fflush(stdout);	
	if (model.command_pool) vkapi.vkDestroyCommandPool(vkapi.device, model.command_pool, NULL);
	model.command_pool = NULL;
	if (model.buffer) vkapi.vkDestroyBuffer(vkapi.device, model.buffer, NULL);
	model.buffer = NULL;
	if (model.memory) {
		vkapi.vkUnmapMemory(vkapi.device, model.memory);
		vkapi.vkFreeMemory(vkapi.device, model.memory, NULL);
	}
	model.memory = NULL;
	model.mapped_memory = NULL;
	if (model.pipeline) vkapi.vkDestroyPipeline(vkapi.device, model.pipeline, NULL);
	model.pipeline = NULL;
	if (model.vs_module) vkapi.vkDestroyShaderModule(vkapi.device, model.vs_module, NULL);
	model.vs_module = NULL;
	if (model.fs_module) vkapi.vkDestroyShaderModule(vkapi.device, model.fs_module, NULL);
	model.fs_module = NULL;
	if (model.pipeline_layout) vkapi.vkDestroyPipelineLayout(vkapi.device, model.pipeline_layout, NULL);
	model.pipeline_layout = NULL;
	if (model.set_layout) vkapi.vkDestroyDescriptorSetLayout(vkapi.device, model.set_layout, NULL);
	model.set_layout = NULL;
	printf("D"); fflush(stdout);	
};

VkResult render_init(void) {

	VkResult result;

	VkAttachmentDescription attachments[] = {
		{
			.format = vkapi.s_format,
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

	result = vkCreateRenderPass(vkapi.device, &render_pass_ci, NULL, &render_ctx.render_pass);
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
	
	result = vkapi.vkCreateDescriptorPool(vkapi.device, &dpool_ci, NULL, &render_ctx.descriptor_pool);
	if (result != VK_SUCCESS) {
		printf("vkCreateDescriptorPool failed: %i", result);
		goto error;
	}
	return VK_SUCCESS;
error:
	if (render_ctx.descriptor_pool) vkDestroyDescriptorPool(vkapi.device, render_ctx.descriptor_pool, NULL);
	if (render_ctx.render_pass) vkDestroyRenderPass(vkapi.device, render_ctx.render_pass, NULL);

	return VK_ERROR_INITIALIZATION_FAILED;
}

void render_deinit(void) {
	
	if (render_ctx.descriptor_pool) vkDestroyDescriptorPool(vkapi.device, render_ctx.descriptor_pool, NULL);
	if (render_ctx.render_pass) vkDestroyRenderPass(vkapi.device, render_ctx.render_pass, NULL);
}

uint32_t create_swapchain(VkSwapchainKHR *swapchain_p, VkExtent2D *output_extent_p, VkImage** swapchain_images_p) {

	VkResult result;
	int i;
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	VkExtent2D extent = {};
	VkImage* swapchain_images = NULL;

	// get current surface size
	result = vkapi.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkapi.physical_device, vkapi.surface, &vkapi.s_caps);
	if (result != VK_SUCCESS) {
		fprintf(stderr, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed: %i\n", result);
		goto error;
	}

        if (vkapi.s_caps.currentExtent.width == -1) {
            extent.width = 500;
            extent.height = 500;

            if (extent.width < vkapi.s_caps.minImageExtent.width)
                extent.width = vkapi.s_caps.minImageExtent.width;
            else if (extent.width > vkapi.s_caps.maxImageExtent.width)
                extent.width = vkapi.s_caps.maxImageExtent.width;

            if (extent.height < vkapi.s_caps.minImageExtent.height)
                extent.height = vkapi.s_caps.minImageExtent.height;
            else if (extent.height > vkapi.s_caps.maxImageExtent.height)
                extent.height = vkapi.s_caps.maxImageExtent.height;
        }
        else {
            extent = vkapi.s_caps.currentExtent;
        }

	VkPresentModeKHR mode = VK_PRESENT_MODE_FIFO_KHR;
        for (i = 0; i < vkapi.s_modes_count; i++) {
		if (vkapi.s_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
			mode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
	}
	printf("Using present mode: %i\n", mode);

	VkSwapchainCreateInfoKHR swapchain_ci = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = vkapi.surface,
		.minImageCount = vkapi.s_caps.minImageCount + 2,
		.imageFormat = vkapi.s_format,
		.imageColorSpace = vkapi.s_colorspace,
		.imageExtent = extent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.preTransform = vkapi.s_caps.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		//.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
		.presentMode = mode,
		.oldSwapchain = *swapchain_p,
	};

	if (vkapi.g_queue_family != vkapi.p_queue_family)
		swapchain_ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
	
	result = vkapi.vkCreateSwapchainKHR(vkapi.device, &swapchain_ci, NULL, &swapchain);
	if (result != VK_SUCCESS) {
		fprintf(stderr, "vkCreateSwapchainKHR failed: %i\n", result);
		goto error;
	}
	
	if (*swapchain_p) vkapi.vkDestroySwapchainKHR(vkapi.device, *swapchain_p, NULL);
	*swapchain_p = VK_NULL_HANDLE;
	if (*swapchain_images_p) free(*swapchain_images_p);
	*swapchain_images_p = NULL;

	uint32_t image_count = 0;
	vkapi.vkGetSwapchainImagesKHR(vkapi.device, swapchain, &image_count, NULL);

	fprintf(stderr, "Available %u images\n", image_count);

	swapchain_images = calloc(image_count, sizeof(VkImage));
	vkapi.vkGetSwapchainImagesKHR(vkapi.device, swapchain, &image_count, swapchain_images);

	*swapchain_images_p = swapchain_images;
	*output_extent_p = extent;
	*swapchain_p = swapchain;
	
	return image_count;
error:
	if (swapchain) vkapi.vkDestroySwapchainKHR(vkapi.device, swapchain, NULL);
	if (swapchain_p && *swapchain_p) vkapi.vkDestroySwapchainKHR(vkapi.device, *swapchain_p, NULL);
	if (swapchain_images_p && *swapchain_images_p) free(*swapchain_images_p);
	return 0;
}

void destroy_framebuffers(uint32_t count, struct framebuffer * framebuffers) {

	uint32_t i;

	if (framebuffers) {
		for(i = 0; i < count; i++) {
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
};

struct framebuffer * create_framebuffers(uint32_t count, VkImage* images, VkExtent2D extent) {

	uint32_t i;
	VkResult result;

	struct framebuffer * framebuffers = calloc(count, sizeof(struct framebuffer));
	struct VkImageViewCreateInfo iv_ci = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = vkapi.s_format,
		.components = {},
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.levelCount = 1,
			.layerCount = 1,
			},
	};
	struct VkFramebufferCreateInfo fb_ci = {
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		.renderPass = render_ctx.render_pass,
		.attachmentCount = 1,
		.width = extent.width,
		.height = extent.height,
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
	for(i = 0; i < count; i++) {
		iv_ci.image = images[i];
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
		framebuffers[i].width = extent.width;
		framebuffers[i].height = extent.height;
		printf("framebuffers[%li] width: %llu\n", (long)i, (long long)framebuffers[i].width);
		if (vkapi.device_features.pipelineStatisticsQuery) {
			result = vkapi.vkCreateQueryPool(vkapi.device, &qp_ci, NULL, &framebuffers[i].query_pool);
			if (result != VK_SUCCESS) framebuffers[i].query_pool = VK_NULL_HANDLE;
		}

	}
	return framebuffers;
error:
	destroy_framebuffers(count, framebuffers);
	return NULL;
}

void * render_loop(void * argument) {

	VkResult result;
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	VkExtent2D output_extent = {};
	uint32_t image_count = 0;
	VkImage* swapchain_images = NULL;
	uint32_t image_index;
	struct framebuffer * framebuffers = NULL;
	VkSemaphore image_acquired_sem, rendering_complete_sem;

	const VkSemaphoreCreateInfo sem_ci = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	};
	result = vkapi.vkCreateSemaphore(vkapi.device, &sem_ci, NULL, &image_acquired_sem);
	if (result != VK_SUCCESS) {
		fprintf(stderr, "vkCreateSemaphore failed: %i\n", result);
		goto finish;
	}
	result = vkapi.vkCreateSemaphore(vkapi.device, &sem_ci, NULL, &rendering_complete_sem);
	if (result != VK_SUCCESS) {
		fprintf(stderr, "vkCreateSemaphore failed: %i\n", result);
		goto finish;
	}
	printf("image_acquired_sem = %lx, rendering_complete_sem = %lx\n",
			(long)image_acquired_sem, (long)rendering_complete_sem);

	result = render_init();
	if (result != VK_SUCCESS) {
		goto finish;
	}
	while(!exit_requested) {
		image_count = create_swapchain(&swapchain, &output_extent, &swapchain_images);
		if (image_count == 0) goto finish;

		framebuffers = create_framebuffers(image_count, swapchain_images, output_extent);
		if (!framebuffers) goto finish;
		
		while(!exit_requested) {
			result = vkapi.vkAcquireNextImageKHR(vkapi.device,
							     swapchain,
							     50000000,
							     image_acquired_sem,
							     VK_NULL_HANDLE,
							     &image_index);
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
			init_model(&framebuffers[image_index]);
			render_model(&framebuffers[image_index], image_acquired_sem, rendering_complete_sem);
			VkPresentInfoKHR pi = {
				.sType =  VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
				.swapchainCount = 1,
				.pSwapchains = &swapchain,
				.pImageIndices = &image_index,
				.waitSemaphoreCount = 1,
				.pWaitSemaphores = &rendering_complete_sem,
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
			vkapi.vkDeviceWaitIdle(vkapi.device);
			VkQueryPool qp = framebuffers[image_index].query_pool;
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

			destroy_model();
			usleep(100000);
		}
	}
	fprintf(stderr, "render_loop finished normally (exit_requested=%i)\n", exit_requested);
finish:
	fprintf(stderr, "render thread cleaning up...\n");
	vkDeviceWaitIdle(vkapi.device);
	destroy_model();
	render_deinit();
	vkapi.vkDestroySemaphore(vkapi.device, image_acquired_sem, NULL);
	vkapi.vkDestroySemaphore(vkapi.device, rendering_complete_sem, NULL);
	if (framebuffers) destroy_framebuffers(image_count, framebuffers);
	if (swapchain) vkapi.vkDestroySwapchainKHR(vkapi.device, swapchain, NULL);
	if (swapchain_images) free(swapchain_images);
	exit_requested = 1;
	return NULL;
}

int main(int argc, char **argv) {

	VkResult result;
	int exit_code = 2;
	struct plat_surface * surf = NULL;

	result = vkapi_init_instance("vulkan play");
	if (result != VK_SUCCESS) {
		goto finish;
	}

#ifdef HAVE_XCB
	if (!surf) surf = plat_xcb_get_surface();
#endif

	if (!surf) {
		printf("Failed to create presentation surface.\n");
		goto finish;
	}

	result = vkapi_init_device(surf->surface);
	if (result != VK_SUCCESS) {
		goto finish;
	}

	pthread_t render_thread;
	pthread_create(&render_thread, NULL, render_loop, NULL);

	surf->event_loop(surf);

	pthread_join(render_thread, NULL);
	
	exit_code = 0;

finish:
	if (vkapi.device) vkDeviceWaitIdle(vkapi.device);
	
	vkapi_finish_device();

	if (surf) surf->destroy(surf);

	vkapi_finish();

	return exit_code;
}
