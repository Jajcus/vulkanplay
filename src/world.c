#include "main.h"
#include "world.h"
#include "scene.h"
#include "input_callbacks.h"
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "models/plane.h"
#include "models/terrain.h"
#include "models/tetrahedron.h"
#include "models/sphere.h"
#include "printmath.h"

const double tick_length = 0.05;

const float move_rate = 10.0; // units per second
const float rotate_rate = 25.0; // degrees per second

#define IN_QUEUE_LEN 16

enum input_event_type {
	EVENT_NONE,
	EVENT_MOUSE_BUTTON_PRESS,
	EVENT_MOUSE_BUTTON_RELEASE,
	EVENT_MOUSE_MOVE,
	EVENT_KEY_PRESS,
	EVENT_KEY_RELEASE,
};

struct input_event {
	enum input_event_type type;
	double timestamp;
	float x, y;
	int buttons;
	int keycode;
};

struct input_event in_queue[IN_QUEUE_LEN];
int in_queue_head=0, in_queue_tail=0;
pthread_mutex_t in_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t in_queue_event = PTHREAD_COND_INITIALIZER;

#define in_queue_empty() (in_queue_head == in_queue_tail)
#define in_queue_full() (((in_queue_head + 1) % IN_QUEUE_LEN) == in_queue_tail)

struct world {
	struct scene * scene;
	struct model * terrain;

	// character position and direction
	Vec3 ch_position;
	float ch_direction; // angle around Y axis, in degrees

	Vec4 ch_movement;
	float ch_rotation;
	double moving_forward, moving_back, moving_left, moving_right;
	double turning_left, turning_right;

	double last_tick, next_tick;

	pthread_t thread;
	pthread_mutex_t mutex;
	bool stop; /* request to stop the rendering thread */
};

static inline double get_time(void) {

	struct timeval tv;

	gettimeofday(&tv, NULL);
	return (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0;
}

void on_mouse_button_press(float x, float y, int button) {

	pthread_mutex_lock(&in_queue_mutex);
	if (in_queue_full()) {
		fprintf(stderr, "dropping mouse button press event\n");
	}
	else {
		in_queue[in_queue_head].type = EVENT_MOUSE_BUTTON_PRESS;
		in_queue[in_queue_head].timestamp = get_time();
		in_queue[in_queue_head].x = x;
		in_queue[in_queue_head].y = y;
		in_queue[in_queue_head].buttons = button;
		in_queue_head = (in_queue_head + 1) % IN_QUEUE_LEN;
		pthread_cond_signal(&in_queue_event);
	}
	pthread_mutex_unlock(&in_queue_mutex);
}
void on_mouse_button_release(float x, float y, int button) {

	pthread_mutex_lock(&in_queue_mutex);
	if (in_queue_full()) {
		fprintf(stderr, "dropping mouse button release event\n");
	}
	else {
		in_queue[in_queue_head].type = EVENT_MOUSE_BUTTON_RELEASE;
		in_queue[in_queue_head].timestamp = get_time();
		in_queue[in_queue_head].x = x;
		in_queue[in_queue_head].y = y;
		in_queue[in_queue_head].buttons = button;
		in_queue_head = (in_queue_head + 1) % IN_QUEUE_LEN;
		pthread_cond_signal(&in_queue_event);
	}
	pthread_mutex_unlock(&in_queue_mutex);
}
void on_mouse_move(float x, float y, int buttons) {

	pthread_mutex_lock(&in_queue_mutex);
	if (in_queue_full()) {
		fprintf(stderr, "dropping mouse move event\n");
	}
	else {
		in_queue[in_queue_head].type = EVENT_MOUSE_MOVE;
		in_queue[in_queue_head].timestamp = get_time();
		in_queue[in_queue_head].x = x;
		in_queue[in_queue_head].y = y;
		in_queue[in_queue_head].buttons = buttons;
		in_queue_head = (in_queue_head + 1) % IN_QUEUE_LEN;
		pthread_cond_signal(&in_queue_event);
	}
	pthread_mutex_unlock(&in_queue_mutex);
}

void on_key_press(int keycode) {

	if (keycode == KEY_ESCAPE) {
		request_exit();
	}
	else if (keycode == KEY_NONE) {
		return;
	}
	pthread_mutex_lock(&in_queue_mutex);
	if (in_queue_full()) {
		fprintf(stderr, "dropping key press event\n");
	}
	else {
		in_queue[in_queue_head].type = EVENT_KEY_PRESS;
		in_queue[in_queue_head].timestamp = get_time();
		in_queue[in_queue_head].keycode = keycode;
		in_queue_head = (in_queue_head + 1) % IN_QUEUE_LEN;
		pthread_cond_signal(&in_queue_event);
	}
	pthread_mutex_unlock(&in_queue_mutex);

}
void on_key_release(int keycode) {

	if (keycode == KEY_NONE) {
		return;
	}
	pthread_mutex_lock(&in_queue_mutex);
	if (in_queue_full()) {
		fprintf(stderr, "dropping key release event\n");
	}
	else {
		in_queue[in_queue_head].type = EVENT_KEY_RELEASE;
		in_queue[in_queue_head].timestamp = get_time();
		in_queue[in_queue_head].keycode = keycode;
		in_queue_head = (in_queue_head + 1) % IN_QUEUE_LEN;
		pthread_cond_signal(&in_queue_event);
	}
	pthread_mutex_unlock(&in_queue_mutex);
}

static Vec3 make_direction_vector(float x_angle) {

	x_angle = deg_to_rad(x_angle);
	Vec3 result = { sinf(x_angle), 0.0, cosf(x_angle) };

	return result;
}

bool get_in_queue_event(struct input_event *event_p, double deadline) {

	bool got_event = false;
	struct timespec ts = {0};
	int r;

	if (deadline > 0) {
		ts.tv_sec = (time_t) deadline;
		ts.tv_nsec = (long) fmod(deadline, 1) * 1000000000;
	}

	pthread_mutex_lock(&in_queue_mutex);
	if (!in_queue_empty()) {
		got_event = true;
	}
	else {
		r = pthread_cond_timedwait(&in_queue_event, &in_queue_mutex, &ts);
		if (r == 0) {
			got_event = true;
		}
		else if (r && r!= ETIMEDOUT) {
			fprintf(stderr, "unexpected pthread_cond_timedwait() result: %i\n", r);
			request_exit();
		}
	}
	if (got_event) {
		*event_p = in_queue[in_queue_tail];
		in_queue_tail = (in_queue_tail + 1) % IN_QUEUE_LEN;
	}
	pthread_mutex_unlock(&in_queue_mutex);
	return got_event;
}

void world_process_key_event(struct world * world, struct input_event event) {

	double * key;
	float * var;
	float rate;
	switch(event.keycode) {
		case 'w':
		case KEY_UP:
			key = &world->moving_forward;
			rate = move_rate;
			var = &world->ch_movement.z;
			break;
		case 's':
		case KEY_DOWN:
			key = &world->moving_back;
			rate = -move_rate;
			var = &world->ch_movement.z;
			break;
		case 'd':
		case KEY_RIGHT:
			key = &world->moving_right;
			rate = move_rate;
			var = &world->ch_movement.x;
			break;
		case 'a':
		case KEY_LEFT:
			key = &world->moving_left;
			rate = -move_rate;
			var = &world->ch_movement.x;
			break;
		case '.':
			key = &world->turning_right;
			rate = rotate_rate;
			var = &world->ch_rotation;
			break;
		case ',':
			key = &world->turning_left;
			rate = -rotate_rate;
			var = &world->ch_rotation;
			break;
		default:
			return;
	}
	if (event.type == EVENT_KEY_PRESS) {
		if (!*key) *key = event.timestamp;
	}
	else if (*key) {
		if (event.timestamp > *key) *var += rate * (event.timestamp - *key);
		*key = 0.0f;
	}
}
void world_continue_movement(struct world * world, double now) {

	if (world->moving_forward && world->moving_forward < now) {
		world->ch_movement.z += move_rate * (now - world->moving_forward);
		world->moving_forward = now;
	}
	if (world->moving_back && world->moving_back < now) {
		world->ch_movement.z -= move_rate * (now - world->moving_back);
		world->moving_back = now;
	}
	if (world->moving_right && world->moving_right < now) {
		world->ch_movement.x += move_rate * (now - world->moving_right);
		world->moving_right = now;
	}
	if (world->moving_left && world->moving_left < now) {
		world->ch_movement.x -= move_rate * (now - world->moving_left);
		world->moving_left = now;
	}
	if (world->turning_right && world->turning_right < now) {
		world->ch_rotation += rotate_rate * (now - world->turning_right);
		world->turning_right = now;
	}
	if (world->turning_left && world->turning_left < now) {
		world->ch_rotation -= rotate_rate * (now - world->turning_left);
		world->turning_left = now;
	}
}

static const Vec4 zero_movement = {0.0f, 0.0f, 0.0f, 1.0f};

void * world_loop(void * arg) {

	struct world * world = (struct world *) arg;

	struct timeval tv;
	gettimeofday(&tv, NULL);

	double now = get_time();
	world->last_tick = now;

	while(!exit_requested()) {
		world->next_tick = world->last_tick + tick_length;
		pthread_mutex_lock(&world->mutex);
		bool stop = world->stop;
		pthread_mutex_unlock(&world->mutex);
		if (stop) break;

		world->ch_movement = zero_movement;
		world->ch_rotation = 0.0f;

		// process queued events
		do {
			struct input_event event;
			if (get_in_queue_event(&event, world->next_tick)) {
				switch(event.type) {
					case EVENT_KEY_PRESS:
					case EVENT_KEY_RELEASE:
						world_process_key_event(world, event);
						break;
					default:
						break;
				}
			}
			now = get_time();
		} while(now <= world->next_tick);

		world_continue_movement(world, now);

		//////////////////
		// transform world

		// apply movement
		Mat4 rot_matrix = mat4_rotate_Y(MAT4_IDENTITY, -deg_to_rad(world->ch_direction));
		Vec34 dir_movement;
		dir_movement.v4 = mat4_mul_vec4(rot_matrix, world->ch_movement);
		world->ch_position = vec3_add(world->ch_position, dir_movement.v3);
		world->ch_position.y = sample_terrain_height(world->terrain, world->ch_position.x, world->ch_position.z) + 2.0f;

		// apply rotation
		world->ch_direction += world->ch_rotation;

		// update scene eye
		scene_set_eye(world->scene, world->ch_position, make_direction_vector(world->ch_direction));

		world->last_tick = now;
	}

	return NULL;
}

const Vec3 initial_position = {0.0, 1.0, -10.0};
const double initial_direction = 0.0;

struct world * create_world(void) {

	struct world * world = (struct world *)calloc(1, sizeof(struct world));

	pthread_mutex_init(&world->mutex, NULL);

	world->ch_direction = initial_direction;

	world->scene = create_scene();

	world->terrain = create_terrain(256, 256, "assets/heightmap.data", 32);

	world->ch_position = initial_position;
	world->ch_position.y = sample_terrain_height(world->terrain, world->ch_position.x, world->ch_position.z) + 2.0f;

	scene_add_object(world->scene, world->terrain, MAT4_IDENTITY);

	struct model * water = create_plane(3);

	Mat4 water_matrix = mat4_translate(0,  TERR_WATER_THRESHOLD, 0.0f);

	water_matrix.a.x = 100000;
	water_matrix.b.y = 100000;
	water_matrix.c.z = 100000;

	scene_add_object(world->scene, water, water_matrix);

	struct model * tetrahedron = create_tetrahedron(0);

	Mat4 tth_matrix = mat4_translate(0, sample_terrain_height(world->terrain, 0, 0), 0);

	scene_add_object(world->scene, tetrahedron, tth_matrix);

	struct model * sphere = create_sphere(1, 8, 1);
	Mat4 mat = mat4_translate(0.0f, 0.5f + sample_terrain_height(world->terrain, 0.0f, 2.0f), 2.0f);
	scene_add_object(world->scene, sphere, mat);

	scene_set_eye(world->scene, world->ch_position, make_direction_vector(world->ch_direction));

	return world;
}
void start_world(struct world * world) {

	pthread_create(&world->thread, NULL, world_loop, world);
}
void destroy_world(struct world * world) {

	if (!world) return;

	pthread_mutex_lock(&world->mutex);
	world->stop = true;
	pthread_mutex_unlock(&world->mutex);

	if (world->thread) pthread_join(world->thread, NULL);

	free(world);
}
struct scene * world_get_scene(struct world * world) {
	return world->scene;
}
