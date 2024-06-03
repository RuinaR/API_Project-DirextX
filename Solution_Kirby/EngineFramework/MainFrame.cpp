#include "pch.h"
#include "ObjectManager.h"
#include "CollisionManager.h"
#include "DebugWindow.h"
MainFrame* MainFrame::m_Pthis = nullptr;

void MainFrame::Create(HINSTANCE hInstance)
{
	if (!m_Pthis)
	{
		m_Pthis = new MainFrame();
		WindowFrame::Create(hInstance);
		Mouse::Create();
		Camera::Create();
	}
}

MainFrame* MainFrame::GetInstance()
{
	return m_Pthis;
}

void MainFrame::Destroy()
{
	if (m_Pthis)
	{
		Camera::Destroy();
		Mouse::Destroy();
		WindowFrame::Destroy();
		delete m_Pthis;
		m_Pthis = nullptr;
	}
}

Timer& MainFrame::Timer()
{
	return m_timer;
}

double MainFrame::DeltaTime()
{
    //cout << m_timer.getTotalDeltaTime() << endl;
    if (m_timer.getTotalDeltaTime() > 0.02)
        return 0.02f;
	return m_timer.getTotalDeltaTime();
}

void MainFrame::Initialize(int targetFPS, Scene* scene)
{
	m_targetFPS = targetFPS;
    WindowFrame::GetInstance()->Initialize();
	ObjectManager::Create();
	CollisionManager::Create();

	m_scene = scene;
	WindowFrame::GetInstance()->SetScene(m_scene);
}

int MainFrame::Run()
{
    MSG Message;
    double targetFrameTime = 1.0 / m_targetFPS;
    WCHAR strFPS[64];
    int frameCount = 0;
    int fps = 0;
    double fpsCheckTime = 0.0;
    m_timer.tick();  // 최초 시간 초기화

    while (TRUE) 
    {
        while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))  // 메시지 처리 우선
        {         
            WindowFrame::GetInstance()->Run(&Message);
            if (Message.message == WM_QUIT)
            {
                Release();
                return (int)Message.wParam;
            }    
        }

        if (WindowFrame::GetInstance()->IsFocus())
        {
            m_timer.tick();
            if (m_timer.getTotalDeltaTime() >= targetFrameTime)
            {
                frameCount++;         
                fpsCheckTime += m_timer.getTotalDeltaTime();

                //UPDATE
                ObjectManager::GetInstance()->Update();
                CollisionManager::GetInstance()->Update();
                
                //RENDER
                InvalidateRect(WindowFrame::GetInstance()->GetHWND(), NULL, FALSE);
                UpdateWindow(WindowFrame::GetInstance()->GetHWND());

                // FPS 계산
                if (fpsCheckTime > 1.0)
                {
                    fps = frameCount;
                    frameCount = 0;
                    fpsCheckTime = 0.0;
                    swprintf_s(strFPS, _countof(strFPS), L"FPS : %d", fps);
                    SetWindowText(WindowFrame::GetInstance()->GetHWND(), strFPS);
                }
                m_timer.resetTotalDeltaTime(); // 업데이트, 랜더 후 토탈 델타 타임 리셋
            }
        }
    }
}

void MainFrame::Release()
{
	ObjectManager::GetInstance()->Release();
	ObjectManager::Destroy();
	CollisionManager::Destroy();
}
