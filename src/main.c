
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <alloca.h>
#include <pthread.h>

#include "main.h"
#include "vkapi.h"
#include "surface.h"
#include "renderer.h"
#include "scene.h"
#include "linalg.h"

#include "models/tetrahedron.h"

#ifdef HAVE_XCB
#include "platform/plat_xcb.h"
#endif

#ifdef HAVE_WAYLAND
#include "platform/plat_wl.h"
#endif

static pthread_mutex_t _exit_mutex = PTHREAD_MUTEX_INITIALIZER;
static int _exit_requested = 0;

struct options options = {
	.fullscreen = 0,
	.pres_mode = -1,
	.win_width = 500,
	.win_height = 500,
	.stats = 0,
	.fps_cap = 0,
};

void request_exit(void) {

	pthread_mutex_lock(&_exit_mutex);
	_exit_requested = 1;
	pthread_mutex_unlock(&_exit_mutex);
}
int exit_requested(void) {

	int exit_requested;
	pthread_mutex_lock(&_exit_mutex);
	exit_requested = _exit_requested;
	pthread_mutex_unlock(&_exit_mutex);
	return exit_requested;
}

void usage(FILE *fp, const char *name) {

	fprintf(fp, "\n"
"Usage:\n"
"\n"
"    %s [OPTIONS]\n"
"\n"
"Options:\n"
"    --help, -h                this message\n"
"    --fullscreen, -f          full-screen mode\n"
"    --vsync=MODE, -v MODE     presentation (vsync) mode.\n"
"    --stats, -s               show pipeline statistics\n"
"    --width=VALUE, -W VALUE   window width\n"
"    --height=VALUE, -H VALUE  window height\n"
"    --fps-cap=VALUE, -c VALUE FPS cap\n"
"\n", name);
}

void parse_args(int argc, char **argv) {

	int i;
	char * opt = NULL;
	char * arg = NULL;
	for(i = 1; i < argc; i++) {
		opt = argv[i];
		arg = NULL;
		if (opt[0] == '-' && opt[1] == '-') {
			char * p = strchr(opt, '=');
			if (p) {
				*p = '\000';
				arg = p + 1;
			}
		}
		else if (opt[0] != '-') {
			break;
		}
		if (!strcmp(opt, "-h") || !strcmp(opt, "--help")) {
			usage(stdout, argv[0]);
			exit(0);
		}
		else if (!strcmp(opt, "-f") || !strcmp(opt, "--fullscreen")) {
			options.fullscreen = 1;
		}
		else if (!strcmp(opt, "-v") || !strcmp(opt, "--vsync")) {
			if (!arg) {
				if (i < argc - 1) arg = argv[++i];
				else break;
			}
			int val = atoi(arg);
			options.pres_mode = val;
		}
		else if (!strcmp(opt, "-s") || !strcmp(opt, "--stats")) {
			options.stats = 1;
		}
		else if (!strcmp(opt, "-W") || !strcmp(opt, "--width")) {
			if (!arg) {
				if (i < argc - 1) arg = argv[++i];
				else break;
			}
			int val = atoi(arg);
			if (val <= 0) break;
			options.win_width = val;
		}
		else if (!strcmp(opt, "-H") || !strcmp(opt, "--height")) {
			if (!arg) {
				if (i < argc - 1) arg = argv[++i];
				else break;
			}
			int val = atoi(arg);
			if (val <= 0) break;
			options.win_height = val;
		}
		else if (!strcmp(opt, "-c") || !strcmp(opt, "--fps-cap")) {
			if (!arg) {
				if (i < argc - 1) arg = argv[++i];
				else break;
			}
			float val = atof(arg);
			if (val <= 0) break;
			options.fps_cap = val;
		}
		else break;
	}
	if (i < argc) {
		fprintf(stderr, "Invalid argument: %s %s\n", opt,
							arg ? arg : "");
		usage(stderr, argv[0]);
		exit(1);
	}
}

static struct scene * scene = NULL;
static Vec4 eye_base = {-0.5f, 1.0f, 6.0f, 1.0f};
static Vec3 look_at = {0.0f, 0.0f, 0.0f};
static float y_angle = 0, x_angle = 0;

void on_left_click(float x, float y) {

	printf("left click on (%f, %f)\n", x, y);
	if (x == 0) return;

	y_angle += 10.0 * x;
	x_angle += 10.0 * y;

	y_angle = fmod(y_angle, 360.0);
	x_angle = fmod(x_angle, 360.0);

	Vec34 eye;
        Mat4 rot1, rot2;

        rot1 = mat4_rotate_Y(MAT4_IDENTITY, (float)deg_to_rad((float)y_angle));
        rot2 = mat4_rotate_X(rot1, (float)deg_to_rad((float)x_angle));

	eye.v4 = mat4_mul_vec4(rot2, eye_base);

	scene_set_eye(scene, eye.v3, look_at);
}

int main(int argc, char **argv) {

	VkResult result;
	int exit_code = 2;
	struct plat_surface * surf = NULL;

	if (!linalg_sanity_ok()) {
		fprintf(stderr, "linmath.h structure sanity check failed!\n");
		abort();
	}

	parse_args(argc, argv);

	result = vkapi_init_instance("vulkan play");
	if (result != VK_SUCCESS) {
		goto finish;
	}

#ifdef HAVE_XCB
	if (!surf) surf = plat_xcb_get_surface();
#endif
#ifdef HAVE_WAYLAND
	if (!surf) surf = plat_wl_get_surface();
#endif

	if (!surf) {
		printf("Failed to create presentation surface.\n");
		goto finish;
	}

	result = vkapi_init_device(surf);
	if (result != VK_SUCCESS) {
		goto finish;
	}

	scene = create_scene();
	struct model * tetrahedron = create_tetrahedron(0);

	scene_add_object(scene, tetrahedron, MAT4_IDENTITY);

	struct renderer * renderer = start_renderer(surf, scene);
	if (!renderer) {
		goto finish;
	}

	surf->event_loop(surf);

	stop_renderer(renderer);

	exit_code = 0;
finish:
	if (vkapi.device) vkapi.vkDeviceWaitIdle(vkapi.device);

	vkapi_finish_device();

	if (scene) destroy_scene(scene);

	if (surf) surf->destroy(surf);

	vkapi_finish();

	return exit_code;
}
