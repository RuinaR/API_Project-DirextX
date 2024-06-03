#pragma once
#include "Scene.h"
#include "StageMaker.h"

class StartScene : public Scene
{
private:
	HBITMAP m_bg = NULL;
public:
	void Init() override;
	void Release() override;
	void Start() override;
};

