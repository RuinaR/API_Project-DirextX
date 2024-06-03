#pragma once
#include "Scene.h"
#include "StageMaker.h"

class StartScene : public Scene
{
private:
	IDirect3DTexture9* m_bg = NULL;
public:
	void Init() override;
	void Release() override;
	void Start() override;
};

