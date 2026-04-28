#include "pch.h"
#include "UILabel.h"
#include "RenderManager.h"
#include "MainFrame.h"
#include "SceneJsonUtility.h"

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

const char* UILabel::GetInspectorName() const
{
	return "UILabel";
}

void UILabel::DrawInspector()
{
	UIElement::DrawInspector();
	std::string text = ConvertToString(m_text);
	ImGui::Text("Text: %s", text.c_str());
	ImGui::Text("Font Size: %d", m_fontSize);
	ImGui::Text("Scale: %.2f", m_scale);
	ImGui::Text("Font: %s", m_font ? "Created" : "None");

	float color[4] =
	{
		static_cast<float>((m_color >> 16) & 0xff) / 255.0f,
		static_cast<float>((m_color >> 8) & 0xff) / 255.0f,
		static_cast<float>(m_color & 0xff) / 255.0f,
		static_cast<float>((m_color >> 24) & 0xff) / 255.0f
	};
	ImGui::ColorButton("Color", ImVec4(color[0], color[1], color[2], color[3]));
}

const char* UILabel::GetSerializableType() const
{
	return "UILabel";
}

std::string UILabel::Serialize() const
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
	oss << "\"text\": \"" << SceneJson::EscapeString(ConvertToString(m_text)) << "\", ";
	oss << "\"color\": " << static_cast<DWORD>(m_color) << ", ";
	oss << "\"fontSize\": " << m_fontSize << ", ";
	oss << "\"scale\": " << m_scale;
	oss << " }";
	return oss.str();
}

bool UILabel::Deserialize(const std::string& componentJson)
{
	UIElement::Deserialize(componentJson);

	std::string text;
	DWORD color = static_cast<DWORD>(m_color);
	int fontSize = m_fontSize;
	float scale = m_scale;

	SceneJson::ReadString(componentJson, "text", text);
	SceneJson::ReadDword(componentJson, "color", color);
	SceneJson::ReadInt(componentJson, "fontSize", fontSize);
	SceneJson::ReadFloat(componentJson, "scale", scale);

	SetText(ConvertToWideString(text));
	SetColor(static_cast<D3DCOLOR>(color));
	SetFontSize(fontSize);
	SetScale(scale);
	return true;
}
