#include "pch.h"
#include "UIElement.h"

void UIElement::Initialize()
{
}

void UIElement::Release()
{
}

void UIElement::Start()
{
}

void UIElement::Update()
{
}

void UIElement::RenderUI()
{
}

void UIElement::SetPosition(const D3DXVECTOR2& position)
{
	m_position = position;
	ApplyTransform();
}

D3DXVECTOR2 UIElement::GetPosition() const
{
	return m_position;
}

void UIElement::SetSize(const D3DXVECTOR2& size)
{
	m_size = size;
	ApplyTransform();
}

D3DXVECTOR2 UIElement::GetSize() const
{
	return m_size;
}

void UIElement::SetVisible(bool visible)
{
	m_visible = visible;
}

bool UIElement::IsVisible() const
{
	return m_visible;
}

void UIElement::SetEnabled(bool enabled)
{
	m_enabled = enabled;
}

bool UIElement::IsEnabled() const
{
	return m_enabled;
}

void UIElement::SetOrderInLayer(int orderInLayer)
{
	m_orderInLayer = orderInLayer;
}

int UIElement::GetOrderInLayer() const
{
	return m_orderInLayer;
}

RECT UIElement::GetRect() const
{
	RECT rect =
	{
		static_cast<LONG>(m_position.x),
		static_cast<LONG>(m_position.y),
		static_cast<LONG>(m_position.x + m_size.x),
		static_cast<LONG>(m_position.y + m_size.y)
	};
	return rect;
}

bool UIElement::ContainsPoint(const D3DXVECTOR2& point) const
{
	RECT rect = GetRect();
	POINT winPoint =
	{
		static_cast<LONG>(point.x),
		static_cast<LONG>(point.y)
	};
	return PtInRect(&rect, winPoint) == TRUE;
}

void UIElement::ApplyTransform()
{
	if (m_gameObj)
	{
		m_gameObj->SetPosition(D3DXVECTOR3(m_position.x + (m_size.x * 0.5f), m_position.y + (m_size.y * 0.5f), 0.0f));
		m_gameObj->Size3D() = D3DXVECTOR3(m_size.x, m_size.y, 1.0f);
	}
}
