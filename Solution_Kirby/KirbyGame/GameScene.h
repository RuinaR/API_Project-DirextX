#pragma once
#include "Scene.h"
#include "StageMaker.h"
#include "InputString.h"
#include "ColorButton.h"
class GameScene : public Scene
{
private:
	IDirect3DTexture9* m_bg = NULL;
	InputString* m_input = nullptr;
	ColorButton* m_btn = nullptr;

	void StartGame();
public:
	void Init() override;
	void Release() override;
	void Start() override;
};

