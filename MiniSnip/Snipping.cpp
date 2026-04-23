#include "Snipping.h"
#include "Actions.h"

HBITMAP g_hScreenshot = NULL;
POINT g_startPoint = { 0, 0 };
POINT g_endPoint = { 0, 0 };
bool g_isSelecting = false;
std::vector<HWND> g_hOverlayWnds;

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);

bool RegisterSnippingOverlayClass()
{
    WNDCLASS wcOverlay = { };
    wcOverlay.lpfnWndProc = OverlayWndProc;
    wcOverlay.hInstance = g_hInstance;
    wcOverlay.lpszClassName = OVERLAY_WND_CLASS;
    wcOverlay.hCursor = LoadCursor(NULL, IDC_CROSS);
    wcOverlay.style = CS_DBLCLKS;
    return RegisterClass(&wcOverlay) != 0;
}

void CloseAllOverlays()
{
    for (HWND hwnd : g_hOverlayWnds)
    {
        if (IsWindow(hwnd)) DestroyWindow(hwnd);
    }
    g_hOverlayWnds.clear();

    if (g_hScreenshot)
    {
        DeleteObject(g_hScreenshot);
        g_hScreenshot = NULL;
    }
    g_isSelecting = false;
}

void StartSnipping()
{
    if (!g_hOverlayWnds.empty()) return;

    g_hScreenshot = TakeFullscreenScreenshot();
    if (!g_hScreenshot) return;

    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, 0);
}

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
    int x = lprcMonitor->left;
    int y = lprcMonitor->top;
    int w = lprcMonitor->right - lprcMonitor->left;
    int h = lprcMonitor->bottom - lprcMonitor->top;

    HWND hOverlay = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
        OVERLAY_WND_CLASS,
        L"Snipping Overlay",
        WS_POPUP,
        x, y, w, h,
        NULL,
        NULL,
        g_hInstance,
        NULL
    );

    if (hOverlay)
    {
        SetLayeredWindowAttributes(hOverlay, 0, 255, LWA_ALPHA);
        ShowWindow(hOverlay, SW_SHOW);
        g_hOverlayWnds.push_back(hOverlay);
    }
    return TRUE;
}

LRESULT CALLBACK OverlayWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        RECT rcWnd;
        GetWindowRect(hWnd, &rcWnd);
        int wndWidth = rcWnd.right - rcWnd.left;
        int wndHeight = rcWnd.bottom - rcWnd.top;

        HDC hdcMem = CreateCompatibleDC(hdc);
        HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, g_hScreenshot);

        int virtualScreenX = GetSystemMetrics(SM_XVIRTUALSCREEN);
        int virtualScreenY = GetSystemMetrics(SM_YVIRTUALSCREEN);

        int sourceX = rcWnd.left - virtualScreenX;
        int sourceY = rcWnd.top - virtualScreenY;

        BitBlt(hdc, 0, 0, wndWidth, wndHeight, hdcMem, sourceX, sourceY, SRCCOPY);

        SelectObject(hdcMem, hOldBitmap);
        DeleteDC(hdcMem);

        if (g_isSelecting)
        {
            HPEN hPen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
            HGDIOBJ hOldPen = SelectObject(hdc, hPen);
            HGDIOBJ hOldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));

            RECT rcGlobal = GetSelectionRect();
            RECT rcLocal = rcGlobal;

            rcLocal.left -= rcWnd.left;
            rcLocal.right -= rcWnd.left;
            rcLocal.top -= rcWnd.top;
            rcLocal.bottom -= rcWnd.top;

            Rectangle(hdc, rcLocal.left, rcLocal.top, rcLocal.right, rcLocal.bottom);

            SelectObject(hdc, hOldPen);
            SelectObject(hdc, hOldBrush);
            DeleteObject(hPen);
        }

        EndPaint(hWnd, &ps);
    }
    break;

    case WM_LBUTTONDOWN:
    {
        POINT pt = { (short)LOWORD(lParam), (short)HIWORD(lParam) };
        ClientToScreen(hWnd, &pt);

        g_startPoint = pt;
        g_endPoint = g_startPoint;
        g_isSelecting = true;
        SetCapture(hWnd);

        for (HWND hOverlay : g_hOverlayWnds) InvalidateRect(hOverlay, NULL, FALSE);
    }
    break;

    case WM_MOUSEMOVE:
    {
        POINT pt = { (short)LOWORD(lParam), (short)HIWORD(lParam) };
        ClientToScreen(hWnd, &pt);

        if (g_isSelecting)
        {
            g_endPoint = pt;
        }

        for (HWND hOverlay : g_hOverlayWnds) InvalidateRect(hOverlay, NULL, FALSE);
    }
    break;

    case WM_LBUTTONUP:
    {
        g_isSelecting = false;
        ReleaseCapture();

        RECT selectionRect = GetSelectionRect();
        HBITMAP hCroppedBitmap = CreateCroppedBitmap(g_hScreenshot, selectionRect);

        CloseAllOverlays();

        if (hCroppedBitmap)
        {
            CopyBitmapToClipboard(hCroppedBitmap);
            PostMessage(g_hMainWnd, WM_APP_SHOW_NOTIFICATION, NOTIFY_COPY_SUCCESS, 0);
            DeleteObject(hCroppedBitmap);
        }
    }
    break;

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) CloseAllOverlays();
        break;

    case WM_RBUTTONDOWN:
        CloseAllOverlays();
        break;

    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

HBITMAP TakeFullscreenScreenshot()
{
    int screenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    int screenX = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int screenY = GetSystemMetrics(SM_YVIRTUALSCREEN);

    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, screenWidth, screenHeight);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);

    BitBlt(hdcMem, 0, 0, screenWidth, screenHeight, hdcScreen, screenX, screenY, SRCCOPY);

    SelectObject(hdcMem, hOldBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);
    return hBitmap;
}

RECT GetSelectionRect()
{
    RECT rc = { 0 };
    rc.left = min(g_startPoint.x, g_endPoint.x);
    rc.top = min(g_startPoint.y, g_endPoint.y);
    rc.right = max(g_startPoint.x, g_endPoint.x);
    rc.bottom = max(g_startPoint.y, g_endPoint.y);
    return rc;
}

HBITMAP CreateCroppedBitmap(HBITMAP hSrcBitmap, RECT rcCrop)
{
    int width = rcCrop.right - rcCrop.left;
    int height = rcCrop.bottom - rcCrop.top;

    if (width <= 0 || height <= 0) return NULL;

    int virtualX = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int virtualY = GetSystemMetrics(SM_YVIRTUALSCREEN);

    HDC hdcSrc = CreateCompatibleDC(NULL);
    HBITMAP hOldSrcBitmap = (HBITMAP)SelectObject(hdcSrc, hSrcBitmap);

    HDC hdcDest = CreateCompatibleDC(NULL);
    HBITMAP hDestBitmap = CreateCompatibleBitmap(hdcSrc, width, height);
    HBITMAP hOldDestBitmap = (HBITMAP)SelectObject(hdcDest, hDestBitmap);

    BitBlt(hdcDest, 0, 0, width, height, hdcSrc, rcCrop.left - virtualX, rcCrop.top - virtualY, SRCCOPY);

    SelectObject(hdcSrc, hOldSrcBitmap);
    SelectObject(hdcDest, hOldDestBitmap);
    DeleteDC(hdcSrc);
    DeleteDC(hdcDest);

    return hDestBitmap;
}
