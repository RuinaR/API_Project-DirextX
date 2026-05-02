#include "pch.h"
#include "UIElement.h"
#include "RenderManager.h"
#include "SceneDataManager.h"
#include "SceneJsonUtility.h"

namespace
{
	const char* kUIAnchorPresetNames[] =
	{
		"TopLeft",
		"TopCenter",
		"TopRight",
		"MiddleLeft",
		"MiddleCenter",
		"MiddleRight",
		"BottomLeft",
		"BottomCenter",
		"BottomRight",
	};
}

void UIElement::Initialize()
{
	ApplyPendingTransform();
}

void UIElement::Release()
{
}

void UIElement::Start()
{
	ApplyPendingTransform();
}

void UIElement::Update()
{
	RefreshAnchoredPosition();
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

	const D3DXVECTOR2 anchorPoint = GetAnchorPoint();
	const D3DXVECTOR2 size = GetSize();
	const D3DXVECTOR2 selfAnchorOffset = GetSelfAnchorOffset(size);
	const D3DXVECTOR2 localOffset(
		position->x - anchorPoint.x + selfAnchorOffset.x,
		position->y - anchorPoint.y + selfAnchorOffset.y);
	SetLocalOffset(&localOffset);
}

D3DXVECTOR2 UIElement::GetPosition() const
{
	if (!m_gameObj)
	{
		return m_pendingPosition;
	}

	const D3DXVECTOR2 anchorPoint = GetAnchorPoint();
	const D3DXVECTOR2 size = GetSize();
	const D3DXVECTOR2 selfAnchorOffset = GetSelfAnchorOffset(size);
	return D3DXVECTOR2(
		anchorPoint.x + m_localOffset.x - selfAnchorOffset.x,
		anchorPoint.y + m_localOffset.y - selfAnchorOffset.y);
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

	m_localOffset = *offset;
	RefreshAnchoredPosition();
}

D3DXVECTOR2 UIElement::GetLocalOffset() const
{
	if (!m_gameObj)
	{
		return m_pendingPosition;
	}

	return m_localOffset;
}

void UIElement::SetAnchorPreset(UIAnchorPreset anchorPreset)
{
	if (m_anchorPreset == anchorPreset)
	{
		return;
	}

	D3DXVECTOR2 absolutePosition = m_gameObj != nullptr ? GetPosition() : m_pendingPosition;
	m_anchorPreset = anchorPreset;
	if (m_gameObj != nullptr)
	{
		SetPosition(&absolutePosition);
	}
}

UIAnchorPreset UIElement::GetAnchorPreset() const
{
	return m_anchorPreset;
}

void UIElement::RefreshLayout()
{
	RefreshAnchoredPosition();
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

	m_gameObj->Size3D() = D3DXVECTOR3(size->x, size->y, 1.0f);
	RefreshAnchoredPosition();
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

D3DXVECTOR2 UIElement::GetAnchorPoint() const
{
	float left = 0.0f;
	float top = 0.0f;
	float width = static_cast<float>(DEFAULT_WINDOW_CLIENT_WIDTH);
	float height = static_cast<float>(DEFAULT_WINDOW_CLIENT_HEIGHT);
	if (m_gameObj != nullptr)
	{
		GameObject* parent = m_gameObj->GetParent();
		if (parent != nullptr)
		{
			UIElement* parentUIElement = parent->GetComponent<UIElement>();
			if (parentUIElement != nullptr)
			{
				const D3DXVECTOR2 parentPosition = parentUIElement->GetPosition();
				const D3DXVECTOR2 parentSize = parentUIElement->GetSize();
				left = parentPosition.x;
				top = parentPosition.y;
				width = parentSize.x;
				height = parentSize.y;
			}
			else
			{
				const D3DXVECTOR2 parentSize = parent->Size2D();
				const D3DXVECTOR3 parentPosition = parent->Position();
				left = parentPosition.x - (parentSize.x * 0.5f);
				top = parentPosition.y - (parentSize.y * 0.5f);
				width = parentSize.x;
				height = parentSize.y;
			}
		}
		else if (RenderManager::GetInstance() != nullptr)
		{
			const D3DXVECTOR2 uiCanvasSize = RenderManager::GetInstance()->GetUICanvasSize();
			width = uiCanvasSize.x;
			height = uiCanvasSize.y;
		}
	}

	switch (m_anchorPreset)
	{
	case UIAnchorPreset::TopLeft:
		return D3DXVECTOR2(left, top);
	case UIAnchorPreset::TopCenter:
		return D3DXVECTOR2(left + (width * 0.5f), top);
	case UIAnchorPreset::TopRight:
		return D3DXVECTOR2(left + width, top);
	case UIAnchorPreset::MiddleLeft:
		return D3DXVECTOR2(left, top + (height * 0.5f));
	case UIAnchorPreset::MiddleCenter:
		return D3DXVECTOR2(left + (width * 0.5f), top + (height * 0.5f));
	case UIAnchorPreset::MiddleRight:
		return D3DXVECTOR2(left + width, top + (height * 0.5f));
	case UIAnchorPreset::BottomLeft:
		return D3DXVECTOR2(left, top + height);
	case UIAnchorPreset::BottomCenter:
		return D3DXVECTOR2(left + (width * 0.5f), top + height);
	case UIAnchorPreset::BottomRight:
		return D3DXVECTOR2(left + width, top + height);
	default:
		return D3DXVECTOR2(left, top);
	}
}

D3DXVECTOR2 UIElement::GetSelfAnchorOffset(const D3DXVECTOR2& size) const
{
	switch (m_anchorPreset)
	{
	case UIAnchorPreset::TopLeft:
		return D3DXVECTOR2(0.0f, 0.0f);
	case UIAnchorPreset::TopCenter:
		return D3DXVECTOR2(size.x * 0.5f, 0.0f);
	case UIAnchorPreset::TopRight:
		return D3DXVECTOR2(size.x, 0.0f);
	case UIAnchorPreset::MiddleLeft:
		return D3DXVECTOR2(0.0f, size.y * 0.5f);
	case UIAnchorPreset::MiddleCenter:
		return D3DXVECTOR2(size.x * 0.5f, size.y * 0.5f);
	case UIAnchorPreset::MiddleRight:
		return D3DXVECTOR2(size.x, size.y * 0.5f);
	case UIAnchorPreset::BottomLeft:
		return D3DXVECTOR2(0.0f, size.y);
	case UIAnchorPreset::BottomCenter:
		return D3DXVECTOR2(size.x * 0.5f, size.y);
	case UIAnchorPreset::BottomRight:
		return D3DXVECTOR2(size.x, size.y);
	default:
		return D3DXVECTOR2(0.0f, 0.0f);
	}
}

const char* UIElement::AnchorPresetToString(UIAnchorPreset anchorPreset)
{
	const int index = static_cast<int>(anchorPreset);
	if (index < 0 || index >= static_cast<int>(IM_ARRAYSIZE(kUIAnchorPresetNames)))
	{
		return kUIAnchorPresetNames[0];
	}

	return kUIAnchorPresetNames[index];
}

UIAnchorPreset UIElement::AnchorPresetFromString(const std::string& anchorPreset)
{
	for (int i = 0; i < static_cast<int>(IM_ARRAYSIZE(kUIAnchorPresetNames)); ++i)
	{
		if (anchorPreset == kUIAnchorPresetNames[i])
		{
			return static_cast<UIAnchorPreset>(i);
		}
	}

	return UIAnchorPreset::TopLeft;
}

void UIElement::RefreshAnchoredPosition()
{
	if (m_gameObj == nullptr)
	{
		return;
	}

	const D3DXVECTOR2 anchorPoint = GetAnchorPoint();
	const D3DXVECTOR2 size = GetSize();
	const D3DXVECTOR2 selfAnchorOffset = GetSelfAnchorOffset(size);
	m_gameObj->SetPosition(D3DXVECTOR3(
		anchorPoint.x + m_localOffset.x - selfAnchorOffset.x + (size.x * 0.5f),
		anchorPoint.y + m_localOffset.y - selfAnchorOffset.y + (size.y * 0.5f),
		m_gameObj->Position().z));
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
	int anchorPreset = static_cast<int>(m_anchorPreset);

	if (ImGui::DragFloat2("Local Offset", &localOffset.x, 1.0f))
	{
		SetLocalOffset(&localOffset);
	}
	if (ImGui::Combo("Anchor Preset", &anchorPreset, kUIAnchorPresetNames, IM_ARRAYSIZE(kUIAnchorPresetNames)))
	{
		SetAnchorPreset(static_cast<UIAnchorPreset>(anchorPreset));
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
	D3DXVECTOR2 localOffset = GetLocalOffset();
	D3DXVECTOR2 size = GetSize();

	std::ostringstream oss;
	oss << "{ ";
	oss << SceneJson::WriteVector2("position", &localOffset) << ", ";
	oss << SceneJson::WriteVector2("size", &size) << ", ";
	oss << "\"visible\": " << (m_visible ? "true" : "false") << ", ";
	oss << "\"enabled\": " << (m_enabled ? "true" : "false") << ", ";
	oss << "\"orderInLayer\": " << m_orderInLayer << ", ";
	oss << "\"anchorPreset\": \"" << AnchorPresetToString(m_anchorPreset) << "\"";
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
	std::string anchorPreset = AnchorPresetToString(m_anchorPreset);

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
	SceneJson::ReadString(componentJson, "anchorPreset", anchorPreset);

	SetVisible(visible);
	SetEnabled(enabled);
	SetOrderInLayer(orderInLayer);
	m_anchorPreset = AnchorPresetFromString(anchorPreset);
	if (m_hasPendingPosition && SceneDataManager::GetCurrentLoadingSceneVersion() <= 5)
	{
		const D3DXVECTOR2 selfAnchorOffset = GetSelfAnchorOffset(size);
		m_pendingPosition.x += selfAnchorOffset.x;
		m_pendingPosition.y += selfAnchorOffset.y;
	}
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
		m_localOffset = m_pendingPosition;
		m_hasPendingPosition = false;
	}

	RefreshAnchoredPosition();
}
