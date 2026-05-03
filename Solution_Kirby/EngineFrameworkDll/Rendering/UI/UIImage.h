#pragma once
#include "../../EngineFrameworkAPI.h"
#include "UIElement.h"

class ImageRender;

class ENGINEFRAMEWORK_API UIImage : public UIElement
{
protected:
	ImageRender* m_imageRender = nullptr;
	IDirect3DTexture9* m_texture = nullptr;
	std::string m_texturePath;

	void ApplyRenderState();

public:
	void Initialize() override;
	void Release() override;

	void SetTexture(IDirect3DTexture9* texture);
	void SetTexture(const std::string& path);
	void SetTexture(const std::wstring& path);
	void SetUseTexture(bool useTexture);
	void SetColor(D3DCOLOR color);
	D3DCOLOR GetColor() const;
	bool IsUseTexture() const;
	const std::string& GetTexturePath() const;
	ImageRender* GetImageRender();

	void SetPosition(const D3DXVECTOR2* position);
	void SetLocalOffset(const D3DXVECTOR2* offset) override;
	void SetSize(const D3DXVECTOR2* size);
	void SetVisible(bool visible);
	void SetOrderInLayer(int orderInLayer);
	const char* GetInspectorName() const override;
	void DrawInspector() override;
	const char* GetSerializableType() const override;
	std::string Serialize() const override;
	bool Deserialize(const std::string& componentJson) override;
};
