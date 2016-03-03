#include <xcb/xcb.h>
#include <signal.h>
#include <malloc.h>
#include <string.h>

#include "vkapi.h"
#include "surface.h"
#include "main.h"
#include "plat_xcb.h"

struct plat_xcb_surface {
	struct plat_surface plat_surface;

	xcb_connection_t *conn;
	xcb_window_t wid;
	xcb_atom_t wm_delete_window_atom;
};

static xcb_atom_t _get_atom(struct xcb_connection_t *conn, const char *name) {

   xcb_atom_t atom;
   xcb_intern_atom_cookie_t cookie;
   xcb_intern_atom_reply_t *reply;

   cookie = xcb_intern_atom(conn, 0, strlen(name), name);
   reply = xcb_intern_atom_reply(conn, cookie, NULL);
   if (reply) atom = reply->atom;
   else atom = XCB_NONE;

   free(reply);
   return atom;
}

struct plat_surface* plat_xcb_get_surface(void) {

	struct plat_xcb_surface * surf = calloc(1, sizeof(struct plat_xcb_surface));

	surf->conn = xcb_connect(NULL, NULL);

	if (xcb_connection_has_error(surf->conn)) {
		printf("Cannot open display\n");
		goto error;
	}
	xcb_screen_t * screen = xcb_setup_roots_iterator (xcb_get_setup (surf->conn)).data;
	surf->wid = xcb_generate_id(surf->conn);

	printf("Conn: %p, root: %li, wid: %li\n", surf->conn, (long)screen->root, (long)surf->wid);

	uint32_t mask = XCB_GC_FOREGROUND | XCB_CW_EVENT_MASK;
	static uint32_t values[] = {
				0,
				XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_KEY_PRESS
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
		xcb_atom_t _net_wm_state_atom = _get_atom(surf->conn, "_NET_WM_STATE");
		xcb_atom_t _net_wm_state_fullscreen_atom = _get_atom(surf->conn, "_NET_WM_STATE_FULLSCREEN");
		if (_net_wm_state_atom != XCB_NONE && _net_wm_state_fullscreen_atom != XCB_NONE) {
			printf("Requesting full-screen window\n");
			xcb_change_property (surf->conn,
					     XCB_PROP_MODE_REPLACE,
					     surf->wid,
					     _net_wm_state_atom,
					     XCB_ATOM_ATOM,
					     32,
					     1,
					     &_net_wm_state_fullscreen_atom);
		}
		else {
			fprintf(stderr, "Cannot request full-screen window\n");
		}
	}

	xcb_atom_t wm_protocols_atom = _get_atom(surf->conn, "WM_PROTOCOLS");
	surf->wm_delete_window_atom = _get_atom(surf->conn, "WM_DELETE_WINDOW");
	if (wm_protocols_atom != XCB_NONE && surf->wm_delete_window_atom != XCB_NONE) {
		xcb_change_property(surf->conn,
					XCB_PROP_MODE_REPLACE,
					surf->wid,
					wm_protocols_atom,
					4, 32, 1,
					&surf->wm_delete_window_atom);
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

	printf("vkCreateXcbSurfaceKHR = %p\n", vkapi.vkCreateXcbSurfaceKHR);
	VkResult result = vkapi.vkCreateXcbSurfaceKHR(vkapi.instance, &surf_ci, NULL, &surf->plat_surface.vk_surface);
	if (result != VK_SUCCESS) {
		printf("vkCreateXcbSurfaceKHR failed: %i\n", result);
		goto error;
	}
	printf("vkCreateXcbSurfaceKHR OK\n");

	surf->plat_surface.event_loop = plat_xcb_event_loop;
	surf->plat_surface.destroy = plat_xcb_destroy_surface;
	surf->plat_surface.width = -1;
	surf->plat_surface.height = -1;

	return (struct plat_surface *)surf;

error:
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

	xcb_generic_event_t *event;
	while ( !_plat_xcb_signal_received && !exit_requested() && (event = xcb_wait_for_event(xcb_surf->conn)) ) {
        	switch (event->response_type & ~0x80) {
			case XCB_EXPOSE: {
				xcb_expose_event_t *expose = (xcb_expose_event_t *)event;

				printf("Window %li exposed. Region to be redrawn at location (%li ,%li), with dimension (%li,%li)\n",
					(long)expose->window, (long)expose->x, (long)expose->y, (long)expose->width, (long)expose->height );
				break;
			}
			case XCB_BUTTON_PRESS: {
				xcb_button_press_event_t *bp = (xcb_button_press_event_t *)event;
				printf("Button %i pressed\n", bp->detail);
				if (bp->detail == 3) {
					printf("Exitting on right click\n");
					request_exit();
				}
				break;
			}
			case XCB_KEY_PRESS: {
				xcb_key_press_event_t *bp = (xcb_key_press_event_t *)event;
				printf("Button %i pressed\n", bp->detail);
				break;
			}

			case XCB_CLIENT_MESSAGE: {
				xcb_client_message_event_t *cm = (xcb_client_message_event_t*)event;
				printf("Client message\n");
				if (cm->data.data32[0] == xcb_surf->wm_delete_window_atom) {
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
