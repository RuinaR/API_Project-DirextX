#include "pch.h"
#include "Button.h"

void Button::Initialize()
{
    m_gameObj->SetTag("UI_Button");
    RenderManager::GetInstance()->ResisterBtn(this);
}

void Button::Release()
{
    RenderManager::GetInstance()->UnresisterBtn(this);
}

void Button::Start()
{
}

void Button::Update()
{
    if (m_isClicked)
    {
        m_isClicked = false;
        ButtonEvent();
    }

    //// 버튼의 위치를 UIPos에 따라 설정합니다.
    //m_gameObj->SetPosition(m_UIPos);
    //// 버튼의 위치를 계산합니다.
    //RECT rect = {
    //    static_cast<LONG>(m_UIPos.x + Camera::GetInstance()->GetPos().x),
    //    static_cast<LONG>(m_UIPos.y + m_UISize.y + Camera::GetInstance()->GetPos().y),
    //    static_cast<LONG>(m_UIPos.x + m_UISize.x + Camera::GetInstance()->GetPos().x),
    //    static_cast<LONG>(m_UIPos.y + Camera::GetInstance()->GetPos().y)
    //};
    //// 마우스 포인터가 버튼 영역 안에 있는지 확인하고, 상태를 업데이트합니다.
    //POINT point = {
    //    Mouse::GetInstance()->GetDXPos().x + ImGui::GetWindowPos().x + Camera::GetInstance()->GetPos().x,
    //    -Mouse::GetInstance()->GetDXPos().y + ImGui::GetWindowPos().y + Camera::GetInstance()->GetPos().y };
    //if (!PtInRect(&rect, point))
    //    m_curColor = m_defaultColor;
}

void Button::UpdateRender()
{
    if (ImGui::Button(m_text.c_str()))
    {
        m_isClicked = true;
    }

    //RECT rect = {
    //    static_cast<LONG>(m_UIPos.x + Camera::GetInstance()->GetPos().x),
    //    static_cast<LONG>(m_UIPos.y + Camera::GetInstance()->GetPos().y),
    //    static_cast<LONG>(m_UIPos.x + m_UISize.x + Camera::GetInstance()->GetPos().x),
    //    static_cast<LONG>(m_UIPos.y + m_UISize.y + Camera::GetInstance()->GetPos().y)
    //};
    //D3DXVECTOR3 wPos = { (float)rect.left, (float)rect.top, 0.0f };
    //D3DXVECTOR3 sPos = WorldToScreen(MainFrame::GetInstance()->GetDevice(), wPos);

    //// 버텍스 좌표 설정
    //CUSTOMVERTEX vertices[4];
    //vertices[0].x = 0;
    //vertices[0].y = 0;
    //vertices[0].z = 0;
    //vertices[0].color = m_curColor;
    //vertices[0].tu = 0.0f;
    //vertices[0].tv = 1.0f;

    //vertices[1].x = 1;
    //vertices[1].y = 0;
    //vertices[1].z = 0;
    //vertices[1].color = m_curColor;
    //vertices[1].tu = 1.0f;
    //vertices[1].tv = 1.0f;

    //vertices[2].x = 1;
    //vertices[2].y = 1;
    //vertices[2].z = 0;
    //vertices[2].color = m_curColor;
    //vertices[2].tu = 1.0f;
    //vertices[2].tv = 0.0f;

    //vertices[3].x = 0;
    //vertices[3].y = 1;
    //vertices[3].z = 0;
    //vertices[3].color = m_curColor;
    //vertices[3].tu = 0.0f;
    //vertices[3].tv = 0.0f;

    //// Render the button
    //D3DXMATRIX matTr, matScale, matWorld;
    //D3DXMatrixScaling(&matScale, m_UISize.x, m_UISize.y, 1.0f);
    //D3DXMatrixTranslation(&matTr, m_UIPos.x + Camera::GetInstance()->GetPos().x,
    //    m_UIPos.y + Camera::GetInstance()->GetPos().y,
    //    m_UIPos.z);
    //matWorld = matScale * matTr;
    //MainFrame::GetInstance()->GetDevice()->SetTransform(D3DTS_WORLD, &matWorld);
    //MainFrame::GetInstance()->GetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertices, sizeof(CUSTOMVERTEX));
    //// Draw the text

    //ID3DXFont* pFont;
    //D3DXCreateFont(MainFrame::GetInstance()->GetDevice(), m_textSize, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
    //    OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
    //    TEXT("Arial"), &pFont);
    //sPos = { sPos.x + 10, sPos.y - (rect.bottom - rect.top) / 2 , 0.0f };
    //RECT textRect = { sPos.x, sPos.y, sPos.x + 100, sPos.y + 100 };
    //pFont->DrawText(
    //    nullptr, m_text.c_str(), -1, &textRect, DT_NOCLIP, m_textColor);
    //pFont->Release();
}

void Button::ButtonEvent()
{
    if (m_event)
    {
        m_event();
    }
}

void Button::SetUIPos(D3DXVECTOR3 v)
{
    m_UIPos = v;
}

void Button::SetUISize(D3DXVECTOR2 v)
{
    m_UISize = v;
    if (m_gameObj)
        m_gameObj->Size2D() = m_UISize;
}


void Button::SetTextColor(D3DCOLOR col)
{
    m_textColor = col;
}

void Button::SetText(string str)
{
    m_text = str;
}

void Button::SetEvent(std::function<void()> func)
{
    m_event = func;
}