#include "Settings.h"
#include <commctrl.h>
#include <dwmapi.h>

AppSettings g_settings;

std::wstring GetSettingsFilePath()
{
    WCHAR szPath[MAX_PATH];
    if (GetModuleFileName(NULL, szPath, MAX_PATH))
    {
        std::wstring exePath(szPath);
        std::wstring iniPath = exePath.substr(0, exePath.find_last_of(L"\\/") + 1) + L"MiniSnip.ini";

        HANDLE hFile = CreateFile(iniPath.c_str(), GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hFile);
            return iniPath;
        }
    }

    PWSTR pwszPath = NULL;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &pwszPath)))
    {
        std::wstring appDataPath(pwszPath);
        CoTaskMemFree(pwszPath);
        appDataPath += L"\\MiniSnip";
        CreateDirectory(appDataPath.c_str(), NULL);
        return appDataPath + L"\\MiniSnip.ini";
    }
    return L"MiniSnip.ini";
}

void LoadSettings()
{
    std::wstring path = GetSettingsFilePath();

    g_settings.hkCopyMod = GetPrivateProfileInt(L"Hotkeys", L"CopyMod", g_settings.hkCopyMod, path.c_str());
    g_settings.hkCopyKey = GetPrivateProfileInt(L"Hotkeys", L"CopyKey", g_settings.hkCopyKey, path.c_str());
}

void SaveSettings()
{
    std::wstring path = GetSettingsFilePath();

    WritePrivateProfileString(L"Hotkeys", L"CopyMod", std::to_wstring(g_settings.hkCopyMod).c_str(), path.c_str());
    WritePrivateProfileString(L"Hotkeys", L"CopyKey", std::to_wstring(g_settings.hkCopyKey).c_str(), path.c_str());
}

WORD GetHkCombo(DWORD mod)
{
    WORD w = 0;
    if (mod & MOD_CONTROL) w |= HOTKEYF_CONTROL;
    if (mod & MOD_SHIFT)   w |= HOTKEYF_SHIFT;
    if (mod & MOD_ALT)     w |= HOTKEYF_ALT;
    return w;
}

DWORD GetModFromHk(WORD w)
{
    DWORD mod = 0;
    if (w & HOTKEYF_CONTROL) mod |= MOD_CONTROL;
    if (w & HOTKEYF_SHIFT)   mod |= MOD_SHIFT;
    if (w & HOTKEYF_ALT)     mod |= MOD_ALT;
    return mod;
}

INT_PTR CALLBACK SettingsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HBRUSH hBrushBg = NULL;
    switch (message)
    {
    case WM_INITDIALOG:
    {
        BOOL value = TRUE;
        DwmSetWindowAttribute(hDlg, 20, &value, sizeof(value));

        hBrushBg = CreateSolidBrush(RGB(32, 32, 32));

        SendDlgItemMessage(hDlg, IDC_HOTKEY_COPY, HKM_SETHOTKEY, MAKEWORD(g_settings.hkCopyKey, GetHkCombo(g_settings.hkCopyMod)), 0);
        return (INT_PTR)TRUE;
    }
    case WM_CTLCOLORDLG:
        return (INT_PTR)hBrushBg;
    case WM_CTLCOLORSTATIC:
    {
        HDC hdc = (HDC)wParam;
        SetTextColor(hdc, RGB(240, 240, 240));
        SetBkMode(hdc, TRANSPARENT);
        return (INT_PTR)hBrushBg;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
        {
            WORD hkCopy = (WORD)SendDlgItemMessage(hDlg, IDC_HOTKEY_COPY, HKM_GETHOTKEY, 0, 0);
            g_settings.hkCopyKey = LOBYTE(hkCopy);
            g_settings.hkCopyMod = GetModFromHk(HIBYTE(hkCopy));

            SaveSettings();
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        else if (LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    case WM_DESTROY:
        if (hBrushBg) DeleteObject(hBrushBg);
        break;
    }
    return (INT_PTR)FALSE;
}

void ShowSettingsDialog(HWND hParent)
{
    if (g_hMainWnd)
    {
        UnregisterHotKey(g_hMainWnd, HOTKEY_ID_SNIP_COPY);
    }

    DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_SETTINGS_DLG), hParent, SettingsDlgProc);

    if (g_hMainWnd)
    {
        PostMessage(g_hMainWnd, WM_APP_UPDATE_HOTKEYS, 0, 0);
    }
}
