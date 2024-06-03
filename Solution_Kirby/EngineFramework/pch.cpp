#include "pch.h"

void DrawTextInRect(HDC hdc, const std::wstring& text, const RECT& rect)
{
    if (hdc != NULL)
    {
        // 사각형 내부에 텍스트 출력
        DrawText(hdc, text.c_str(), -1, (LPRECT)&rect, DT_CENTER | DT_WORDBREAK | DT_VCENTER);
    }
}

wstring ConvertToWideString(const std::string& narrowStr)
{
    wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(narrowStr);
}

void FillRectWithColor(HDC hdc, const RECT& rect, COLORREF color)
{
    if (hdc != NULL)
    {
        HBRUSH hBrush = CreateSolidBrush(color);
        if (hBrush != NULL)
        {
            FillRect(hdc, &rect, hBrush);
            DeleteObject(hBrush);
        }
    }
}