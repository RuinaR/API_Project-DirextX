#pragma once
#include "Scene.h"
#include "Edit.h"
#include "InputString.h"
#include "Button.h"
class EditerScene : public Scene
{
private:
	IDirect3DTexture9* m_bg = NULL;
	Edit* m_edit = nullptr;
	InputString* m_input = nullptr;
	Button* m_btn = nullptr;
	void StartEdit();
public:
	void Init() override;
	void Release() override;
	void Start() override;
};

