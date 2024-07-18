#include "pch.h"
#include "StartScene.h"

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance
	, LPSTR lpszCmdParam, int nCmdShow)
{
	{
		ObjectPool<int> objInt;
		objInt.ShowDebug();

		SharedPointer<int> one = objInt.AcquireObject();
		objInt.ShowDebug();

		{
			SharedPointer<int> two = objInt.AcquireObject();
			objInt.ShowDebug();
		}

		objInt.ShowDebug();
	}

	MainFrame::Create(hInstance);
	MainFrame::GetInstance()->Initialize(TARGETFPS, new StartScene());
	int result = MainFrame::GetInstance()->Run();
	MainFrame::Destroy();

	_CrtDumpMemoryLeaks();
	return result;
}