#include "pch.h"
#include "InputString.h"

void InputString::Initialize()
{
	m_hEdit = CreateWindowEx(
        0,                              // 추가 스타일
        L"EDIT",                        // 클래스 이름
        L"",                            // 텍스트
        WS_CHILD | WS_VISIBLE | WS_BORDER,
        10, 10,                         // X, Y 위치
        200, 100,                       // 너비, 높이
        WindowFrame::GetInstance()->GetHWND(),          // 부모 윈도우 핸들
        NULL,                           // 메뉴 핸들
        WindowFrame::GetInstance()->GetHInst(),          // 인스턴스 핸들
        NULL);                          // 생성 파라미터
}

void InputString::Release()
{
    DestroyWindow(m_hEdit);
}

void InputString::Start()
{
}

void InputString::Update()
{
    
}

string InputString::GetString()
{
    if (!m_hEdit)
        return "";
    int length = GetWindowTextLength(m_hEdit); 
    wstring text;
    if (length > 0)
    {
        wchar_t* buffer = new wchar_t[length + 1];
        GetWindowText(m_hEdit, buffer, length + 1);
        text = buffer;
        delete[] buffer;
    }
    string retval(text.begin(), text.end());
    retval = retval.c_str();
    return retval;
}

void InputString::SetSize(int x, int y, int w, int h)
{
    SetWindowPos(m_hEdit, NULL, x, y, w, h, SWP_NOZORDER);
}
