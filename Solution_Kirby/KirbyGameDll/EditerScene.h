#pragma once
#include "Scene.h"
#include "Edit.h"
#include "InputString.h"
class UIButton;
class EditerScene : public Scene
{
private:
	IDirect3DTexture9* m_bg = NULL;
	Edit* m_edit = nullptr;
	InputString* m_input = nullptr;
	UIButton* m_btn = nullptr;
	void StartEdit();
public:
	void Init() override;
	void Release() override;
	void Start() override;
	const char* GetSceneName() const override;
	void BuildInitialSceneObjects() override;
};

