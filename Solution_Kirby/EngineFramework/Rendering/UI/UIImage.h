#pragma once
#include "UIElement.h"

class ImageRender;

class UIImage : public UIElement
{
protected:
	ImageRender* m_imageRender = nullptr;
	IDirect3DTexture9* m_texture = nullptr;

	void ApplyRenderState();

public:
	void Initialize() override;
	void Release() override;

	void SetTexture(IDirect3DTexture9* texture);
	void SetTexture(const std::string& path);
	void SetTexture(const std::wstring& path);
	void SetUseTexture(bool useTexture);
	void SetColor(D3DCOLOR color);
	ImageRender* GetImageRender();

	void SetPosition(const D3DXVECTOR2& position);
	void SetSize(const D3DXVECTOR2& size);
	void SetVisible(bool visible);
	void SetOrderInLayer(int orderInLayer);
};
