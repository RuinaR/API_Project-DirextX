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
    WindowFrame::GetInstance()->Initialize();
    m_hWnd = WindowFrame::GetInstance()->GetHWND();
    m_width = MAXWINDOWW;
    m_height = MAXWINDOWH;

    if (NULL == (m_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
        return ;

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory(&d3dpp, sizeof(d3dpp));

    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow = m_hWnd;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dpp.BackBufferWidth = m_width;
    d3dpp.BackBufferHeight = m_height;
    d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;


    if (FAILED(m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hWnd,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING,
        &d3dpp, &m_pd3dDevice)))
    {
        return ;
    }

    // Turn off culling, so we see the front and back of the triangle
    m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

    // Turn off D3D lighting, since we are providing our own vertex colors
    m_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
    m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
    m_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

    RECT rect;
    D3DVIEWPORT9 vp;
    GetClientRect(m_hWnd, &rect);
    //int offset = 50;
    vp.X = 0;
    vp.Y = 0;
    vp.Width = rect.right - rect.left;// +offset;
    vp.Height = rect.bottom - rect.top;// +offset;
    vp.MinZ = 0.0f;
    vp.MaxZ = 1.0f;

    Camera::GetInstance()->InitializeView();

    //float aspectRatio = (float)vp.Width / (float)vp.Height;
    //D3DXMatrixPerspectiveFovLH(&m_matProj, D3DX_PI / 4, aspectRatio, 1.0f, 1000.0f);
    D3DXMatrixOrthoLH(&m_matProj, MAXWINDOWW, MAXWINDOWH, 1.0f, 1000.0f);
    m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &m_matProj);
    m_pd3dDevice->SetViewport(&vp);

    D3DXCreateFont(m_pd3dDevice, 30, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        TEXT("Arial"), &m_pFont);

	m_targetFPS = targetFPS;
    
    Mouse::GetInstance()->Initialize();
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

    ShowWindow(m_hWnd, SW_SHOWDEFAULT);
    UpdateWindow(m_hWnd);
    while (TRUE) 
    {
        while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))  // 메시지 처리 우선
        {         
            if (Message.message == WM_QUIT)
            {
                Release();
                return (int)Message.wParam;
            }    
            WindowFrame::GetInstance()->Run(&Message);
		}

		if (WindowFrame::GetInstance()->IsFocus())
		{
			m_timer.tick();
			if (m_timer.getTotalDeltaTime() >= targetFrameTime)
			{
				frameCount++;
				fpsCheckTime += m_timer.getTotalDeltaTime();

				if (NULL == m_pd3dDevice)
					return -1;

				m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

				if (SUCCEEDED(m_pd3dDevice->BeginScene()))
				{
					//UPDATE
					ObjectManager::GetInstance()->Update();
					CollisionManager::GetInstance()->Update();

					//RENDER
					InvalidateRect(WindowFrame::GetInstance()->GetHWND(), NULL, FALSE);
					UpdateWindow(WindowFrame::GetInstance()->GetHWND());
					m_pd3dDevice->EndScene();
				}
				m_pd3dDevice->Present(NULL, NULL, NULL, NULL);

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

ID3DXFont* MainFrame::GetFont()
{
    return m_pFont;
}

LPDIRECT3DDEVICE9 MainFrame::GetDevice()
{
    return m_pd3dDevice;
}

void MainFrame::Release()
{
	ObjectManager::GetInstance()->Release();
	ObjectManager::Destroy();
	CollisionManager::Destroy();

    if (m_pd3dDevice != NULL)
        m_pd3dDevice->Release();

    if (m_pD3D != NULL)
        m_pD3D->Release();

    if (m_pFont != NULL)
        m_pFont->Release();
}
