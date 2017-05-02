#include <stdbool.h>

#define WINDOWS_MAX 32

#ifdef __cplusplus
extern "C" {
#endif

	////////////////////////////////////////////////////////////////////////// Lifecycle
	struct eng_window* eng_window_malloc();
	bool eng_window_init(struct eng_window* window, unsigned width, unsigned height, const char* title);
	void eng_window_free(struct eng_window* window);

	////////////////////////////////////////////////////////////////////////// Window API
	void eng_window_close(struct eng_window* window);

	////////////////////////////////////////////////////////////////////////// Callbacks
	void eng_bind_onclose(struct eng_window* window, void(*on_close)(void* user_data), void* user_data);
	void eng_unbind_onclose(struct eng_window* window, void(*on_close)(void* user_data));

	////////////////////////////////////////////////////////////////////////// Debug
	void eng_window_dbg_print(struct eng_window* window);

#ifdef __cplusplus
}
#endif