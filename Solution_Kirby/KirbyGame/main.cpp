#include "pch.h"
#include "StartScene.h"

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance
	, LPSTR lpszCmdParam, int nCmdShow)
{
	MainFrame::Create(hInstance);
	MainFrame::GetInstance()->Initialize(TARGETFPS, new StartScene(), RenderType::Game);
	int result = MainFrame::GetInstance()->Run();
	MainFrame::Destroy();

	_CrtDumpMemoryLeaks();
	return result;
}