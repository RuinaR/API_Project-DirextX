#pragma once
#include "Component.h"

class InputString : public Component
{
private:
	char m_inputBuffer[256] = "";
public:
	void Initialize() override;
	void Release() override;
	void Start() override;
	void Update() override;

	string GetString();
};

