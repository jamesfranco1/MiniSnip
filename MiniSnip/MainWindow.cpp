#include "MainWindow.h"
#include "Snipping.h"
#include "NotificationWindow.h"
#include "Settings.h"

HWND g_hMainWnd = NULL;

void UpdateHotkeys(HWND hWnd)
{
    UnregisterHotKey(hWnd, HOTKEY_ID_SNIP_COPY);
    if (g_settings.hkCopyKey) RegisterHotKey(hWnd, HOTKEY_ID_SNIP_COPY, g_settings.hkCopyMod, g_settings.hkCopyKey);
}

bool RegisterMainAppWindow()
{
    WNDCLASS wc = { };
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = g_hInstance;
    wc.lpszClassName = MAIN_WND_CLASS;
    return RegisterClass(&wc) != 0;
}

HWND CreateMainAppWindow()
{
    return CreateWindowEx(
        0,
        MAIN_WND_CLASS,
        L"MiniSnip",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        HWND_MESSAGE,
        NULL,
        g_hInstance,
        NULL
    );
}

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        LoadSettings();
        SetupTrayIcon(hWnd);
        UpdateHotkeys(hWnd);
        break;
    case WM_HOTKEY:
        if (LOWORD(wParam) == HOTKEY_ID_SNIP_COPY) StartSnipping();
        break;
    case WM_APP_TRAY_MSG:
        if (LOWORD(lParam) == WM_RBUTTONUP) ShowContextMenu(hWnd);
        break;
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case IDM_START_SNIP:
            StartSnipping();
            break;
        case IDM_SETTINGS:
            ShowSettingsDialog(hWnd);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
    }
    break;
    case WM_APP_UPDATE_HOTKEYS:
        UpdateHotkeys(hWnd);
        break;
    case WM_APP_SHOW_NOTIFICATION:
        if (g_hNotificationWnd)
        {
            PostMessage(g_hNotificationWnd, WM_APP_SHOW_NOTIFICATION, wParam, lParam);
        }
        break;
    case WM_DESTROY:
        UnregisterHotKey(hWnd, HOTKEY_ID_SNIP_COPY);
        RemoveTrayIcon(hWnd);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

void SetupTrayIcon(HWND hWnd)
{
    NOTIFYICONDATA nid = { };
    nid.cbSize = sizeof(nid);
    nid.hWnd = hWnd;
    nid.uID = 1;
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid.uCallbackMessage = WM_APP_TRAY_MSG;
    nid.hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_MAINICON));
    wcscpy_s(nid.szTip, L"MiniSnip");
    Shell_NotifyIcon(NIM_ADD, &nid);

    nid.uVersion = NOTIFYICON_VERSION_4;
    Shell_NotifyIcon(NIM_SETVERSION, &nid);
}

void RemoveTrayIcon(HWND hWnd)
{
    NOTIFYICONDATA nid = { };
    nid.cbSize = sizeof(nid);
    nid.hWnd = hWnd;
    nid.uID = 1;
    Shell_NotifyIcon(NIM_DELETE, &nid);
}

void ShowContextMenu(HWND hWnd)
{
    HMENU hMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDC_CONTEXTMENU));
    HMENU hSubMenu = GetSubMenu(hMenu, 0);
    if (hSubMenu)
    {
        POINT pt;
        GetCursorPos(&pt);
        SetForegroundWindow(hWnd);
        TrackPopupMenu(hSubMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
    }
    DestroyMenu(hMenu);
}
