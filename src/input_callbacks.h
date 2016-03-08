#ifndef input_callbacks
#define input_callbacks

#define LEFT_BUTTON	1
#define RIGHT_BUTTON	2
#define MIDDLE_BUTTON	4

void on_mouse_button_press(float x, float y, int button);
void on_mouse_button_release(float x, float y, int button);
void on_mouse_move(float x, float y, int buttons);

void on_key_press(int keycode);
void on_key_release(int keycode);

enum keycode {
	KEY_NONE,
	KEY_SPECIAL_BEGIN = 0x1000,
	KEY_BACKSPACE,
	KEY_CAPS_LOCK,
	KEY_DELETE,
	KEY_DOWN,
	KEY_END,
	KEY_ENTER,
	KEY_ESCAPE,
	KEY_F1,
	KEY_F10,
	KEY_F11,
	KEY_F12,
	KEY_F2,
	KEY_F3,
	KEY_F4,
	KEY_F5,
	KEY_F6,
	KEY_F7,
	KEY_F8,
	KEY_F9,
	KEY_HOME,
	KEY_INSERT,
	KEY_KEYPAD_0,
	KEY_KEYPAD_1,
	KEY_KEYPAD_2,
	KEY_KEYPAD_3,
	KEY_KEYPAD_4,
	KEY_KEYPAD_5,
	KEY_KEYPAD_6,
	KEY_KEYPAD_7,
	KEY_KEYPAD_8,
	KEY_KEYPAD_9,
	KEY_KEYPAD_ADD,
	KEY_KEYPAD_DIV,
	KEY_KEYPAD_DOT,
	KEY_KEYPAD_ENTER,
	KEY_KEYPAD_MUL,
	KEY_KEYPAD_SUB,
	KEY_LEFT,
	KEY_LEFT_ALT,
	KEY_LEFT_CONTROL,
	KEY_LEFT_SHIFT,
	KEY_NUM_LOCK,
	KEY_PAGE_DOWN,
	KEY_PAGE_UP,
	KEY_PAUSE,
	KEY_RIGHT,
	KEY_RIGHT_CONTROL,
	KEY_RIGTH_ALT,
	KEY_RIGTH_SHIFT,
	KEY_TAB,
	KEY_UP,
	KEY_SPECIAL_END
};

#endif
