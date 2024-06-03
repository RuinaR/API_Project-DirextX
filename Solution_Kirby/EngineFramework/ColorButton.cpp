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
	m_gameObj->SetPosition({
		m_UIPos.x + Camera::GetInstance()->GetPos().x,
		m_UIPos.y + Camera::GetInstance()->GetPos().y});

	RECT rect = {
	m_gameObj->Position().x - Camera::GetInstance()->GetPos().x,
	m_gameObj->Position().y - Camera::GetInstance()->GetPos().y,
	m_gameObj->Position().x - Camera::GetInstance()->GetPos().x + m_UISize.x,
	m_gameObj->Position().y - Camera::GetInstance()->GetPos().y + m_UISize.y };
	POINT point = { Mouse::GetInstance()->GetPos().X, Mouse::GetInstance()->GetPos().Y };
	if (!PtInRect(&rect, point))
		m_curColor = m_defaultColor;

	HBRUSH hBrush = CreateSolidBrush(m_curColor);
	HPEN hpen = CreatePen(PS_SOLID, 3, m_borderColor);
	HBRUSH oldBrush = (HBRUSH)SelectObject(WindowFrame::GetInstance()->GetBuffer()->GetHDC(), hBrush);
	HPEN oldPen = (HPEN)SelectObject(WindowFrame::GetInstance()->GetBuffer()->GetHDC(), hpen);
	
	HFONT hFont = CreateFont(m_textSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
		CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("°íµñÃ¼"));
	HFONT oldFont = (HFONT)SelectObject(WindowFrame::GetInstance()->GetBuffer()->GetHDC(), hFont);
	Rectangle(WindowFrame::GetInstance()->GetBuffer()->GetHDC(), rect.left, rect.top, rect.right, rect.bottom);

	
	int centerX = (rect.right - rect.left) / 2;
	int centerY = (rect.bottom - rect.top) / 2;
	COLORREF textColor = ::SetTextColor(WindowFrame::GetInstance()->GetBuffer()->GetHDC(), m_textColor);
	int bkmode = SetBkMode(WindowFrame::GetInstance()->GetBuffer()->GetHDC(), TRANSPARENT);
	DrawText(WindowFrame::GetInstance()->GetBuffer()->GetHDC(), m_text.c_str(), -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	SetBkMode(WindowFrame::GetInstance()->GetBuffer()->GetHDC(), bkmode);
	::SetTextColor(WindowFrame::GetInstance()->GetBuffer()->GetHDC(), textColor);
	SelectObject(WindowFrame::GetInstance()->GetBuffer()->GetHDC(), oldFont);
	SelectObject(WindowFrame::GetInstance()->GetBuffer()->GetHDC(), oldBrush);
	SelectObject(WindowFrame::GetInstance()->GetBuffer()->GetHDC(), oldPen);
	DeleteObject(hFont);
	DeleteObject(hBrush);
	DeleteObject(hpen);
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

void ColorButton::SetUIPos(Vector2D v)
{
	m_UIPos = v;
}

void ColorButton::SetUISize(Vector2D v)
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

void ColorButton::SetText(wstring str)
{
	m_text = str;
}

void ColorButton::SetEvent(function<void()> func)
{
	m_event = func;
}
