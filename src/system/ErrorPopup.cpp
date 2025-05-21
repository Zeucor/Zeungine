#include <zg/Window.hpp>
#include <zg/system/ErrorPopup.hpp>
#include <iostream>
#include <zg/Standard.hpp>
#if defined(_WIN32)
// #elif defined(__linux__)
// #include <gtk/gtk.h>
// #elif defined(MACOS)
// #include <Cocoa/Cocoa.h>
#endif
using namespace zg::system;
#if defined(_WIN32)
LRESULT CALLBACK ErrorWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}
#endif
bool ErrorPopup::show(const std::string& message)
{
#if defined(_WIN32)
	// MessageBoxA(0, message.c_str(), "Error", MB_OK | MB_ICONERROR);
	const char CLASS_NAME[] = "ErrorPopupWindow";

	WNDCLASS wc = {};
	wc.lpfnWndProc = ErrorWindowProc;
	wc.hInstance = GetModuleHandle(nullptr);
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	HWND hwnd = CreateWindowEx(0, CLASS_NAME, "Error", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 500, 400,
														 nullptr, nullptr, GetModuleHandle(nullptr), nullptr);

	if (!hwnd)
		return false;

	HWND hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", message.c_str(),
															WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY, 10, 10, 460, 300,
															hwnd, nullptr, GetModuleHandle(nullptr), nullptr);

	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
#elif defined(__linux__)
	std::cerr << message << std::endl;
	// gtk_init(0, nullptr);
	// GtkWidget* dialog =
	// 	gtk_message_dialog_new(nullptr, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", message.c_str());
	// gtk_dialog_run(GTK_DIALOG(dialog));
	// gtk_widget_destroy(dialog);
#elif defined(MACOS)
	std::cerr << message << std::endl;
	// @autoreleasepool
	// {
	// 	NSAlert* alert = [[NSAlert alloc] init];
	// 	[alert setMessageText:@"Error"];
	// 	[alert setInformativeText:[NSString stringWithUTF8String:message.c_str()]];
	// 	[alert addButtonWithTitle:@"OK"];
	// 	[alert setAlertStyle:NSAlertStyleCritical];
	// 	[alert runModal];
	// }
#endif
	return true;
}
