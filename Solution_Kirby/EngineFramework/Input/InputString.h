#pragma once
#include "Component.h"

class UIImage;
class UILabel;

class InputString : public Component
{
private:
	string m_text;
	UIImage* m_background = nullptr;
	UILabel* m_label = nullptr;
	D3DXVECTOR2 m_position = D3DXVECTOR2(40.0f, 125.0f);
	D3DXVECTOR2 m_size = D3DXVECTOR2(260.0f, 38.0f);
	bool m_isFocused = true;
	bool m_wasLeftDown = false;
	bool m_keyWasDown[256] = {};

	void CreateUI();
	void UpdateFocus();
	void UpdateTextInput();
	void RefreshLabel();
	bool IsKeyPressedOnce(int virtualKey);
	void AppendCharacter(char character);
	void Backspace();
	std::wstring GetDisplayText() const;

public:
	void Initialize() override;
	void Release() override;
	void Start() override;
	void Update() override;

	string GetString();
};

