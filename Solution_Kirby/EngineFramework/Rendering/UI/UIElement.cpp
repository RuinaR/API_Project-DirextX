#include "pch.h"
#include "UIElement.h"
#include "SceneJsonUtility.h"

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

void UIElement::SetPosition(const D3DXVECTOR2* position)
{
	if (!position)
	{
		return;
	}

	m_position = *position;
	ApplyTransform();
}

D3DXVECTOR2 UIElement::GetPosition() const
{
	return m_position;
}

void UIElement::SetSize(const D3DXVECTOR2* size)
{
	if (!size)
	{
		return;
	}

	m_size = *size;
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

bool UIElement::ContainsPoint(const D3DXVECTOR2* point) const
{
	if (!point)
	{
		return false;
	}

	RECT rect = GetRect();
	POINT winPoint =
	{
		static_cast<LONG>(point->x),
		static_cast<LONG>(point->y)
	};
	return PtInRect(&rect, winPoint) == TRUE;
}

const char* UIElement::GetInspectorName() const
{
	return "UIElement";
}

void UIElement::DrawInspector()
{
	ImGui::Text("Position: %.2f, %.2f", m_position.x, m_position.y);
	ImGui::Text("Size: %.2f, %.2f", m_size.x, m_size.y);
	ImGui::Text("Visible: %s", m_visible ? "true" : "false");
	ImGui::Text("Enabled: %s", m_enabled ? "true" : "false");
	ImGui::Text("Order In Layer: %d", m_orderInLayer);
}

std::string UIElement::Serialize() const
{
	std::ostringstream oss;
	oss << "{ ";
	oss << SceneJson::WriteVector2("position", &m_position) << ", ";
	oss << SceneJson::WriteVector2("size", &m_size) << ", ";
	oss << "\"visible\": " << (m_visible ? "true" : "false") << ", ";
	oss << "\"enabled\": " << (m_enabled ? "true" : "false") << ", ";
	oss << "\"orderInLayer\": " << m_orderInLayer;
	oss << " }";
	return oss.str();
}

bool UIElement::Deserialize(const std::string& componentJson)
{
	D3DXVECTOR2 position = m_position;
	D3DXVECTOR2 size = m_size;
	bool visible = m_visible;
	bool enabled = m_enabled;
	int orderInLayer = m_orderInLayer;

	SceneJson::ReadVector2(componentJson, "position", &position);
	SceneJson::ReadVector2(componentJson, "size", &size);
	SceneJson::ReadBool(componentJson, "visible", visible);
	SceneJson::ReadBool(componentJson, "enabled", enabled);
	SceneJson::ReadInt(componentJson, "orderInLayer", orderInLayer);

	SetPosition(&position);
	SetSize(&size);
	SetVisible(visible);
	SetEnabled(enabled);
	SetOrderInLayer(orderInLayer);
	return true;
}

void UIElement::ApplyTransform()
{
	if (m_gameObj)
	{
		m_gameObj->SetPosition(D3DXVECTOR3(m_position.x + (m_size.x * 0.5f), m_position.y + (m_size.y * 0.5f), 0.0f));
		m_gameObj->Size3D() = D3DXVECTOR3(m_size.x, m_size.y, 1.0f);
	}
}
