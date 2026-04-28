#include "pch.h"
#include "InputString.h"

void InputString::Initialize()
{
	
}

void InputString::Release()
{
  
}

void InputString::Start()
{
}

void InputString::Update()
{
    ImGui::Begin("InputText");

    // InputText 위젯 생성
    ImGui::InputText("Input", m_inputBuffer, IM_ARRAYSIZE(m_inputBuffer));

    ImGui::End();
}

string InputString::GetString()
{
    string str = m_inputBuffer;
    return str;
}

