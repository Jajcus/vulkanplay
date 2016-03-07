#ifndef input_callbacks
#define input_callbacks

#define LEFT_BUTTON	1
#define RIGHT_BUTTON	2
#define MIDDLE_BUTTON	4

void on_mouse_button_press(float x, float y, int button);
void on_mouse_button_release(float x, float y, int button);
void on_mouse_move(float x, float y, int buttons);

void on_key_press(int keycode, const char * keyname);
void on_key_release(int keycode, const char * keyname);

#endif
