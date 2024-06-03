#pragma once
#include "Component.h"

class InputString : public Component
{
private:
	HWND m_hEdit = NULL;
public:
	void Initialize() override;
	void Release() override;
	void Start() override;
	void Update() override;

	string GetString();
	void SetSize(int x, int y, int w, int h);
};

