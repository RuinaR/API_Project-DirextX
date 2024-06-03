#pragma once
#include "Component.h"
class WindowFrame;

class BitmapRender : public Component
{
private:
	HBITMAP m_bit = { 0 };

	void DrawBitmap(HDC hdc, int x, int y, int w, int h, HBITMAP hbit, UINT transparentColor);

public:
	BitmapRender(HBITMAP hbit);

	void Initialize() override;
	void Release() override;
	void ChangeBitmap(HBITMAP hbit);
	void Start() override;
	void Update() override;
};

