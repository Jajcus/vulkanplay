#include <xcb/xcb.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "vkapi.h"
#include "surface.h"
#include "main.h"
#include "platform/plat_xcb.h"
#include "input_callbacks.h"

enum atom_id {
	_NET_WM_STATE,
	_NET_WM_STATE_FULLSCREEN,
	WM_PROTOCOLS,
	WM_DELETE_WINDOW,
	ATOM_COUNT,
};
static const char * atom_names[ATOM_COUNT] = {
	"_NET_WM_STATE",
	"_NET_WM_STATE_FULLSCREEN",
	"WM_PROTOCOLS",
	"WM_DELETE_WINDOW",
};

struct keysym_to_code {
	xcb_keysym_t keysym;
	enum keycode code;
};

// based on X11/keysymdef.h
static const struct keysym_to_code keysym_dict[] = {
	{0xff08, KEY_BACKSPACE},
	{0xff09, KEY_TAB},
	{0xff0d, KEY_ENTER},
	{0xff13, KEY_PAUSE},
	{0xff1b, KEY_ESCAPE},
	{0xff50, KEY_HOME},
	{0xff51, KEY_LEFT},
	{0xff52, KEY_UP},
	{0xff53, KEY_RIGHT},
	{0xff54, KEY_DOWN},
	{0xff55, KEY_PAGE_UP},
	{0xff56, KEY_PAGE_DOWN},
	{0xff57, KEY_END},
	{0xff7f, KEY_NUM_LOCK},
	{0xff63, KEY_INSERT},
	{0xff8d, KEY_KEYPAD_ENTER},
	{0xff95, KEY_KEYPAD_7},
	{0xff96, KEY_KEYPAD_4},
	{0xff97, KEY_KEYPAD_8},
	{0xff98, KEY_KEYPAD_6},
	{0xff99, KEY_KEYPAD_2},
	{0xff9a, KEY_KEYPAD_9},
	{0xff9b, KEY_KEYPAD_3},
	{0xff9c, KEY_KEYPAD_1},
	{0xff9d, KEY_KEYPAD_5},
	{0xff9e, KEY_KEYPAD_0},
	{0xff9f, KEY_KEYPAD_DOT},
	{0xffaa, KEY_KEYPAD_MUL},
	{0xffab, KEY_KEYPAD_ADD},
	{0xffad, KEY_KEYPAD_SUB},
	{0xffae, KEY_KEYPAD_DOT},
	{0xffaf, KEY_KEYPAD_DIV},
	{0xffb1, KEY_KEYPAD_1},
	{0xffb2, KEY_KEYPAD_2},
	{0xffb3, KEY_KEYPAD_3},
	{0xffb4, KEY_KEYPAD_4},
	{0xffb5, KEY_KEYPAD_5},
	{0xffb6, KEY_KEYPAD_6},
	{0xffb7, KEY_KEYPAD_7},
	{0xffb8, KEY_KEYPAD_8},
	{0xffb9, KEY_KEYPAD_9},
	{0xffbd, KEY_KEYPAD_ENTER},
	{0xffbe, KEY_F1},
	{0xffbf, KEY_F2},
	{0xffc0, KEY_F3},
	{0xffc1, KEY_F4},
	{0xffc2, KEY_F5},
	{0xffc3, KEY_F6},
	{0xffc4, KEY_F7},
	{0xffc5, KEY_F8},
	{0xffc6, KEY_F9},
	{0xffc7, KEY_F10},
	{0xffc8, KEY_F11},
	{0xffc9, KEY_F12},
	{0xffe1, KEY_LEFT_SHIFT},
	{0xffe2, KEY_RIGTH_SHIFT},
	{0xffe3, KEY_LEFT_CONTROL},
	{0xffe4, KEY_RIGHT_CONTROL},
	{0xffe5, KEY_CAPS_LOCK},
	{0xffe9, KEY_LEFT_ALT},
	{0xffea, KEY_RIGTH_ALT},
	{0xffff, KEY_DELETE},
	{0x0000, KEY_NONE},
};

struct plat_xcb_surface {
	struct plat_surface plat_surface;

	xcb_connection_t *conn;
	xcb_window_t wid;

	xcb_atom_t atoms[ATOM_COUNT];
	xcb_intern_atom_cookie_t atom_cookies[ATOM_COUNT];

	xcb_get_keyboard_mapping_reply_t *keymap;
	xcb_keysym_t * keysyms;
	int keysyms_length;
	int first_keycode, last_keycode;
	xcb_get_keyboard_mapping_cookie_t keymap_cookie;
};

static void _request_atoms(struct plat_xcb_surface * surf) {

	int i;
	for(i = 0; i < ATOM_COUNT; i++){
		surf->atom_cookies[i] = xcb_intern_atom(surf->conn, 0,
							strlen(atom_names[i]),
							atom_names[i]);
	}
}

static void _collect_atoms(struct plat_xcb_surface * surf) {

	int i;
	for(i = 0; i < ATOM_COUNT; i++){
		xcb_intern_atom_reply_t *reply;
		reply = xcb_intern_atom_reply(surf->conn, surf->atom_cookies[i], NULL);
		if (reply) {
			surf->atoms[i] = reply->atom;
			free(reply);
		}
		else surf->atoms[i] = XCB_NONE;
	}
}


static void _request_keymap(struct plat_xcb_surface * surf) {

	surf->first_keycode = xcb_get_setup(surf->conn)->min_keycode;
	surf->last_keycode = xcb_get_setup(surf->conn)->max_keycode;
	surf->keymap_cookie = xcb_get_keyboard_mapping(surf->conn, surf->first_keycode,
						surf->last_keycode - surf->first_keycode + 1);
}

static void _collect_keymap(struct plat_xcb_surface * surf) {

	surf->keymap = xcb_get_keyboard_mapping_reply(surf->conn, surf->keymap_cookie, NULL);
	if (surf->keymap) {
		surf->keysyms = xcb_get_keyboard_mapping_keysyms(surf->keymap);
		surf->keysyms_length = xcb_get_keyboard_mapping_keysyms_length(surf->keymap);
	}
}
static xcb_keysym_t _keycode_to_keysym(struct plat_xcb_surface * surf, int keycode) {

	int kpk = surf->keymap->keysyms_per_keycode;

	if (keycode < surf->first_keycode || keycode > surf->last_keycode) {
		return XCB_NO_SYMBOL;
	}

	xcb_keysym_t * syms = &surf->keysyms[(keycode - surf->first_keycode) * kpk];

	return syms[0];
}

enum keycode _keysym_to_code(xcb_keysym_t keysym) {

	if (keysym >= 0x20 && keysym < 0x7f) {
		return (enum keycode)keysym;
	}
	int i;
	for(i = 0; keysym_dict[i].keysym; i++) {
		if (keysym_dict[i].keysym == keysym) return keysym_dict[i].code;
	}
	return KEY_NONE;
}

struct plat_surface* plat_xcb_get_surface(void) {

	struct plat_xcb_surface * surf = calloc(1, sizeof(struct plat_xcb_surface));

	surf->conn = xcb_connect(NULL, NULL);
	if (xcb_connection_has_error(surf->conn)) {
		printf("Cannot open display\n");
		goto error;
	}

	_request_atoms(surf);
	_request_keymap(surf);
	_collect_atoms(surf);
	_collect_keymap(surf);
	if (!surf->keymap)  {
		printf("Cannot read X11 keymap\n");
		goto error;
	}

	xcb_screen_t * screen = xcb_setup_roots_iterator (xcb_get_setup (surf->conn)).data;
	surf->wid = xcb_generate_id(surf->conn);

	printf("Conn: %p, root: %li, wid: %li\n", (void *)surf->conn, (long)screen->root, (long)surf->wid);

	uint32_t mask = XCB_GC_FOREGROUND | XCB_CW_EVENT_MASK;
	static uint32_t values[] = {
				0,
				XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_STRUCTURE_NOTIFY
				| XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE
				| XCB_EVENT_MASK_POINTER_MOTION
				| XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE
				};
	values[0] = screen->black_pixel;

	xcb_create_window(surf->conn,
                           XCB_COPY_FROM_PARENT,          /* depth               */
                           surf->wid,
                           screen->root,                  /* parent window       */
                           0, 0,                          /* x, y                */
                           options.win_width,
			   options.win_height,
                           10,                            /* border_width        */
                           XCB_WINDOW_CLASS_INPUT_OUTPUT, /* class               */
                           screen->root_visual,           /* visual              */
                           mask, values);                 /* masks, values       */

	char *title = "Vulkan Play";
	xcb_change_property (surf->conn,
                             XCB_PROP_MODE_REPLACE,
                             surf->wid,
                             XCB_ATOM_WM_NAME,
                             XCB_ATOM_STRING,
                             8,
                             strlen(title),
                             title);

	if (options.fullscreen) {
		if (surf->atoms[_NET_WM_STATE] != XCB_NONE && surf->atoms[_NET_WM_STATE_FULLSCREEN] != XCB_NONE) {
			printf("Requesting full-screen window\n");
			xcb_change_property (surf->conn,
					     XCB_PROP_MODE_REPLACE,
					     surf->wid,
					     surf->atoms[_NET_WM_STATE],
					     XCB_ATOM_ATOM,
					     32,
					     1,
					     &surf->atoms[_NET_WM_STATE_FULLSCREEN]);
		}
		else {
			fprintf(stderr, "Cannot request full-screen window\n");
		}
	}

	if (surf->atoms[WM_PROTOCOLS] != XCB_NONE && surf->atoms[WM_DELETE_WINDOW] != XCB_NONE) {
		xcb_change_property(surf->conn,
					XCB_PROP_MODE_REPLACE,
					surf->wid,
					surf->atoms[WM_PROTOCOLS],
					4, 32, 1,
					&surf->atoms[WM_DELETE_WINDOW]);
	}
	else {
		fprintf(stderr, "Could not request WM_DELETE_WINDOW event.");
	}

	xcb_map_window(surf->conn, surf->wid);
        xcb_flush(surf->conn);

	VkXcbSurfaceCreateInfoKHR surf_ci = {
		.sType=VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
		.connection=surf->conn,
		.window=surf->wid,
	};

	VkResult result = vkapi.vkCreateXcbSurfaceKHR(vkapi.instance, &surf_ci, NULL, &surf->plat_surface.vk_surface);
	if (result != VK_SUCCESS) {
		printf("vkCreateXcbSurfaceKHR failed: %i\n", result);
		goto error;
	}

	surf->plat_surface.event_loop = plat_xcb_event_loop;
	surf->plat_surface.destroy = plat_xcb_destroy_surface;
	surf->plat_surface.width = -1;
	surf->plat_surface.height = -1;

	return (struct plat_surface *)surf;

error:
	if (surf->keymap) free(surf->keymap);
	xcb_disconnect(surf->conn);
	free(surf);
	return NULL;
}

static volatile int _plat_xcb_signal_received = 0;

static void _plat_xcb_sig_handler(int signum) {

	_plat_xcb_signal_received = signum;
}

/* platform-dependent */
void plat_xcb_event_loop(struct plat_surface *surf) {

	struct plat_xcb_surface * xcb_surf = (struct plat_xcb_surface *)surf;

	struct sigaction old_sa;
	struct sigaction sa = {
		.sa_handler = _plat_xcb_sig_handler,
	};
	sigaction(SIGINT, &sa, &old_sa);

	int width = 1, height = 1;

	xcb_generic_event_t *event, *next_event = NULL;
	while ( !_plat_xcb_signal_received && !exit_requested() ) {
		if (next_event) event = next_event;
		else event = xcb_wait_for_event(xcb_surf->conn);
		if (!event) break;
		next_event = NULL;
		switch (event->response_type & ~0x80) {
			case XCB_EXPOSE: {
				// xcb_expose_event_t *expose = (xcb_expose_event_t *)event;
				break;
			}
			case XCB_BUTTON_PRESS: {
				xcb_button_press_event_t *bp = (xcb_button_press_event_t *)event;
				printf("Button %i pressed at (%4i, %4i)\n", bp->detail, bp->event_x, bp->event_y);
				int button = 0;
				switch(bp->detail) {
					case 1: button = LEFT_BUTTON; break;
					case 2: button = MIDDLE_BUTTON; break;
					case 3: button = RIGHT_BUTTON; break;
				}
				if (button) {
					on_mouse_button_press(
						2.0f * bp->event_x / width - 1.0f,
					       -2.0f * bp->event_y / height + 1.0f,
					       button);
				}
				break;
			}
			case XCB_BUTTON_RELEASE: {
				xcb_button_release_event_t *bp = (xcb_button_release_event_t *)event;
				printf("Button %i released at (%4i, %4i)\n", bp->detail, bp->event_x, bp->event_y);
				int button = 0;
				switch(bp->detail) {
					case 1: button = LEFT_BUTTON; break;
					case 2: button = MIDDLE_BUTTON; break;
					case 3: button = RIGHT_BUTTON; break;
				}
				if (button) {
					on_mouse_button_release(
						2.0f * bp->event_x / width - 1.0f,
					       -2.0f * bp->event_y / height + 1.0f,
					       button);
				}
				break;
			}
			case XCB_MOTION_NOTIFY: {
				xcb_motion_notify_event_t *bp = (xcb_motion_notify_event_t *)event;
				int button = 0;
				if ((bp->state & XCB_KEY_BUT_MASK_BUTTON_1)) button |= LEFT_BUTTON;
				if ((bp->state & XCB_KEY_BUT_MASK_BUTTON_2)) button |= MIDDLE_BUTTON;
				if ((bp->state & XCB_KEY_BUT_MASK_BUTTON_3)) button |= RIGHT_BUTTON;
				on_mouse_move(2.0f * bp->event_x / width - 1.0f,
					       -2.0f * bp->event_y / height + 1.0f,
					       button);
				break;
			}
			case XCB_KEY_PRESS: {
				xcb_key_press_event_t *kp = (xcb_key_press_event_t *)event;
				printf("Key %i pressed\n", kp->detail);
				xcb_keysym_t keysym = _keycode_to_keysym(xcb_surf, kp->detail);
				if (keysym >= 0x20 && keysym < 0x7f) {
					printf("  '%c' pressed\n", (int)keysym);
				}
				else {
					printf("  0x%04x pressed\n", keysym);
				}
				on_key_press(_keysym_to_code(keysym));
				break;
			}
			case XCB_KEY_RELEASE: {
				xcb_key_release_event_t *kr = (xcb_key_release_event_t *)event;
				next_event = xcb_poll_for_event(xcb_surf->conn);
				if (next_event && ((next_event->response_type & ~0x80) == XCB_KEY_PRESS)) {
					xcb_key_press_event_t *kp = (xcb_key_press_event_t *)event;
					if (kp->detail == kr->detail && kp->time == kr->time) {
						// auto-repeat
						next_event = NULL;
						break;
					}
				}
				printf("Key %i released\n", kr->detail);
				xcb_keysym_t keysym = _keycode_to_keysym(xcb_surf, kr->detail);
				if (keysym >= 0x20 && keysym < 0x7f) {
					printf("  '%c' released\n", (int)keysym);
				}
				else {
					printf("  0x%04x released\n", keysym);
				}
				on_key_release(_keysym_to_code(keysym));
				break;
			}
			case XCB_CONFIGURE_NOTIFY: {
				xcb_configure_notify_event_t *cn = (xcb_configure_notify_event_t *)event;
				printf("Configure notify, new size: (%4i, %4i)\n", cn->width, cn->height);
				width = cn->width ? cn->width : 1;
				height = cn->height ? cn->height : 1;
				break;
			}
			case XCB_CLIENT_MESSAGE: {
				xcb_client_message_event_t *cm = (xcb_client_message_event_t*)event;
				printf("Client message\n");
				if (cm->data.data32[0] == xcb_surf->atoms[WM_DELETE_WINDOW]) {
					printf("DELETE_WINDOW request\n");
					request_exit();
				}
				break;
			}
			default:
				printf ("Unknown event: %i\n", event->response_type);
				break;
                }
		free (event);
        }
	printf("Finishing platform event loop (exit_requested=%i, signal_received=%i).\n", exit_requested(), _plat_xcb_signal_received);
	if (_plat_xcb_signal_received) {
		printf("Signal %i received\n", _plat_xcb_signal_received);
	}
	request_exit();

	sigaction(SIGINT, &old_sa, NULL);
}

void plat_xcb_destroy_surface(struct plat_surface *surf) {

	struct plat_xcb_surface * xcb_surf = (struct plat_xcb_surface *)surf;

	finalize_surface(surf);

	xcb_disconnect(xcb_surf->conn);

	free(surf);
}

