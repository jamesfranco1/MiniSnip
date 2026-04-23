#include "Actions.h"

void CopyBitmapToClipboard(HBITMAP hBitmap)
{
    if (OpenClipboard(g_hMainWnd))
    {
        EmptyClipboard();
        SetClipboardData(CF_BITMAP, hBitmap);
        CloseClipboard();
    }
}
