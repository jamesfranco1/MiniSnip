#pragma once

#include <windows.h>
#include <shlobj.h>
#include <knownfolders.h>
#include <objbase.h>
#include <gdiplus.h>
#include <string>
#include <vector>
#include <memory>
#include "resource.h"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

constexpr UINT WM_APP_TRAY_MSG = WM_APP + 1;
constexpr int HOTKEY_ID_SNIP_COPY = 1;

constexpr UINT WM_APP_SHOW_NOTIFICATION = WM_APP + 2;
constexpr UINT WM_APP_UPDATE_HOTKEYS = WM_APP + 5;

constexpr int NOTIFY_COPY_SUCCESS = 3;

const WCHAR MAIN_WND_CLASS[] = L"MiniSnipMainWndClass";
const WCHAR OVERLAY_WND_CLASS[] = L"MiniSnipOverlayWndClass";
const WCHAR NOTIFICATION_WND_CLASS[] = L"MiniSnipNotificationWndClass";

struct AppSettings {
    DWORD hkCopyMod = 0;
    DWORD hkCopyKey = VK_F7;
};

extern HINSTANCE g_hInstance;
extern HWND g_hMainWnd;
extern HWND g_hNotificationWnd;
extern HBITMAP g_hScreenshot;
extern POINT g_startPoint;
extern POINT g_endPoint;
extern bool g_isSelecting;
extern std::vector<HWND> g_hOverlayWnds;
extern ULONG_PTR g_gdiplusToken;
extern AppSettings g_settings;
