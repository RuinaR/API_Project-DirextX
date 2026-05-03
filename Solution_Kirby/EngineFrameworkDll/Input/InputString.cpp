#include "pch.h"
#include "InputString.h"
#include "UIImage.h"
#include "UILabel.h"
#include "Mouse.h"

void InputString::Initialize()
{
	CreateUI();
}

void InputString::Release()
{
}

void InputString::Start()
{
}

void InputString::Update()
{
	UpdateFocus();

	if (m_isFocused)
	{
		UpdateTextInput();
	}

	RefreshLabel();
}

string InputString::GetString()
{
	return m_text;
}

void InputString::CreateUI()
{
	GameObject* backgroundObj = new GameObject();
	m_background = new UIImage();
	backgroundObj->AddComponent(m_background);
	backgroundObj->InitializeSet();
	backgroundObj->SetParent(m_gameObj);
	m_background->SetPosition(&m_position);
	m_background->SetSize(&m_size);
	m_background->SetUseTexture(false);
	m_background->SetColor(D3DCOLOR_ARGB(230, 245, 245, 245));
	m_background->SetOrderInLayer(20);

	GameObject* labelObj = new GameObject();
	m_label = new UILabel();
	labelObj->AddComponent(m_label);
	labelObj->InitializeSet();
	labelObj->SetParent(m_gameObj);
	D3DXVECTOR2 labelPosition(m_position.x + 10.0f, m_position.y + 8.0f);
	D3DXVECTOR2 labelSize(m_size.x - 20.0f, m_size.y - 12.0f);
	m_label->SetPosition(&labelPosition);
	m_label->SetSize(&labelSize);
	m_label->SetColor(D3DCOLOR_ARGB(255, 30, 30, 30));
	m_label->SetFontSize(18);
	m_label->SetOrderInLayer(30);
	RefreshLabel();
}

void InputString::UpdateFocus()
{
	D3DXVECTOR2 mousePos = Mouse::GetInstance()->GetGameViewPos();
	const bool isLeftDown = Mouse::GetInstance()->IsLeftDown();
	if (isLeftDown && !m_wasLeftDown)
	{
		RECT rect =
		{
			static_cast<LONG>(m_position.x),
			static_cast<LONG>(m_position.y),
			static_cast<LONG>(m_position.x + m_size.x),
			static_cast<LONG>(m_position.y + m_size.y)
		};
		POINT point =
		{
			static_cast<LONG>(mousePos.x),
			static_cast<LONG>(mousePos.y)
		};
		m_isFocused = PtInRect(&rect, point) == TRUE;
	}
	m_wasLeftDown = isLeftDown;

	if (m_background)
	{
		m_background->SetColor(m_isFocused ? D3DCOLOR_ARGB(245, 255, 255, 255) : D3DCOLOR_ARGB(230, 245, 245, 245));
	}
}

void InputString::UpdateTextInput()
{
	const bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
	const bool capsLock = (GetKeyState(VK_CAPITAL) & 0x0001) != 0;
	const bool upperCase = shift != capsLock;

	for (int key = 'A'; key <= 'Z'; key++)
	{
		if (IsKeyPressedOnce(key))
		{
			AppendCharacter(static_cast<char>(upperCase ? key : key + ('a' - 'A')));
		}
	}

	for (int key = '0'; key <= '9'; key++)
	{
		if (IsKeyPressedOnce(key))
		{
			AppendCharacter(static_cast<char>(key));
		}
	}

	if (IsKeyPressedOnce(VK_OEM_MINUS))
	{
		AppendCharacter(shift ? '_' : '-');
	}
	if (IsKeyPressedOnce(VK_OEM_PERIOD))
	{
		AppendCharacter('.');
	}
	if (IsKeyPressedOnce(VK_BACK))
	{
		Backspace();
	}
}

void InputString::RefreshLabel()
{
	if (m_label)
	{
		m_label->SetText(GetDisplayText());
		m_label->SetColor(m_text.empty() ? D3DCOLOR_ARGB(255, 120, 120, 120) : D3DCOLOR_ARGB(255, 30, 30, 30));
	}
}

bool InputString::IsKeyPressedOnce(int virtualKey)
{
	const bool isDown = (GetAsyncKeyState(virtualKey) & 0x8000) != 0;
	const bool pressed = isDown && !m_keyWasDown[virtualKey];
	m_keyWasDown[virtualKey] = isDown;
	return pressed;
}

void InputString::AppendCharacter(char character)
{
	if (m_text.size() < 255)
	{
		m_text.push_back(character);
	}
}

void InputString::Backspace()
{
	if (!m_text.empty())
	{
		m_text.pop_back();
	}
}

std::wstring InputString::GetDisplayText() const
{
	if (m_text.empty())
	{
		return L"Map Name";
	}

	return std::wstring(m_text.begin(), m_text.end());
}
