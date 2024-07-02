#include "pch.h"
#include "ObjectManager.h"
#include "CollisionManager.h"
#include "RenderManager.h"
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
    m_pWorld = new b2World(m_gravity);
    m_pWorld->SetContactListener(&m_cListener);
    m_pWorld->SetContinuousPhysics(true);

    WindowFrame::GetInstance()->Initialize();
    m_hWnd = WindowFrame::GetInstance()->GetHWND();
    m_width = DRAWWINDOWW;
    m_height = DRAWWINDOWH;

    if (NULL == (m_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
        return ;

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory(&d3dpp, sizeof(d3dpp));

    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow = m_hWnd;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;


    if (FAILED(m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hWnd,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING,
        &d3dpp, &m_pd3dDevice)))
    {
        return ;
    }

    m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);

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
    vp.Width = m_width;//rect.right - rect.left;// +offset;
    vp.Height = m_height;//rect.bottom - rect.top;// +offset;
    vp.MinZ = 0.0f;
    vp.MaxZ = 1.0f;

    Camera::GetInstance()->InitializeView();

    //float aspectRatio = (float)vp.Width / (float)vp.Height;
    //D3DXMatrixPerspectiveFovLH(&m_matProj, D3DX_PI / 4, aspectRatio, 1.0f, 1000.0f);
    D3DXMatrixOrthoLH(&m_matProj, DRAWWINDOWW, DRAWWINDOWH, 1.0f, 1000.0f);
    m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &m_matProj);
    m_pd3dDevice->SetViewport(&vp);

    D3DXCreateFont(m_pd3dDevice, 30, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        TEXT("Arial"), &m_pFont);

	m_targetFPS = targetFPS;
    
    Mouse::GetInstance()->Initialize();
	ObjectManager::Create();
    ObjectManager::GetInstance()->Initialize();
    RenderManager::Create();
    RenderManager::GetInstance()->Initialize();
	//CollisionManager::Create();

    m_scene = scene;
    WindowFrame::GetInstance()->SetScene(m_scene);
}

int MainFrame::Run()
{
    MSG Message;
    double targetFrameTime = 1.0 / m_targetFPS;
    m_timer.tick();

    int32 velocityIterations = 8;
    int32 positionIterations = 3;
    ShowWindow(m_hWnd, SW_SHOWDEFAULT);
    UpdateWindow(m_hWnd);


    //imgui test
    //set
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(m_hWnd);
    ImGui_ImplDX9_Init(m_pd3dDevice);

    while (TRUE) 
    {
        while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))  // 메시지 처리 우선
        {         
            WindowFrame::GetInstance()->Run(&Message);
            if (Message.message == WM_QUIT)
            {
                //imgui Cleanup
                ImGui_ImplDX9_Shutdown();
                ImGui_ImplWin32_Shutdown();
                ImGui::DestroyContext();
                //--
                Release();
                return (int)Message.wParam;
            }    
		}

		if (WindowFrame::GetInstance()->IsFocus())
		{
			m_timer.tick();
			if (m_timer.getTotalDeltaTime() >= targetFrameTime)
			{               
				if (NULL == m_pd3dDevice)
					return -1;

                //imgui Update
                ImGui_ImplDX9_NewFrame();
                ImGui_ImplWin32_NewFrame();
                ImGui::NewFrame();
                ImGui::Begin("Frame");
                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
                ImGui::End();

                //UPDATE
                m_pWorld->Step(m_timer.getTotalDeltaTime(), velocityIterations, positionIterations);
				ObjectManager::GetInstance()->Update();
			    //RENDER
                RenderManager::GetInstance()->Update();
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

b2World* MainFrame::GetBox2dWorld()
{
    return m_pWorld;
}

void MainFrame::Release()
{
	ObjectManager::GetInstance()->Release();
	ObjectManager::Destroy();
    RenderManager::GetInstance()->Release();
    RenderManager::Destroy();
	//CollisionManager::Destroy();

    if (m_pd3dDevice != NULL)
        m_pd3dDevice->Release();

    if (m_pD3D != NULL)
        m_pD3D->Release();

    if (m_pFont != NULL)
        m_pFont->Release();

    delete m_pWorld;
}

void CollisionListener::BeginContact(b2Contact* contact)
{
    b2Fixture* fixtureA = contact->GetFixtureA();
    b2Fixture* fixtureB = contact->GetFixtureB();

    BoxCollider* dataA = (BoxCollider*)fixtureA->GetBody()->GetUserData().pointer;
    BoxCollider* dataB = (BoxCollider*)fixtureB->GetBody()->GetUserData().pointer;

    dataA->GetGameObject()->OnCollisionEnter(dataB);
    dataB->GetGameObject()->OnCollisionEnter(dataA);
}

void CollisionListener::EndContact(b2Contact* contact)
{
    b2Fixture* fixtureA = contact->GetFixtureA();
    b2Fixture* fixtureB = contact->GetFixtureB();

	BoxCollider* dataA = (BoxCollider*)fixtureA->GetBody()->GetUserData().pointer;
	BoxCollider* dataB = (BoxCollider*)fixtureB->GetBody()->GetUserData().pointer;
	if (dataA)
		dataA->GetGameObject()->OnCollisionExit(dataB);
	if (dataB)
		dataB->GetGameObject()->OnCollisionExit(dataA);
}

void CollisionListener::PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
{
	b2Fixture* fixtureA = contact->GetFixtureA();
	b2Fixture* fixtureB = contact->GetFixtureB();

	BoxCollider* dataA = (BoxCollider*)fixtureA->GetBody()->GetUserData().pointer;
	BoxCollider* dataB = (BoxCollider*)fixtureB->GetBody()->GetUserData().pointer;

	dataA->GetGameObject()->OnCollisionStay(dataB);
	dataB->GetGameObject()->OnCollisionStay(dataA);
}

void CollisionListener::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
{
}
