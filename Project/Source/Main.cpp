
#include <Engine/Window.h>

int main(int argsc, char** argsv) {
	struct eng_window* window = eng_window_malloc();
	if (eng_window_init(window, 1280, 720, "Improved Succotash"))
	{
		eng_window_dbg_print(window);
	}
	eng_window_free(window);
	window = nullptr;
	return 0;
}