#include "pch.h"
#include "InputString.h"

void InputString::Initialize()
{
	m_hEdit = CreateWindowEx(
        0,                              // �߰� ��Ÿ��
        L"EDIT",                        // Ŭ���� �̸�
        L"",                            // �ؽ�Ʈ
        WS_CHILD | WS_VISIBLE | WS_BORDER,
        10, 10,                         // X, Y ��ġ
        200, 100,                       // �ʺ�, ����
        WindowFrame::GetInstance()->GetHWND(),          // �θ� ������ �ڵ�
        NULL,                           // �޴� �ڵ�
        WindowFrame::GetInstance()->GetHInst(),          // �ν��Ͻ� �ڵ�
        NULL);                          // ���� �Ķ����
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
