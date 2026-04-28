#include "pch.h"
#include "UIImage.h"
#include "ImageRender.h"
#include "TextureManager.h"

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
	SetTexture(TextureManager::GetInstance()->GetTexture(path));
}

void UIImage::SetTexture(const std::wstring& path)
{
	SetTexture(TextureManager::GetInstance()->GetTexture(path));
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

ImageRender* UIImage::GetImageRender()
{
	return m_imageRender;
}

void UIImage::SetPosition(const D3DXVECTOR2& position)
{
	UIElement::SetPosition(position);
}

void UIImage::SetSize(const D3DXVECTOR2& size)
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
	SetPosition(m_position);
	SetSize(m_size);

	if (m_imageRender)
	{
		m_imageRender->SetUIRender(true);
		m_imageRender->SetOrderInLayer(m_orderInLayer);
		m_imageRender->SetRenderEnabled(m_visible);
	}
}
