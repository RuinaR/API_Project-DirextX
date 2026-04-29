#include "pch.h"
#include "UIElement.h"
#include "SceneJsonUtility.h"

void UIElement::Initialize()
{
	ApplyPendingTransform();
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

void UIElement::InitGameObj(GameObject* obj)
{
	Component::InitGameObj(obj);
	ApplyPendingTransform();
}

void UIElement::SetPosition(const D3DXVECTOR2* position)
{
	if (!position)
	{
		return;
	}

	if (!m_gameObj)
	{
		m_pendingPosition = *position;
		m_hasPendingPosition = true;
		return;
	}

	D3DXVECTOR2 size = GetSize();
	m_gameObj->SetPosition(D3DXVECTOR3(position->x + (size.x * 0.5f), position->y + (size.y * 0.5f), m_gameObj->Position().z));
}

D3DXVECTOR2 UIElement::GetPosition() const
{
	if (!m_gameObj)
	{
		return m_pendingPosition;
	}

	D3DXVECTOR2 size = GetSize();
	D3DXVECTOR3 position = m_gameObj->Position();
	return D3DXVECTOR2(position.x - (size.x * 0.5f), position.y - (size.y * 0.5f));
}

void UIElement::SetLocalOffset(const D3DXVECTOR2* offset)
{
	if (!offset)
	{
		return;
	}

	if (!m_gameObj)
	{
		m_pendingPosition = *offset;
		m_hasPendingPosition = true;
		return;
	}

	GameObject* parent = m_gameObj->GetParent();
	D3DXVECTOR3 basePosition = parent ? parent->Position() : D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_gameObj->SetPosition(D3DXVECTOR3(basePosition.x + offset->x, basePosition.y + offset->y, m_gameObj->Position().z));
}

D3DXVECTOR2 UIElement::GetLocalOffset() const
{
	if (!m_gameObj)
	{
		return m_pendingPosition;
	}

	GameObject* parent = m_gameObj->GetParent();
	D3DXVECTOR3 basePosition = parent ? parent->Position() : D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 position = m_gameObj->Position();
	return D3DXVECTOR2(position.x - basePosition.x, position.y - basePosition.y);
}

void UIElement::SetSize(const D3DXVECTOR2* size)
{
	if (!size)
	{
		return;
	}

	if (!m_gameObj)
	{
		m_pendingSize = *size;
		m_hasPendingSize = true;
		return;
	}

	D3DXVECTOR2 position = GetPosition();
	m_gameObj->Size3D() = D3DXVECTOR3(size->x, size->y, 1.0f);
	m_gameObj->SetPosition(D3DXVECTOR3(position.x + (size->x * 0.5f), position.y + (size->y * 0.5f), m_gameObj->Position().z));
}

D3DXVECTOR2 UIElement::GetSize() const
{
	if (!m_gameObj)
	{
		return m_pendingSize;
	}

	return m_gameObj->Size2D();
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
	D3DXVECTOR2 position = GetPosition();
	D3DXVECTOR2 size = GetSize();
	RECT rect =
	{
		static_cast<LONG>(position.x),
		static_cast<LONG>(position.y),
		static_cast<LONG>(position.x + size.x),
		static_cast<LONG>(position.y + size.y)
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
	D3DXVECTOR2 size = GetSize();
	D3DXVECTOR2 localOffset = GetLocalOffset();
	bool visible = m_visible;
	bool enabled = m_enabled;
	int orderInLayer = m_orderInLayer;

	if (ImGui::DragFloat2("Local Offset", &localOffset.x, 1.0f))
	{
		SetLocalOffset(&localOffset);
	}
	if (ImGui::DragFloat2("Size", &size.x, 1.0f, 0.0f, 10000.0f))
	{
		SetSize(&size);
	}
	if (ImGui::Checkbox("Visible", &visible))
	{
		SetVisible(visible);
	}
	if (ImGui::Checkbox("Enabled", &enabled))
	{
		SetEnabled(enabled);
	}
	if (ImGui::DragInt("Order In Layer", &orderInLayer))
	{
		SetOrderInLayer(orderInLayer);
	}
}

std::string UIElement::Serialize() const
{
	D3DXVECTOR2 position = GetPosition();
	D3DXVECTOR2 size = GetSize();

	std::ostringstream oss;
	oss << "{ ";
	oss << SceneJson::WriteVector2("position", &position) << ", ";
	oss << SceneJson::WriteVector2("size", &size) << ", ";
	oss << "\"visible\": " << (m_visible ? "true" : "false") << ", ";
	oss << "\"enabled\": " << (m_enabled ? "true" : "false") << ", ";
	oss << "\"orderInLayer\": " << m_orderInLayer;
	oss << " }";
	return oss.str();
}

bool UIElement::Deserialize(const std::string& componentJson)
{
	D3DXVECTOR2 position = m_pendingPosition;
	D3DXVECTOR2 size = m_pendingSize;
	bool visible = m_visible;
	bool enabled = m_enabled;
	int orderInLayer = m_orderInLayer;

	if (SceneJson::ReadVector2(componentJson, "position", &position))
	{
		m_pendingPosition = position;
		m_hasPendingPosition = true;
	}
	if (SceneJson::ReadVector2(componentJson, "size", &size))
	{
		m_pendingSize = size;
		m_hasPendingSize = true;
	}
	SceneJson::ReadBool(componentJson, "visible", visible);
	SceneJson::ReadBool(componentJson, "enabled", enabled);
	SceneJson::ReadInt(componentJson, "orderInLayer", orderInLayer);

	SetVisible(visible);
	SetEnabled(enabled);
	SetOrderInLayer(orderInLayer);
	ApplyPendingTransform();
	return true;
}

void UIElement::ApplyPendingTransform()
{
	if (!m_gameObj)
	{
		return;
	}

	if (m_hasPendingSize)
	{
		m_gameObj->Size3D() = D3DXVECTOR3(m_pendingSize.x, m_pendingSize.y, 1.0f);
		m_hasPendingSize = false;
	}

	if (m_hasPendingPosition)
	{
		m_hasPendingPosition = false;
		UIElement::SetPosition(&m_pendingPosition);
	}
}
