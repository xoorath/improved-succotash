#ifdef GAME_WINDOWS
#define _CRT_SECURE_NO_WARNINGS

#include <Engine/Window.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <windows.h>
#include <windowsx.h>
#include <Commdlg.h>
#include <shellapi.h>

#pragma comment(lib,"user32.lib") 

struct eng_window_callback
{
	void(*func)(void*);
	void* user_data;
};

struct eng_window
{
	// display properties
	unsigned width, height;
	char* title;

	// handles
	HWND hwnd;
	HDC hdc;
	HINSTANCE hinstance;

	// callbacks
	struct eng_window_callback* on_close;
	unsigned on_close_count;

	// states
	bool closing;
};

struct eng_window* g_all_windows[WINDOWS_MAX] = {0};

bool eng_window_setup_validate(struct eng_window* window, unsigned width, unsigned height, const char* title);
LRESULT CALLBACK eng_window_wndproc(struct eng_window* window, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK eng_wndproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

struct eng_window* eng_window_malloc()
{
	struct eng_window* window = malloc(sizeof(struct eng_window));
	memset(window, 0, sizeof(struct eng_window));
	return window;
}

void eng_window_free(struct eng_window* window)
{
	for (unsigned i = 0; i < WINDOWS_MAX; ++i)
	{
		if (g_all_windows[i])
		{
			if (g_all_windows[i] == window)
			{
				g_all_windows[i] = NULL;
			}
		}
	}

	free(window->title);
	free(window->on_close);
	free(window);
}

bool eng_window_init(struct eng_window* window, unsigned width, unsigned height, const char* title)
{
	if (!eng_window_setup_validate(window, width, height, title))
	{
		return false;
	}

	window->width = width;
	window->height = height;
	window->title = malloc(strlen(title) + 1);
	strcpy(window->title, title);

	DWORD style, style_ex;
	style = WS_POPUP | WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	style_ex = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;

	if ((window->hinstance = GetModuleHandle(NULL)) == false)
	{
		printf("[eng_window_init] FAILED. GetModuleHandle returned false.");
		return false;
	}


	return true;
}

void eng_window_close(struct eng_window* window)
{
	if (!window->closing)
	{
		window->closing = true;
		if (window->hwnd != 0)
		{
			if (!CloseWindow(window->hwnd))
			{
				printf("[eng_window_close] FAILED. CloseWindow call failed.");
			}
			window->hwnd = 0;
		}

		for (unsigned i = 0; i < window->on_close_count; ++i)
		{
			window->on_close[i].func(window->on_close[i].user_data);
		}
	}
}

////////////////////////////////////////////////////////////////////////// Callbacks

void eng_bind_onclose(struct eng_window* window, void(*on_close)(void*), void* user_data)
{
	if (window->on_close == NULL)
	{
		window->on_close = malloc(sizeof(struct eng_window_callback));
		++window->on_close_count;
	}
	else
	{
		window->on_close = realloc(window->on_close, ++window->on_close_count * sizeof(struct eng_window_callback));
	}
	struct eng_window_callback* callback = &window->on_close[window->on_close_count - 1];
	callback->func = on_close;
	callback->user_data = user_data;
}

void eng_unbind_onclose(struct eng_window* window, void(*on_close)(void*))
{
	for (unsigned i = 0; i < window->on_close_count; ++i)
	{
		if (window->on_close[i].func == on_close)
		{
			--window->on_close_count;
			if (window->on_close_count == 0)
			{
				free(window->on_close);
				return;
			}

			memcpy(&window->on_close[i], &window->on_close[window->on_close_count], sizeof(struct eng_window_callback));
			window->on_close = realloc(window->on_close, window->on_close_count * sizeof(struct eng_window_callback));
			return;
		}
	}
}

////////////////////////////////////////////////////////////////////////// Debug
void eng_window_dbg_print(struct eng_window* window)
{
	printf("[%s] (%d, %d)", window->title, window->width, window->height);
}

////////////////////////////////////////////////////////////////////////// Internal

bool eng_window_setup_validate(struct eng_window* window, unsigned width, unsigned height, const char* title)
{
	if (window == NULL)
	{
		printf("[eng_window_setup] FAILED. Invalid window");
		return false;
	}
	if (width == 0 || width > 10000)
	{
		printf("[eng_window_setup] FAILED. Invalid width");
		return false;
	}
	if (height == 0 || height > 10000)
	{
		printf("[eng_window_setup] FAILED. Invalid height");
		return false;
	}
	if (title == NULL)
	{
		printf("[eng_window_setup] FAILED. Invalid title.");
		return false;
	}
	return true;
}

LRESULT CALLBACK eng_window_wndproc(struct eng_window* window, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CLOSE:
	case WM_QUIT:
		eng_window_close(window);
		break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK eng_wndproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	for (unsigned i = 0; i < WINDOWS_MAX; ++i)
	{
		if (g_all_windows[i])
		{
			if (g_all_windows[i]->hwnd == hwnd)
			{
				return eng_window_wndproc(g_all_windows[i], hwnd, msg, wParam, lParam);
			}
		}
		else
		{
			break;
		}
	}
	printf("[eng_wndproc] FAILED. Could not find window with hwnd 0x%p. Falling back to DefWindowProc.", hwnd);
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

#endif