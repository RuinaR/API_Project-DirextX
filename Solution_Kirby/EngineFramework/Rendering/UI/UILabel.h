#pragma once
#include "UIElement.h"

class UILabel : public UIElement
{
private:
	std::wstring m_text;
	D3DCOLOR m_color = D3DCOLOR_ARGB(255, 255, 255, 255);
	int m_fontSize = 24;
	float m_scale = 1.0f;
	ID3DXFont* m_font = nullptr;

	void RecreateFont();

public:
	void Initialize() override;
	void Release() override;
	void RenderUI() override;

	void SetText(const std::wstring& text);
	void SetText(const wchar_t* text);
	const std::wstring& GetText() const;

	void SetColor(D3DCOLOR color);
	D3DCOLOR GetColor() const;

	void SetFontSize(int fontSize);
	int GetFontSize() const;
	void SetScale(float scale);
	float GetScale() const;

	void SetVisible(bool visible) override;
	void SetOrderInLayer(int orderInLayer) override;
};
