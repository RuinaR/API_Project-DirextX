#pragma once
#include "Scene.h"
#include "StageMaker.h"
#include "InputString.h"
class UIButton;
class GameScene : public Scene
{
private:
	IDirect3DTexture9* m_bg = NULL;
	InputString* m_input = nullptr;
	UIButton* m_btn = nullptr;

	void StartGame();
public:
	void Init() override;
	void Release() override;
	void Start() override;
};

