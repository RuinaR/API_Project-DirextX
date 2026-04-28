#include "pch.h"
#include "UIButton.h"
#include "Mouse.h"
#include "ImageRender.h"

void UIButton::Update()
{
	if (!m_enabled || !m_visible)
	{
		m_isHovered = false;
		m_isPressed = false;
		m_wasLeftDown = false;
		return;
	}

	D3DXVECTOR2 mousePos = Mouse::GetInstance()->GetGameViewPos();
	const bool isLeftDown = Mouse::GetInstance()->IsLeftDown();
	m_isHovered = ContainsPoint(mousePos) && RenderManager::GetInstance()->IsTopUIRenderAt(m_imageRender, mousePos);

	if (m_isHovered && isLeftDown && !m_wasLeftDown)
	{
		m_isPressed = true;
	}

	if (m_isPressed && !isLeftDown)
	{
		if (m_isHovered && m_onClick)
		{
			m_onClick();
		}
		m_isPressed = false;
	}

	if (!m_isHovered && !isLeftDown)
	{
		m_isPressed = false;
	}

	if (m_imageRender)
	{
		if (m_isPressed)
		{
			m_imageRender->SetColor(m_pressedColor);
		}
		else if (m_isHovered)
		{
			m_imageRender->SetColor(m_hoverColor);
		}
		else
		{
			m_imageRender->SetColor(m_normalColor);
		}
	}

	m_wasLeftDown = isLeftDown;
}

void UIButton::SetOnClick(std::function<void()> onClick)
{
	m_onClick = onClick;
}

void UIButton::SetColor(D3DCOLOR color)
{
	m_normalColor = color;
	UIImage::SetColor(color);
}

void UIButton::SetStateColors(D3DCOLOR normalColor, D3DCOLOR hoverColor, D3DCOLOR pressedColor)
{
	m_normalColor = normalColor;
	m_hoverColor = hoverColor;
	m_pressedColor = pressedColor;
	UIImage::SetColor(m_normalColor);
}

bool UIButton::IsHovered() const
{
	return m_isHovered;
}

bool UIButton::IsPressed() const
{
	return m_isPressed;
}
