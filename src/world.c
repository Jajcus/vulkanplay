#include "main.h"
#include "world.h"
#include "scene.h"
#include "input_callbacks.h"
#include <sys/time.h>
#include <time.h>
#include <malloc.h>
#include <string.h>

#include "models/tetrahedron.h"

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
	struct timeval ts;
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

};

void on_mouse_button_press(float x, float y, int button) {

	pthread_mutex_lock(&in_queue_mutex);
	if (in_queue_full()) {
		fprintf(stderr, "dropping mouse button press event\n");
	}
	else {
		in_queue[in_queue_head].type = EVENT_MOUSE_BUTTON_PRESS;
		gettimeofday(&in_queue[in_queue_head].ts, NULL);
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
		gettimeofday(&in_queue[in_queue_head].ts, NULL);
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
		gettimeofday(&in_queue[in_queue_head].ts, NULL);
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
		gettimeofday(&in_queue[in_queue_head].ts, NULL);
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
		gettimeofday(&in_queue[in_queue_head].ts, NULL);
		in_queue[in_queue_head].keycode = keycode;
		in_queue_head = (in_queue_head + 1) % IN_QUEUE_LEN;
		pthread_cond_signal(&in_queue_event);
	}
	pthread_mutex_unlock(&in_queue_mutex);
}

struct world * create_world(void) {

	struct world * world = (struct world *)calloc(1, sizeof(struct world));

	world->scene = create_scene();
	struct model * tetrahedron = create_tetrahedron(0);

	scene_add_object(world->scene, tetrahedron, MAT4_IDENTITY);

	return world;
}
void destroy_world(struct world * world) {

	if (world) free(world);
}
struct scene * world_get_scene(struct world * world) {
	return world->scene;
}
