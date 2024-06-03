#include "pch.h"
#include "ColorButton.h"
void ColorButton::Initialize()
{
    m_gameObj->SetTag("UI_ColorBtn");
    m_curColor = m_defaultColor;
}

void ColorButton::Release()
{
}

void ColorButton::Start()
{
}

void ColorButton::Update()
{
    RECT clientRect;
    GetClientRect(WindowFrame::GetInstance()->GetHWND(), &clientRect);

    // 버튼의 위치를 UIPos에 따라 설정합니다.
    m_gameObj->SetPosition(m_UIPos);

    // 버튼의 위치를 계산합니다.
    RECT rect = {
        static_cast<LONG>(m_UIPos.x),
        static_cast<LONG>(m_UIPos.y),
        static_cast<LONG>(m_UIPos.x + m_UISize.x),
        static_cast<LONG>(m_UIPos.y + m_UISize.y)
    };

    // 마우스 포인터가 버튼 영역 안에 있는지 확인하고, 상태를 업데이트합니다.
    POINT point = { Mouse::GetInstance()->GetPos().x, Mouse::GetInstance()->GetPos().y };
    if (!PtInRect(&rect, point))
        m_curColor = m_defaultColor;

    // 버텍스 좌표 설정
    CUSTOMVERTEX vertices[4];
    vertices[0].x = static_cast<FLOAT>(rect.left);
    vertices[0].y = static_cast<FLOAT>(rect.top);
    vertices[0].z = m_UIPos.z; // z 좌표는 변경하지 않습니다.
    vertices[0].color = m_curColor;
    vertices[0].tu = 0.0f;
    vertices[0].tv = 0.0f;

    vertices[1].x = static_cast<FLOAT>(rect.right);
    vertices[1].y = static_cast<FLOAT>(rect.top);
    vertices[1].z = m_UIPos.z;
    vertices[1].color = m_curColor;
    vertices[1].tu = 1.0f;
    vertices[1].tv = 0.0f;

    vertices[2].x = static_cast<FLOAT>(rect.right);
    vertices[2].y = static_cast<FLOAT>(rect.bottom);
    vertices[2].z = m_UIPos.z;
    vertices[2].color = m_curColor;
    vertices[2].tu = 1.0f;
    vertices[2].tv = 1.0f;

    vertices[3].x = static_cast<FLOAT>(rect.left);
    vertices[3].y = static_cast<FLOAT>(rect.bottom);
    vertices[3].z = m_UIPos.z;
    vertices[3].color = m_curColor;
    vertices[3].tu = 0.0f;
    vertices[3].tv = 1.0f;

    // Render the button
    //D3DXMATRIX matWorld;
    //D3DXMatrixTranslation(&matWorld, 0.0f, 0.0f, 0.0f); // 버튼의 위치를 변경하지 않습니다.
    //MainFrame::GetInstance()->GetDevice()->SetTransform(D3DTS_WORLD, &matWorld);
    MainFrame::GetInstance()->GetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
    MainFrame::GetInstance()->GetDevice()->SetFVF(D3DFVF_CUSTOMVERTEX);
    MainFrame::GetInstance()->GetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertices, sizeof(CUSTOMVERTEX));

    // Draw the text
    RECT textRect = rect;
    MainFrame::GetInstance()->GetFont()->DrawText(
        nullptr, m_text.c_str(), -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE, m_textColor);
    MainFrame::GetInstance()->GetDevice()->SetRenderState(D3DRS_LIGHTING, TRUE);
}

void ColorButton::OnLBtnDown()
{
    m_curColor = m_downColor;
}

void ColorButton::OnLBtnUp()
{
    ButtonEvent();
}

void ColorButton::OnRBtnDown()
{
}

void ColorButton::OnRBtnUp()
{
}

void ColorButton::OnMouseHover()
{
    m_curColor = m_hoverColor;

    if (Mouse::GetInstance()->IsLeftDown())
        m_curColor = m_downColor;
}

void ColorButton::ButtonEvent()
{
    if (m_event)
    {
        m_event();
    }
}

void ColorButton::SetUIPos(D3DXVECTOR3 v)
{
    m_UIPos = v;
}

void ColorButton::SetUISize(D3DXVECTOR2 v)
{
    m_UISize = v;
    if (m_gameObj)
        m_gameObj->Size() = m_UISize;
}

void ColorButton::SetDefaultColor(COLORREF col)
{
    m_defaultColor = col;
}

void ColorButton::SetHoverColor(COLORREF col)
{
    m_hoverColor = col;
}

void ColorButton::SetDownColor(COLORREF col)
{
    m_downColor = col;
}

void ColorButton::SetBorderColor(COLORREF col)
{
    m_borderColor = col;
}

void ColorButton::SetTextColor(COLORREF col)
{
    m_textColor = col;
}

void ColorButton::SetTextSize(int val)
{
    m_textSize = val;
}

void ColorButton::SetText(std::wstring str)
{
    m_text = str;
}

void ColorButton::SetEvent(std::function<void()> func)
{
    m_event = func;
}