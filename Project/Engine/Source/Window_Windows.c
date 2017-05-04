#ifdef GAME_WINDOWS
#define _CRT_SECURE_NO_WARNINGS

#include <Engine/Window.h>
#include <Engine/Log.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <windows.h>
#include <windowsx.h>
#include <Commdlg.h>
#include <shellapi.h>

#pragma comment(lib,"user32.lib")

#if defined(GAME_MOBILE) || defined(GAME_CONSOLE)
#define WINDOWS_MAX 1
#else // GAME_DESKTOP
#define WINDOWS_MAX 4
#endif

struct eng_WindowCallback
{
	void(*Func)(void*);
	void* UserData;
};

struct eng_Window
{
	// display properties
	unsigned Width, Height;
	char* Title;

	// handles
	HWND Hwnd;
	HDC Hdc;
	HINSTANCE Hinstance;

	// callbacks
	struct eng_WindowCallback* OnClose;
	unsigned OnCloseCount;

	// states
	bool Closing;
};

struct eng_Window* g_all_windows[WINDOWS_MAX] = { 0 };

bool eng_WindowSetupValidate(struct eng_Window* window, unsigned width, unsigned height, const char* title);
LRESULT CALLBACK eng_WindowWndproc(struct eng_Window* window, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK eng_Wndproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

void eng_WindowCallbackBind(struct eng_WindowCallback** outCallbacks, unsigned* outCallbacksCount, void(*func)(void*), void* user_data);
void eng_WindowCallbackUnbind(struct eng_WindowCallback** outCallbacks, unsigned* outCallbacksCount, void(*func)(void*));
void eng_WindowCallbackExec(struct eng_WindowCallback* callbacks, unsigned callbacksCount);


////////////////////////////////////////////////////////////////////////// Lifecycle

struct eng_Window* eng_WindowMalloc()
{
	return malloc(sizeof(struct eng_Window));
}

void eng_WindowFree(struct eng_Window* window, bool subAllocationsOnly)
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

	free(window->Title);
	free(window->OnClose);
	if (!subAllocationsOnly)
	{
		free(window);
	}
}

bool eng_WindowInit(struct eng_Window* window, unsigned width, unsigned height, const char* title)
{
	if (!eng_WindowSetupValidate(window, width, height, title))
	{
		return false;
	}

	bool globalWindowAssigned = false;
	for (unsigned i = 0; i < WINDOWS_MAX; ++i) {
		if (g_all_windows[i] == NULL) {
			g_all_windows[i] = window;
			globalWindowAssigned = true;
			break;
		}
	}
	if (!eng_Ensure(globalWindowAssigned, "Maximum number of windows exceded. The max is currently set at %d.", WINDOWS_MAX))
	{
		return false;
	}

	memset(window, 0, sizeof(struct eng_Window));

	window->Width = width;
	window->Height = height;
	window->Title = malloc(strlen(title) + 1);
	strcpy(window->Title, title);

	DWORD style, styleEx;
	style = WS_POPUP | WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	styleEx = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;

	window->Hinstance = GetModuleHandle(NULL);
	if (!eng_Ensure(window->Hinstance != 0, "GetModuleHandle failed."))
	{
		return false;
	}

	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = (WNDPROC)eng_Wndproc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = window->Hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hIconSm = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;  // No brush - we are going to paint our own background
	wc.lpszMenuName = NULL;  // No default menu
	wc.lpszClassName = L"game";
	if (!eng_Ensure(RegisterClassEx(&wc) != 0, "RegisterClassEx failed."))
	{
		return false;
	}

	HWND hwndDesktop = GetDesktopWindow();
	RECT desktop;
	GetWindowRect(hwndDesktop, &desktop);

	int x = desktop.right / 2 - width / 2,
		y = desktop.bottom / 2 - height / 2;

	RECT sysRect = { x, y, width, height };

	AdjustWindowRectEx(&sysRect, style, FALSE, styleEx);

	window->Hwnd = CreateWindowEx(styleEx, L"game", L"game", style, x, y, width, height, NULL, NULL, window->Hinstance, NULL);
	if (!eng_Ensure(window->Hwnd != 0, "CreateWindowEx failed."))
	{
		return false;
	}

	window->Hdc = GetDC(window->Hwnd);
	if (!eng_Ensure(window->Hdc != 0, "GetDC failed."))
	{
		return false;
	}

	// return value is not referring to success/failure. Don't use ensure here.
	// see: https://msdn.microsoft.com/en-us/library/windows/desktop/ms633548(v=vs.85).aspx
	ShowWindow(window->Hwnd, SW_SHOW);

	return true;
}

unsigned eng_WindowGetSizeof()
{
	return sizeof(struct eng_Window);
}

////////////////////////////////////////////////////////////////////////// Window API

void eng_WindowClose(struct eng_Window* window)
{
	if (!window->Closing)
	{
		window->Closing = true;
		if (window->Hwnd != 0)
		{
			eng_Ensure(CloseWindow(window->Hwnd), "CloseWindow call failed");
			window->Hwnd = 0;
		}

		eng_WindowCallbackExec(window->OnClose, window->OnCloseCount);
	}
}

bool eng_WindowUpdate(struct eng_Window* window, unsigned windowCount)
{
	MSG msg;
	unsigned livingWindows = windowCount;
	for (unsigned i = 0; i < windowCount; ++i)
	{
		while (PeekMessage(&msg, window[i].Hwnd, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (window[i].Closing) {
			--livingWindows;
		}
	}
	return livingWindows > 0;
}

const char* eng_WindowGetTitle(struct eng_Window* window)
{
	return window->Title;
}

unsigned eng_WindowGetWidth(struct eng_Window* window)
{
	return window->Width;
}

unsigned eng_WindowGetHeight(struct eng_Window* window)
{
	return window->Height;
}

void eng_WindowSetTitle(struct eng_Window* window, const char* title)
{
	free(window->Title);
	window->Title = malloc(strlen(title) + 1);
	strcpy(window->Title, title);
}

void eng_WindowSetSize(struct eng_Window* window, unsigned width, unsigned height)
{
	window->Width = width;
	window->Height = height;
}

////////////////////////////////////////////////////////////////////////// Callbacks

void eng_OnCloseBind(struct eng_Window* window, void(*on_close)(void*), void* user_data)
{
	eng_WindowCallbackBind(&window->OnClose, &window->OnCloseCount, on_close, user_data);
}

void eng_OnCloseUnbind(struct eng_Window* window, void(*on_close)(void*))
{
	eng_WindowCallbackUnbind(&window->OnClose, &window->OnCloseCount, on_close);
}

////////////////////////////////////////////////////////////////////////// Internal

bool eng_WindowSetupValidate(struct eng_Window* window, unsigned width, unsigned height, const char* title)
{
#if !defined(GAME_FINAL)
	if (window == NULL)
	{
		eng_DevFatal("Invalid window");
		return false;
	}
	if (width == 0 || width > 10000)
	{
		eng_DevFatal("Invalid width");
		return false;
	}
	if (height == 0 || height > 10000)
	{
		eng_DevFatal("Invalid height");
		return false;
	}
	if (title == NULL)
	{
		eng_DevFatal("Invalid title.");
		return false;
	}
#endif
	return true;
}

LRESULT CALLBACK eng_WindowWndproc(struct eng_Window* window, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_CLOSE:
		case WM_QUIT:
			eng_WindowClose(window);
			break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK eng_Wndproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	for (unsigned i = 0; i < WINDOWS_MAX; ++i)
	{
		if (g_all_windows[i])
		{
			if (g_all_windows[i]->Hwnd == hwnd)
			{
				return eng_WindowWndproc(g_all_windows[i], hwnd, msg, wParam, lParam);
			}
		}
		else
		{
			break;
		}
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void eng_WindowCallbackBind(struct eng_WindowCallback** outCallbacks, unsigned* outCallbacksCount, void(*func)(void*), void* user_data) 
{
	if (*outCallbacks == NULL)
	{
		*outCallbacks = malloc(sizeof(struct eng_WindowCallback));
		++(*outCallbacksCount);
	}
	else
	{
		*outCallbacks = realloc(*outCallbacks, ++(*outCallbacksCount) * sizeof(struct eng_WindowCallback));
	}
	struct eng_WindowCallback* callback = &(*outCallbacks)[*outCallbacksCount - 1];
	callback->Func = func;
	callback->UserData = user_data;
}

void eng_WindowCallbackUnbind(struct eng_WindowCallback** outCallbacks, unsigned* outCallbacksCount, void(*func)(void*))
{
	for (unsigned i = 0; i < *outCallbacksCount; ++i)
	{
		if ((*outCallbacks)[i].Func == func)
		{
			--(*outCallbacksCount);
			if ((*outCallbacksCount) == 0)
			{
				free((*outCallbacks));
				(*outCallbacks) = NULL;
				return;
			}

			memcpy(&(*outCallbacks)[i], &(*outCallbacks)[(*outCallbacksCount)], sizeof(struct eng_WindowCallback));
			(*outCallbacks) = realloc((*outCallbacks), (*outCallbacksCount) * sizeof(struct eng_WindowCallback));
			return;
		}
	}
}

void eng_WindowCallbackExec(struct eng_WindowCallback* callbacks, unsigned callbacksCount) 
{
	for (unsigned i = 0; i < callbacksCount; ++i)
	{
		callbacks[i].Func(callbacks[i].UserData);
	}
}

#endif