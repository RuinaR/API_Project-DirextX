#pragma once
#include "Scene.h"
#include "Edit.h"
#include "InputString.h"
#include "ColorButton.h"
class EditerScene : public Scene
{
private:
	HBITMAP m_bg = NULL;
	Edit* m_edit = nullptr;
	InputString* m_input = nullptr;
	ColorButton* m_btn = nullptr;
	void StartEdit();
public:
	void Init() override;
	void Release() override;
	void Start() override;
};

