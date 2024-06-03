#pragma once
#include "Scene.h"
#include "Timer.h"
class MainFrame
{
private:
	static MainFrame* m_Pthis;
	Scene* m_scene = nullptr;
	MainFrame() {}
	~MainFrame() {}
	Timer m_timer;
	int m_targetFPS = 60;
public:
	static void Create(HINSTANCE hInstance);
	static MainFrame* GetInstance();
	static void Destroy();
	Timer& Timer();
	double DeltaTime();
	void Initialize(int targetFPS, Scene* scene);
	int Run();
private:
	void Release();
};