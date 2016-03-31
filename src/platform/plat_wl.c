#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <wayland-client.h>
#include <wayland-server.h>
#include <wayland-client-protocol.h>

#include "vkapi.h"
#include "plat_wl.h"
#include "surface.h"
#include "main.h"

struct plat_wl_surface {
	struct plat_surface plat_surface;

	struct wl_display * display;
	struct wl_compositor * compositor;
	struct wl_surface * surface;
	struct wl_shell * shell;
	struct wl_shell_surface * shell_surface;
};

static void global_registry_handler(void *data, struct wl_registry *registry, uint32_t id, const char *interface, uint32_t version) {

	struct plat_wl_surface * surf = (struct plat_wl_surface *) data;

	if (strcmp(interface, "wl_compositor") == 0) {
		surf->compositor = wl_registry_bind(registry, id, &wl_compositor_interface, 1);
		fprintf(stderr, "Got compositor: %p (id: %d)\n", (void *)surf->compositor, id);
	} else if (strcmp(interface, "wl_shell") == 0) {
		surf->shell = wl_registry_bind(registry, id, &wl_shell_interface, 1);
		fprintf(stderr, "Got shell: %p (id: %d)\n", (void *)surf->shell, id);
	}
}

static void global_registry_remover(void *data, struct wl_registry *registry, uint32_t id) {
	fprintf(stderr, "Got a registry losing event for %d\n", id);
}

static const struct wl_registry_listener registry_listener = {
	global_registry_handler,
	global_registry_remover
};

struct plat_surface* plat_wl_get_surface(void) {

	struct plat_wl_surface * surf = calloc(1, sizeof(struct plat_wl_surface));

	surf->display = wl_display_connect(NULL);
	if (surf->display == NULL) {
		fprintf(stderr, "Can't connect to Wayland display\n");
		goto error;
	}

	struct wl_registry *registry = wl_display_get_registry(surf->display);
	wl_registry_add_listener(registry, &registry_listener, surf);

	wl_display_dispatch(surf->display);
	wl_display_roundtrip(surf->display);

	if (surf->compositor == NULL) {
		fprintf(stderr, "Can't find Wayland compositor\n");
		goto error;
	}

	surf->surface = wl_compositor_create_surface(surf->compositor);
	if (surf->surface == NULL) {
		fprintf(stderr, "Can't create Wayland surface\n");
		goto error;
	}

	surf->shell_surface = wl_shell_get_shell_surface(surf->shell, surf->surface);
	if (surf->shell_surface == NULL) {
		fprintf(stderr, "Can't create Wayland shell surface\n");
		goto error;
	}
	wl_shell_surface_set_toplevel(surf->shell_surface);

	VkWaylandSurfaceCreateInfoKHR surf_ci = {
		.sType=VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
		.display=surf->display,
		.surface=surf->surface,
	};

	VkResult result = vkapi.vkCreateWaylandSurfaceKHR(vkapi.instance, &surf_ci, NULL, &surf->plat_surface.vk_surface);
	if (result != VK_SUCCESS) {
		printf("vkCreateWaylandSurfaceKHR failed: %i\n", result);
		goto error;
	}
	printf("vkCreateWaylandSurfaceKHR OK\n");

	surf->plat_surface.event_loop = plat_wl_event_loop;
	surf->plat_surface.destroy = plat_wl_destroy_surface;
	surf->plat_surface.width = options.win_width;
	surf->plat_surface.height = options.win_height;

	return (struct plat_surface *)surf;

error:
	if (surf) plat_wl_destroy_surface((struct plat_surface *)surf);
	return NULL;
}

static volatile int _plat_wl_signal_received = 0;

static void _plat_wl_sig_handler(int signum) {

	_plat_wl_signal_received = signum;
}

void plat_wl_event_loop(struct plat_surface *surf) {

	struct plat_wl_surface * wl_surf = (struct plat_wl_surface *)surf;

	signal(SIGINT, _plat_wl_sig_handler);

	while(!exit_requested() && !_plat_wl_signal_received) {
		if (wl_display_dispatch(wl_surf->display) < 0) {
			perror("Wayland main loop error");
			request_exit();
		}
	}

	printf("Finishing platform event loop (exit_requested=%i, signal_received=%i).\n", exit_requested(), _plat_wl_signal_received);
	if (_plat_wl_signal_received) {
		printf("Signal %i received\n", _plat_wl_signal_received);
	}
	request_exit();

	signal(SIGINT, SIG_DFL);
}

void plat_wl_destroy_surface(struct plat_surface *surf) {

	struct plat_wl_surface * wl_surf = (struct plat_wl_surface *)surf;

	finalize_surface(surf);

	if (wl_surf->shell_surface) wl_shell_surface_destroy(wl_surf->shell_surface);
	if (wl_surf->surface) wl_surface_destroy(wl_surf->surface);
	if (wl_surf->shell) wl_shell_destroy(wl_surf->shell);
	if (wl_surf->compositor) wl_compositor_destroy(wl_surf->compositor);
	if (wl_surf->display) wl_display_disconnect(wl_surf->display);

	free(surf);
}
