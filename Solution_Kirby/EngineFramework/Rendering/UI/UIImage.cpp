#include "pch.h"
#include "UIImage.h"
#include "ImageRender.h"
#include "TextureManager.h"
#include "SceneJsonUtility.h"

void UIImage::Initialize()
{
	UIElement::Initialize();

	m_imageRender = new ImageRender(m_texture);
	m_imageRender->InitGameObj(m_gameObj);
	m_imageRender->Initialize();

	ApplyRenderState();
}

void UIImage::Release()
{
	if (m_imageRender)
	{
		m_imageRender->Release();
		delete m_imageRender;
	}
	m_imageRender = nullptr;
	UIElement::Release();
}

void UIImage::SetTexture(IDirect3DTexture9* texture)
{
	m_texture = texture;

	if (m_imageRender)
	{
		m_imageRender->ChangeTexture(m_texture);
	}
}

void UIImage::SetTexture(const std::string& path)
{
	m_texturePath = path;
	SetTexture(TextureManager::GetInstance()->GetTexture(path));
	if (m_imageRender)
	{
		m_imageRender->SetTexturePath(path);
	}
}

void UIImage::SetTexture(const std::wstring& path)
{
	SetTexture(ConvertToString(path));
}

void UIImage::SetUseTexture(bool useTexture)
{
	if (m_imageRender)
	{
		m_imageRender->SetUseTexture(useTexture);
	}
}

void UIImage::SetColor(D3DCOLOR color)
{
	if (m_imageRender)
	{
		m_imageRender->SetColor(color);
	}
}

D3DCOLOR UIImage::GetColor() const
{
	return m_imageRender ? m_imageRender->GetColor() : D3DCOLOR_ARGB(255, 255, 255, 255);
}

bool UIImage::IsUseTexture() const
{
	return m_imageRender ? m_imageRender->IsUseTexture() : true;
}

const std::string& UIImage::GetTexturePath() const
{
	return m_texturePath;
}

ImageRender* UIImage::GetImageRender()
{
	return m_imageRender;
}

void UIImage::SetPosition(const D3DXVECTOR2* position)
{
	UIElement::SetPosition(position);
}

void UIImage::SetSize(const D3DXVECTOR2* size)
{
	UIElement::SetSize(size);
}

void UIImage::SetVisible(bool visible)
{
	UIElement::SetVisible(visible);

	if (m_imageRender)
	{
		m_imageRender->SetRenderEnabled(m_visible);
	}
}

void UIImage::SetOrderInLayer(int orderInLayer)
{
	UIElement::SetOrderInLayer(orderInLayer);

	if (m_imageRender)
	{
		m_imageRender->SetOrderInLayer(m_orderInLayer);
	}
}

void UIImage::ApplyRenderState()
{
	SetPosition(&m_position);
	SetSize(&m_size);

	if (m_imageRender)
	{
		m_imageRender->SetUIRender(true);
		m_imageRender->SetOrderInLayer(m_orderInLayer);
		m_imageRender->SetRenderEnabled(m_visible);
		if (!m_texturePath.empty())
		{
			m_imageRender->SetTexturePath(m_texturePath);
		}
	}
}

const char* UIImage::GetInspectorName() const
{
	return "UIImage";
}

void UIImage::DrawInspector()
{
	UIElement::DrawInspector();
	ImGui::Text("Texture: %s", m_texture ? "Assigned" : "None");
	ImGui::Text("ImageRender: %s", m_imageRender ? "Created" : "None");
	if (m_imageRender)
	{
		ImGui::Text("Use Texture: %s", m_imageRender->IsUseTexture() ? "true" : "false");
		ImGui::Text("Render Enabled: %s", m_imageRender->IsRenderEnabled() ? "true" : "false");
		ImGui::Text("Order In Layer: %d", m_imageRender->GetOrderInLayer());
	}
	ImGui::Text("Texture Path: %s", m_texturePath.empty() ? "(none)" : m_texturePath.c_str());
}

const char* UIImage::GetSerializableType() const
{
	return "UIImage";
}

std::string UIImage::Serialize() const
{
	std::ostringstream oss;
	oss << "{ ";
	oss << "\"position\": { \"x\": " << m_position.x << ", \"y\": " << m_position.y << " }, ";
	oss << "\"size\": { \"x\": " << m_size.x << ", \"y\": " << m_size.y << " }, ";
	oss << "\"visible\": " << (m_visible ? "true" : "false") << ", ";
	oss << "\"enabled\": " << (m_enabled ? "true" : "false") << ", ";
	oss << "\"orderInLayer\": " << m_orderInLayer << ", ";
	oss << "\"useTexture\": " << (IsUseTexture() ? "true" : "false") << ", ";
	oss << "\"color\": " << static_cast<DWORD>(GetColor()) << ", ";
	oss << "\"imagePath\": \"" << SceneJson::EscapeString(m_texturePath) << "\"";
	oss << " }";
	return oss.str();
}

bool UIImage::Deserialize(const std::string& componentJson)
{
	UIElement::Deserialize(componentJson);

	bool useTexture = IsUseTexture();
	DWORD color = static_cast<DWORD>(GetColor());
	std::string imagePath;
	SceneJson::ReadBool(componentJson, "useTexture", useTexture);
	SceneJson::ReadDword(componentJson, "color", color);
	SceneJson::ReadString(componentJson, "imagePath", imagePath);

	if (!imagePath.empty())
	{
		SetTexture(imagePath);
	}
	SetUseTexture(useTexture);
	SetColor(static_cast<D3DCOLOR>(color));
	return true;
}
