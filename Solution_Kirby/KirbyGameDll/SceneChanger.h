#pragma once
#include "Component.h"
class SceneChanger : public Component
{
private:
	static SceneChanger* m_Pthis;

public:
	void Initialize() override;
	void Release() override;
	void Start() override;
	void Update() override;

public:
	static SceneChanger* GetInstance();
	static void Create();
	static void Destroy();

public:
	void ChangeGameScene();
	void ChangeStartScene();
	void ChangeEditScene();
};

