#pragma once
#include "UIImage.h"

class UIButton : public UIImage
{
private:
	bool m_isHovered = false;
	bool m_isPressed = false;
	bool m_wasLeftDown = false;
	std::function<void()> m_onClick = nullptr;
	D3DCOLOR m_normalColor = D3DCOLOR_ARGB(255, 255, 255, 255);
	D3DCOLOR m_hoverColor = D3DCOLOR_ARGB(255, 230, 230, 230);
	D3DCOLOR m_pressedColor = D3DCOLOR_ARGB(255, 180, 180, 180);

public:
	void Update() override;

	void SetOnClick(std::function<void()> onClick);
	void SetColor(D3DCOLOR color);
	void SetStateColors(D3DCOLOR normalColor, D3DCOLOR hoverColor, D3DCOLOR pressedColor);
	bool IsHovered() const;
	bool IsPressed() const;
};
