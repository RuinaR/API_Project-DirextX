#include "pch.h"
#include "ObjectManager.h"
#include "CollisionManager.h"
#include "RenderManager.h"
#include "DebugWindow.h"
#include "..\Imgui\source\imgui_impl_win32.h"
#include "..\Imgui\source\imgui_impl_dx9.h"
#include "Resource/AssetDatabase.h"
#include "Resource/ResourceManager.h"
MainFrame* MainFrame::m_Pthis = nullptr;

void MainFrame::Create(HINSTANCE hInstance)
{
	if (!m_Pthis)
	{
		m_Pthis = new MainFrame();
		WindowFrame::Create(hInstance);
		Mouse::Create();
		Camera::Create();
        AssetDatabase::Create();
        ResourceManager::Create();
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
        m_Pthis->Release();
        ResourceManager::Destroy();
        AssetDatabase::Destroy();
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

namespace
{
	UINT ClampClientSize(LONG value)
	{
		return value > 0 ? static_cast<UINT>(value) : 1u;
	}
}

void MainFrame::Initialize(int targetFPS, Scene* scene, RenderType type)
{
    m_released = false;
    m_pWorld = new b2World(m_gravity);
    m_pWorld->SetContactListener(&m_cListener);
    m_pWorld->SetContinuousPhysics(true);
    m_type = type;
    WindowFrame::GetInstance()->Initialize(m_type);
    if (AssetDatabase::GetInstance() != nullptr)
    {
        AssetDatabase::GetInstance()->Scan();
    }
    m_hWnd = WindowFrame::GetInstance()->GetHWND();
    RECT clientRect = {};
    GetClientRect(m_hWnd, &clientRect);
    m_width = static_cast<int>(ClampClientSize(clientRect.right - clientRect.left));
    m_height = static_cast<int>(ClampClientSize(clientRect.bottom - clientRect.top));

    if (NULL == (m_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
        return ;

    ZeroMemory(&m_d3dpp, sizeof(m_d3dpp));

    m_d3dpp.Windowed = TRUE;
    m_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    m_d3dpp.hDeviceWindow = m_hWnd;
    m_d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
    m_d3dpp.BackBufferWidth = static_cast<UINT>(m_width);
    m_d3dpp.BackBufferHeight = static_cast<UINT>(m_height);
    m_d3dpp.EnableAutoDepthStencil = TRUE;
    m_d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
    m_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;


    if (FAILED(m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hWnd,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING,
        &m_d3dpp, &m_pd3dDevice)))
    {
        return ;
    }

    ApplyDeviceState();

    D3DVIEWPORT9 vp;
    vp.X = 0;
    vp.Y = 0;
    vp.Width = static_cast<DWORD>(m_width);
    vp.Height = static_cast<DWORD>(m_height);
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

    m_targetFrameTime = 1.0 / m_targetFPS;
    m_timer.tick();

    m_velocityIterations = 8;
    m_positionIterations = 3;
    ShowWindow(m_hWnd, SW_SHOWDEFAULT);
    UpdateWindow(m_hWnd);
    
    Mouse::GetInstance()->Initialize();
	ObjectManager::Create();
    ObjectManager::GetInstance()->Initialize();
    RenderManager::Create();
    RenderManager::GetInstance()->Initialize();
	//CollisionManager::Create();

    m_scene = scene;
    WindowFrame::GetInstance()->SetScene(m_scene);

    // ImGui 컨텍스트와 입력 옵션을 설정한다.
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); 
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // 키보드 조작 활성화
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // 게임패드 조작 활성화
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigWindowsResizeFromEdges = false;

    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(m_hWnd);
    ImGui_ImplDX9_Init(m_pd3dDevice);
}

void MainFrame::ProcessMouseInput() {
	// 마우스 클릭 시 메인 윈도우에 포커스를 주기
	if (ImGui::IsMouseClicked(0)) {
		// 메인 윈도우에 포커스를 설정
		SetForegroundWindow(m_hWnd);
	}
}

void MainFrame::AddBtnEvent(std::function<void()> p_event)
{
    m_listBtnEvent.push_back(p_event);
}


//int MainFrame::Run()
//{
//    MSG Message;
//    m_timer.tick();
//
//    int32 velocityIterations = 8;
//    int32 positionIterations = 3;
//    ShowWindow(m_hWnd, SW_SHOWDEFAULT);
//    UpdateWindow(m_hWnd);
//
//    while (TRUE) 
//    {
//        while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))  // 메시지 처리 우선
//        {         
//            WindowFrame::GetInstance()->Run(&Message);
//            if (Message.message == WM_QUIT)
//            {
//                //imgui Cleanup
//                ImGui_ImplDX9_Shutdown();
//                ImGui_ImplWin32_Shutdown();
//                ImGui::DestroyContext();
//                //--
//                Release();
//                return (int)Message.wParam;
//			}
//		}
//
//		m_timer.tick();
//		if (m_timer.getTotalDeltaTime() >= m_targetFrameTime)
//		{
//			if (NULL == m_pd3dDevice)
//				return -1;
//
//			if (m_FrameStartEvent)
//			{
//				m_FrameStartEvent();
//				m_FrameStartEvent = NULL;
//			}
//
//			ImGui_ImplDX9_NewFrame();
//			ImGui_ImplWin32_NewFrame();
//			ImGui::NewFrame();
//			//UPDATE
//			m_pWorld->Step(m_timer.getTotalDeltaTime(), velocityIterations, positionIterations);
//			ObjectManager::GetInstance()->Update();
//			//RENDER
//			if (m_type == RenderType::Edit)
//				RenderManager::GetInstance()->EditUpdate();
//			else if (m_type == RenderType::Game)
//				RenderManager::GetInstance()->GameUpdate();
//
//			m_timer.resetTotalDeltaTime(); // 업데이트, 랜더 후 토탈 델타 타임 리셋
//		}
//	}
//}

//void MainFrame::Set() //테스트용
//{
//    m_targetFrameTime = 1.0 / m_targetFPS;
//    m_timer.tick();
//
//    m_velocityIterations = 8;
//    m_positionIterations = 3;
//    ShowWindow(m_hWnd, SW_SHOWDEFAULT);
//    UpdateWindow(m_hWnd);
//
//
//    //imgui set
//    IMGUI_CHECKVERSION();
//    ImGui::CreateContext();
//    ImGuiIO& io = ImGui::GetIO(); (void)io;
//    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
//    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
//    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
//
//    ImGui::StyleColorsDark();
//    ImGui_ImplWin32_Init(m_hWnd);
//    ImGui_ImplDX9_Init(m_pd3dDevice);
//}

bool MainFrame::Update()
{
    while (PeekMessage(&m_message, 0, 0, 0, PM_REMOVE))  // 메시지 처리 우선
    {
        WindowFrame::GetInstance()->Run(&m_message);
        if (m_message.message == WM_QUIT)
        {
            //imgui Cleanup
            ImGui_ImplDX9_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
            //--
            Release();
            return false;
        }
    }

        if (!EnsureDeviceReady())
        {
            Sleep(1);
            return true;
        }

        m_timer.tick();
        if (m_timer.getTotalDeltaTime() >= m_targetFrameTime)
        {
			if (NULL == m_pd3dDevice)
				return false;

            //버튼이벤트 일괄 처리
            for (auto itr = m_listBtnEvent.begin(); itr != m_listBtnEvent.end();)
            {
                (*itr)();
                itr = m_listBtnEvent.erase(itr);
            }

            //ImGui
			ImGui_ImplDX9_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            //UPDATE
            m_pWorld->Step(m_timer.getTotalDeltaTime(), m_velocityIterations, m_positionIterations);
            ObjectManager::GetInstance()->Update();

            //RENDER
            if (m_type == RenderType::Edit)
                RenderManager::GetInstance()->EditUpdate();
            else if (m_type == RenderType::Game)
                RenderManager::GetInstance()->GameUpdate();

            m_timer.resetTotalDeltaTime(); // 업데이트, 랜더 후 토탈 델타 타임 리셋
        }

    return true;
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

void MainFrame::RequestResize(UINT width, UINT height)
{
    m_pendingResize = true;
    m_pendingResizeWidth = width > 0 ? width : 1u;
    m_pendingResizeHeight = height > 0 ? height : 1u;
}

bool MainFrame::HandleResize(UINT width, UINT height)
{
    if (m_pd3dDevice == nullptr || width == 0 || height == 0)
    {
        return false;
    }

    InvalidateDeviceResources();

    m_width = static_cast<int>(width);
    m_height = static_cast<int>(height);
    m_d3dpp.BackBufferWidth = width;
    m_d3dpp.BackBufferHeight = height;

    HRESULT hr = m_pd3dDevice->Reset(&m_d3dpp);
    if (FAILED(hr))
    {
        return false;
    }

    D3DVIEWPORT9 vp = {};
    vp.X = 0;
    vp.Y = 0;
    vp.Width = width;
    vp.Height = height;
    vp.MinZ = 0.0f;
    vp.MaxZ = 1.0f;
    m_pd3dDevice->SetViewport(&vp);
    m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &m_matProj);
    ApplyDeviceState();
    RestoreDeviceResources();
    m_pendingResize = false;
    return true;
}

void MainFrame::InvalidateDeviceResources()
{
    if (m_deviceResourcesInvalidated)
    {
        return;
    }

    if (m_pFont != nullptr)
    {
        m_pFont->OnLostDevice();
    }

    if (ImGui::GetCurrentContext() != nullptr)
    {
        ImGui_ImplDX9_InvalidateDeviceObjects();
    }
    if (RenderManager::GetInstance() != nullptr)
    {
        RenderManager::GetInstance()->ReleaseRenderTargetTexture();
    }

    m_deviceResourcesInvalidated = true;
}

void MainFrame::RestoreDeviceResources()
{
    if (m_pFont != nullptr)
    {
        m_pFont->OnResetDevice();
    }

    if (RenderManager::GetInstance() != nullptr)
    {
        RenderManager::GetInstance()->CreateRenderTargetTexture();
    }

    if (ImGui::GetCurrentContext() != nullptr)
    {
        ImGui_ImplDX9_CreateDeviceObjects();
    }

    m_deviceResourcesInvalidated = false;
}

void MainFrame::ApplyDeviceState()
{
    if (m_pd3dDevice == nullptr)
    {
        return;
    }

    m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    m_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
    m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
    m_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
}

bool MainFrame::EnsureDeviceReady()
{
    if (m_pd3dDevice == nullptr)
    {
        return false;
    }

    if (m_pendingResize)
    {
        return HandleResize(m_pendingResizeWidth, m_pendingResizeHeight);
    }

    HRESULT hr = m_pd3dDevice->TestCooperativeLevel();
    if (hr == D3DERR_DEVICELOST)
    {
        return false;
    }

    if (hr == D3DERR_DEVICENOTRESET)
    {
        RECT clientRect = {};
        GetClientRect(m_hWnd, &clientRect);
        return HandleResize(
            ClampClientSize(clientRect.right - clientRect.left),
            ClampClientSize(clientRect.bottom - clientRect.top));
    }

    return SUCCEEDED(hr);
}

void MainFrame::Release()
{
    if (m_released)
        return;

    m_released = true;

    if (ObjectManager::GetInstance() != nullptr)
    {
        ObjectManager::GetInstance()->Release();
        ObjectManager::Destroy();
    }
    if (RenderManager::GetInstance() != nullptr)
    {
        RenderManager::GetInstance()->Release();
        RenderManager::Destroy();
    }
	//CollisionManager::Destroy();

    if (m_pFont != NULL)
    {
        m_pFont->Release();
        m_pFont = NULL;
    }

    if (m_pd3dDevice != NULL)
    {
        m_pd3dDevice->Release();
        m_pd3dDevice = NULL;
    }

    if (m_pD3D != NULL)
    {
        m_pD3D->Release();
        m_pD3D = NULL;
    }

    delete m_pWorld;
    m_pWorld = nullptr;
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
