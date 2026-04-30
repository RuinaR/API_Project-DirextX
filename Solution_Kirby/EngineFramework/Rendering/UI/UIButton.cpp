#include "pch.h"
#include "UIButton.h"
#include "Mouse.h"
#include "ImageRender.h"
#include "UIActionRegistry.h"
#include "SceneJsonUtility.h"

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
	m_isHovered = ContainsPoint(&mousePos) && RenderManager::GetInstance()->IsTopUIRenderAt(m_imageRender, &mousePos);

	if (m_isHovered && isLeftDown && !m_wasLeftDown)
	{
		m_isPressed = true;
	}

	if (m_isPressed && !isLeftDown)
	{
		const bool shouldClick = m_isHovered && m_onClick;
		m_isPressed = false;

		if (shouldClick)
		{
			m_onClick();
			return;
		}
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

void UIButton::SetActionKey(const std::string& actionKey)
{
	m_actionKey = actionKey;
}

const std::string& UIButton::GetActionKey() const
{
	return m_actionKey;
}

void UIButton::BindActionFromRegistry()
{
	UIActionRegistry::Bind(this, m_actionKey);
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

D3DCOLOR UIButton::GetNormalColor() const
{
	return m_normalColor;
}

D3DCOLOR UIButton::GetHoverColor() const
{
	return m_hoverColor;
}

D3DCOLOR UIButton::GetPressedColor() const
{
	return m_pressedColor;
}

bool UIButton::IsHovered() const
{
	return m_isHovered;
}

bool UIButton::IsPressed() const
{
	return m_isPressed;
}

const char* UIButton::GetInspectorName() const
{
	return "UIButton";
}

void UIButton::DrawInspector()
{
	UIImage::DrawInspector();

	const std::vector<std::string> registeredActionKeys = UIActionRegistry::GetRegisteredActionKeys();
	if (!registeredActionKeys.empty())
	{
		const char* comboPreview = m_actionKey.empty() ? "(None)" : m_actionKey.c_str();
		if (ImGui::BeginCombo("Action Key List", comboPreview))
		{
			const bool isNoneSelected = m_actionKey.empty();
			if (ImGui::Selectable("(None)", isNoneSelected))
			{
				SetActionKey("");
				BindActionFromRegistry();
			}

			if (isNoneSelected)
			{
				ImGui::SetItemDefaultFocus();
			}

			for (std::vector<std::string>::const_iterator itr = registeredActionKeys.begin(); itr != registeredActionKeys.end(); ++itr)
			{
				const bool isSelected = (*itr == m_actionKey);
				if (ImGui::Selectable(itr->c_str(), isSelected))
				{
					SetActionKey(*itr);
					BindActionFromRegistry();
				}

				if (isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
	}
	else
	{
		ImGui::TextDisabled("Registered Action Key: none");
	}

	char actionKey[128] = {};
	strcpy_s(actionKey, m_actionKey.c_str());
	if (ImGui::InputText("Custom Action Key", actionKey, IM_ARRAYSIZE(actionKey)))
	{
		SetActionKey(actionKey);
		BindActionFromRegistry();
	}

	float normalColor[4] =
	{
		static_cast<float>((m_normalColor >> 16) & 0xff) / 255.0f,
		static_cast<float>((m_normalColor >> 8) & 0xff) / 255.0f,
		static_cast<float>(m_normalColor & 0xff) / 255.0f,
		static_cast<float>((m_normalColor >> 24) & 0xff) / 255.0f
	};
	float hoverColor[4] =
	{
		static_cast<float>((m_hoverColor >> 16) & 0xff) / 255.0f,
		static_cast<float>((m_hoverColor >> 8) & 0xff) / 255.0f,
		static_cast<float>(m_hoverColor & 0xff) / 255.0f,
		static_cast<float>((m_hoverColor >> 24) & 0xff) / 255.0f
	};
	float pressedColor[4] =
	{
		static_cast<float>((m_pressedColor >> 16) & 0xff) / 255.0f,
		static_cast<float>((m_pressedColor >> 8) & 0xff) / 255.0f,
		static_cast<float>(m_pressedColor & 0xff) / 255.0f,
		static_cast<float>((m_pressedColor >> 24) & 0xff) / 255.0f
	};

	bool changedColor = false;
	changedColor |= ImGui::ColorEdit4("Normal Color", normalColor);
	changedColor |= ImGui::ColorEdit4("Hover Color", hoverColor);
	changedColor |= ImGui::ColorEdit4("Pressed Color", pressedColor);
	if (changedColor)
	{
		SetStateColors(
			D3DCOLOR_COLORVALUE(normalColor[0], normalColor[1], normalColor[2], normalColor[3]),
			D3DCOLOR_COLORVALUE(hoverColor[0], hoverColor[1], hoverColor[2], hoverColor[3]),
			D3DCOLOR_COLORVALUE(pressedColor[0], pressedColor[1], pressedColor[2], pressedColor[3]));
	}

	ImGui::Text("Hovered: %s", m_isHovered ? "true" : "false");
	ImGui::Text("Pressed: %s", m_isPressed ? "true" : "false");
	ImGui::Text("OnClick: %s", m_onClick ? "Bound" : "None");
}

const char* UIButton::GetSerializableType() const
{
	return "UIButton";
}

std::string UIButton::Serialize() const
{
	D3DXVECTOR2 position = GetPosition();
	D3DXVECTOR2 size = GetSize();

	std::ostringstream oss;
	oss << "{ ";
	oss << "\"position\": { \"x\": " << position.x << ", \"y\": " << position.y << " }, ";
	oss << "\"size\": { \"x\": " << size.x << ", \"y\": " << size.y << " }, ";
	oss << "\"visible\": " << (m_visible ? "true" : "false") << ", ";
	oss << "\"enabled\": " << (m_enabled ? "true" : "false") << ", ";
	oss << "\"orderInLayer\": " << m_orderInLayer << ", ";
	oss << "\"imagePath\": \"" << SceneJson::EscapeString(m_texturePath) << "\", ";
	oss << "\"useTexture\": " << (IsUseTexture() ? "true" : "false") << ", ";
	oss << "\"normalColor\": " << static_cast<DWORD>(m_normalColor) << ", ";
	oss << "\"hoverColor\": " << static_cast<DWORD>(m_hoverColor) << ", ";
	oss << "\"pressedColor\": " << static_cast<DWORD>(m_pressedColor) << ", ";
	oss << "\"actionKey\": \"" << SceneJson::EscapeString(m_actionKey) << "\"";
	oss << " }";
	return oss.str();
}

bool UIButton::Deserialize(const std::string& componentJson)
{
	UIImage::Deserialize(componentJson);

	DWORD normalColor = static_cast<DWORD>(m_normalColor);
	DWORD hoverColor = static_cast<DWORD>(m_hoverColor);
	DWORD pressedColor = static_cast<DWORD>(m_pressedColor);
	std::string actionKey;

	SceneJson::ReadDword(componentJson, "normalColor", normalColor);
	SceneJson::ReadDword(componentJson, "hoverColor", hoverColor);
	SceneJson::ReadDword(componentJson, "pressedColor", pressedColor);
	SceneJson::ReadString(componentJson, "actionKey", actionKey);

	SetStateColors(static_cast<D3DCOLOR>(normalColor), static_cast<D3DCOLOR>(hoverColor), static_cast<D3DCOLOR>(pressedColor));
	SetActionKey(actionKey);
	BindActionFromRegistry();
	return true;
}
