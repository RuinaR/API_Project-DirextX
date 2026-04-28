#include "pch.h"
#include "UILabel.h"
#include "RenderManager.h"
#include "MainFrame.h"

void UILabel::Initialize()
{
	UIElement::Initialize();
	RecreateFont();
	if (RenderManager::GetInstance())
	{
		RenderManager::GetInstance()->RegisterUI(this);
	}
}

void UILabel::Release()
{
	if (RenderManager::GetInstance())
	{
		RenderManager::GetInstance()->UnregisterUI(this);
	}

	if (m_font)
	{
		m_font->Release();
		m_font = nullptr;
	}

	UIElement::Release();
}

void UILabel::RenderUI()
{
	if (!m_visible || !m_enabled || !m_font)
	{
		return;
	}

	RECT rect = GetRect();
	m_font->DrawTextW(nullptr, m_text.c_str(), -1, &rect, DT_LEFT | DT_TOP | DT_NOCLIP, m_color);
}

void UILabel::SetText(const std::wstring& text)
{
	m_text = text;
}

void UILabel::SetText(const wchar_t* text)
{
	m_text = text ? text : L"";
}

const std::wstring& UILabel::GetText() const
{
	return m_text;
}

void UILabel::SetColor(D3DCOLOR color)
{
	m_color = color;
}

D3DCOLOR UILabel::GetColor() const
{
	return m_color;
}

void UILabel::SetFontSize(int fontSize)
{
	if (fontSize <= 0)
	{
		fontSize = 1;
	}

	m_fontSize = fontSize;
	RecreateFont();
}

int UILabel::GetFontSize() const
{
	return m_fontSize;
}

void UILabel::SetScale(float scale)
{
	if (scale <= 0.0f)
	{
		scale = 1.0f;
	}

	m_scale = scale;
	RecreateFont();
}

float UILabel::GetScale() const
{
	return m_scale;
}

void UILabel::SetVisible(bool visible)
{
	UIElement::SetVisible(visible);
}

void UILabel::SetOrderInLayer(int orderInLayer)
{
	UIElement::SetOrderInLayer(orderInLayer);
	if (RenderManager::GetInstance())
	{
		RenderManager::GetInstance()->RefreshUIOrder(this);
	}
}

void UILabel::RecreateFont()
{
	if (m_font)
	{
		m_font->Release();
		m_font = nullptr;
	}

	LPDIRECT3DDEVICE9 device = MainFrame::GetInstance()->GetDevice();
	if (!device)
	{
		return;
	}

	const int fontHeight = static_cast<int>(m_fontSize * m_scale);
	D3DXCreateFontW(device, fontHeight, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		L"Malgun Gothic", &m_font);
}
