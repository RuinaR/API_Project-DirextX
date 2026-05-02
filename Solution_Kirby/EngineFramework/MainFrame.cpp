#include "pch.h"
#include "ObjectManager.h"
#include "RenderManager.h"
#include "DebugWindow.h"
#include "..\Imgui\source\imgui_impl_win32.h"
#include "..\Imgui\source\imgui_impl_dx9.h"
#include "Resource/AssetDatabase.h"
#include "Resource/ResourceManager.h"
#include <unordered_set>
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
	return m_frameDeltaTime;
}

double MainFrame::UnscaledDeltaTime() const
{
	return m_unscaledDeltaTime;
}

float MainFrame::GetTimeScale() const
{
	return m_scene != nullptr ? m_scene->GetTimeScale() : 1.0f;
}

void MainFrame::SetTimeScale(float timeScale)
{
	if (timeScale < 0.0f)
	{
		timeScale = 0.0f;
	}

	if (m_scene != nullptr)
	{
		m_scene->SetTimeScale(timeScale);
	}
}

namespace
{
	struct CollisionPair
	{
		Collider2D* first = nullptr;
		Collider2D* second = nullptr;

		bool operator==(const CollisionPair& other) const
		{
			return first == other.first && second == other.second;
		}
	};

	struct CollisionPairHasher
	{
		size_t operator()(const CollisionPair& pair) const
		{
			const size_t h1 = std::hash<Collider2D*>{}(pair.first);
			const size_t h2 = std::hash<Collider2D*>{}(pair.second);
			return h1 ^ (h2 << 1);
		}
	};

	struct TriggerPair
	{
		Collider2D* first = nullptr;
		Collider2D* second = nullptr;

		bool operator==(const TriggerPair& other) const
		{
			return first == other.first && second == other.second;
		}
	};

	struct TriggerPairHasher
	{
		size_t operator()(const TriggerPair& pair) const
		{
			const size_t h1 = std::hash<Collider2D*>{}(pair.first);
			const size_t h2 = std::hash<Collider2D*>{}(pair.second);
			return h1 ^ (h2 << 1);
		}
	};

	std::unordered_set<CollisionPair, CollisionPairHasher> g_activeCollisionPairs;
	std::unordered_set<TriggerPair, TriggerPairHasher> g_activeTriggerPairs;

	UINT ClampClientSize(LONG value)
	{
		return value > 0 ? static_cast<UINT>(value) : 1u;
	}

	float ClampAspectRatio(int width, int height)
	{
		if (height <= 0)
		{
			return 1.0f;
		}

		return static_cast<float>(width > 0 ? width : 1) / static_cast<float>(height);
	}

	float GetGameRenderAspectRatio()
	{
		return static_cast<float>(LOGICAL_RENDER_WIDTH) / static_cast<float>(LOGICAL_RENDER_HEIGHT);
	}

	bool LoadEditorKoreanFont(ImGuiIO& io)
	{
		char windowsDirectory[MAX_PATH] = {};
		if (GetWindowsDirectoryA(windowsDirectory, MAX_PATH) == 0)
		{
			return false;
		}

		const std::string fontDirectory = std::string(windowsDirectory) + "\\Fonts\\";
		const std::string fontCandidates[] =
		{
			fontDirectory + "malgun.ttf",
			fontDirectory + "gulim.ttc",
		};

		for (const std::string& fontPath : fontCandidates)
		{
			if (GetFileAttributesA(fontPath.c_str()) == INVALID_FILE_ATTRIBUTES)
			{
				continue;
			}

			if (io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 16.0f, nullptr, io.Fonts->GetGlyphRangesKorean()) != nullptr)
			{
				return true;
			}
		}

		return false;
	}

	bool ShouldRunEditorSimulation(MainFrame* mainFrame)
	{
		if (mainFrame == nullptr)
		{
			return false;
		}

		if (mainFrame->GetRenderType() != RenderType::Edit)
		{
			return true;
		}

		if (mainFrame->IsEditorPlaying())
		{
			return true;
		}

		return false;
	}

	Collider2D* GetColliderFromFixture(b2Fixture* fixture)
	{
		if (fixture == nullptr)
		{
			return nullptr;
		}

		return reinterpret_cast<Collider2D*>(fixture->GetUserData().pointer);
	}

	GameObject* GetCollisionGameObject(Collider2D* collider)
	{
		if (collider == nullptr)
		{
			return nullptr;
		}

		return collider->GetGameObject();
	}

	bool IsPendingDestroyCollisionObject(Collider2D* collider)
	{
		GameObject* gameObject = GetCollisionGameObject(collider);
		return gameObject == nullptr || gameObject->GetDestroy();
	}

	bool IsActiveCollisionObject(Collider2D* collider)
	{
		GameObject* gameObject = GetCollisionGameObject(collider);
		return gameObject != nullptr && !gameObject->GetDestroy() && gameObject->GetActive();
	}

	bool IsTriggerContact(b2Fixture* fixtureA, b2Fixture* fixtureB)
	{
		return (fixtureA != nullptr && fixtureA->IsSensor()) ||
			(fixtureB != nullptr && fixtureB->IsSensor());
	}

	CollisionPair MakeCollisionPair(Collider2D* colliderA, Collider2D* colliderB)
	{
		if (colliderA < colliderB)
		{
			return { colliderA, colliderB };
		}
		return { colliderB, colliderA };
	}

	TriggerPair MakeTriggerPair(Collider2D* colliderA, Collider2D* colliderB)
	{
		if (colliderA < colliderB)
		{
			return { colliderA, colliderB };
		}
		return { colliderB, colliderA };
	}

	bool HasTrackedCollisionPair(Collider2D* colliderA, Collider2D* colliderB)
	{
		if (colliderA == nullptr || colliderB == nullptr || colliderA == colliderB)
		{
			return false;
		}

		return g_activeCollisionPairs.find(MakeCollisionPair(colliderA, colliderB)) != g_activeCollisionPairs.end();
	}

	bool HasTrackedTriggerPair(Collider2D* colliderA, Collider2D* colliderB)
	{
		if (colliderA == nullptr || colliderB == nullptr || colliderA == colliderB)
		{
			return false;
		}

		return g_activeTriggerPairs.find(MakeTriggerPair(colliderA, colliderB)) != g_activeTriggerPairs.end();
	}

	void DispatchCollisionEnter(Collider2D* receiver, Collider2D* other)
	{
		// 충돌 Enter/Stay는 delete-pending 또는 inactive 오브젝트가 포함되면 전달하지 않는다.
		if (!IsActiveCollisionObject(receiver) || !IsActiveCollisionObject(other))
		{
			return;
		}

		GetCollisionGameObject(receiver)->OnCollisionEnter(other);
	}

	void DispatchCollisionStay(Collider2D* receiver, Collider2D* other)
	{
		// 충돌 Enter/Stay는 delete-pending 또는 inactive 오브젝트가 포함되면 전달하지 않는다.
		if (!IsActiveCollisionObject(receiver) || !IsActiveCollisionObject(other))
		{
			return;
		}

		GetCollisionGameObject(receiver)->OnCollisionStay(other);
	}

	void DispatchCollisionExit(Collider2D* receiver, Collider2D* other)
	{
		GameObject* receiverObject = GetCollisionGameObject(receiver);
		if (receiverObject == nullptr || receiverObject->GetDestroy())
		{
			return;
		}

		Collider2D* safeOther = other;
		GameObject* otherObject = GetCollisionGameObject(other);
		if (otherObject == nullptr || otherObject->GetDestroy())
		{
			// 상대가 이미 삭제 예약이면 Exit는 nullptr 정책으로 전달한다.
			safeOther = nullptr;
		}

		receiverObject->OnCollisionExit(safeOther);
	}

	void DispatchTriggerEnter(Collider2D* receiver, Collider2D* other)
	{
		// Trigger Enter/Stay도 delete-pending 또는 inactive 오브젝트가 포함되면 전달하지 않는다.
		if (!IsActiveCollisionObject(receiver) || !IsActiveCollisionObject(other))
		{
			return;
		}

		GetCollisionGameObject(receiver)->OnTriggerEnter(other);
	}

	void DispatchTriggerStay(Collider2D* receiver, Collider2D* other)
	{
		// Trigger Enter/Stay도 delete-pending 또는 inactive 오브젝트가 포함되면 전달하지 않는다.
		if (!IsActiveCollisionObject(receiver) || !IsActiveCollisionObject(other))
		{
			return;
		}

		GetCollisionGameObject(receiver)->OnTriggerStay(other);
	}

	void DispatchTriggerExit(Collider2D* receiver, Collider2D* other)
	{
		GameObject* receiverObject = GetCollisionGameObject(receiver);
		if (receiverObject == nullptr || receiverObject->GetDestroy())
		{
			return;
		}

		Collider2D* safeOther = other;
		GameObject* otherObject = GetCollisionGameObject(other);
	if (otherObject == nullptr || otherObject->GetDestroy())
	{
		safeOther = nullptr;
	}

	// Trigger pair tracking 기준으로 삭제/강제 종료 시 Exit(nullptr)를 허용한다.
	receiverObject->OnTriggerExit(safeOther);
}

	void TrackCollisionPair(Collider2D* colliderA, Collider2D* colliderB)
	{
		if (colliderA == nullptr || colliderB == nullptr || colliderA == colliderB)
		{
			return;
		}

		g_activeCollisionPairs.insert(MakeCollisionPair(colliderA, colliderB));
	}

	void UntrackCollisionPair(Collider2D* colliderA, Collider2D* colliderB)
	{
		if (colliderA == nullptr || colliderB == nullptr || colliderA == colliderB)
		{
			return;
		}

		g_activeCollisionPairs.erase(MakeCollisionPair(colliderA, colliderB));
	}

	void TrackTriggerPair(Collider2D* colliderA, Collider2D* colliderB)
	{
		if (colliderA == nullptr || colliderB == nullptr || colliderA == colliderB)
		{
			return;
		}

		g_activeTriggerPairs.insert(MakeTriggerPair(colliderA, colliderB));
	}

	void UntrackTriggerPair(Collider2D* colliderA, Collider2D* colliderB)
	{
		if (colliderA == nullptr || colliderB == nullptr || colliderA == colliderB)
		{
			return;
		}

		g_activeTriggerPairs.erase(MakeTriggerPair(colliderA, colliderB));
	}

	void CleanupActiveCollisionPairs()
	{
		for (std::unordered_set<CollisionPair, CollisionPairHasher>::iterator itr = g_activeCollisionPairs.begin();
			itr != g_activeCollisionPairs.end();)
		{
			const CollisionPair pair = *itr;
			Collider2D* colliderA = pair.first;
			Collider2D* colliderB = pair.second;

			if (colliderA == nullptr || colliderB == nullptr)
			{
				itr = g_activeCollisionPairs.erase(itr);
				continue;
			}

			const bool colliderADestroyed = IsPendingDestroyCollisionObject(colliderA);
			const bool colliderBDestroyed = IsPendingDestroyCollisionObject(colliderB);
			if (colliderADestroyed || colliderBDestroyed)
			{
				if (!colliderADestroyed)
				{
					DispatchCollisionExit(colliderA, nullptr);
				}
				if (!colliderBDestroyed)
				{
					DispatchCollisionExit(colliderB, nullptr);
				}

				itr = g_activeCollisionPairs.erase(itr);
				continue;
			}

			++itr;
		}
	}

	void DispatchActiveTriggerStayEvents()
	{
		for (std::unordered_set<TriggerPair, TriggerPairHasher>::iterator itr = g_activeTriggerPairs.begin();
			itr != g_activeTriggerPairs.end();)
		{
			const TriggerPair pair = *itr;
			Collider2D* colliderA = pair.first;
			Collider2D* colliderB = pair.second;

			if (colliderA == nullptr || colliderB == nullptr)
			{
				itr = g_activeTriggerPairs.erase(itr);
				continue;
			}

			const bool colliderADestroyed = IsPendingDestroyCollisionObject(colliderA);
			const bool colliderBDestroyed = IsPendingDestroyCollisionObject(colliderB);
			if (colliderADestroyed || colliderBDestroyed)
			{
				if (!colliderADestroyed)
				{
					DispatchTriggerExit(colliderA, nullptr);
				}
				if (!colliderBDestroyed)
				{
					DispatchTriggerExit(colliderB, nullptr);
				}

				itr = g_activeTriggerPairs.erase(itr);
				continue;
			}

			DispatchTriggerStay(colliderA, colliderB);
			DispatchTriggerStay(colliderB, colliderA);
			++itr;
		}
	}

	void BroadcastDeviceLostToComponents()
	{
		ObjectManager* objectManager = ObjectManager::GetInstance();
		if (objectManager == nullptr || objectManager->GetObjList() == nullptr)
		{
			return;
		}

		for (list<GameObject*>::iterator objItr = objectManager->GetObjList()->begin();
			objItr != objectManager->GetObjList()->end();
			++objItr)
		{
			GameObject* gameObject = *objItr;
			if (gameObject == nullptr || gameObject->GetDestroy())
			{
				continue;
			}

			vector<Component*>* components = gameObject->GetComponentVec();
			if (components == nullptr)
			{
				continue;
			}

			for (vector<Component*>::iterator componentItr = components->begin();
				componentItr != components->end();
				++componentItr)
			{
				if (*componentItr != nullptr)
				{
					(*componentItr)->OnDeviceLost();
				}
			}
		}
	}

	void BroadcastDeviceResetToComponents()
	{
		ObjectManager* objectManager = ObjectManager::GetInstance();
		if (objectManager == nullptr || objectManager->GetObjList() == nullptr)
		{
			return;
		}

		for (list<GameObject*>::iterator objItr = objectManager->GetObjList()->begin();
			objItr != objectManager->GetObjList()->end();
			++objItr)
		{
			GameObject* gameObject = *objItr;
			if (gameObject == nullptr || gameObject->GetDestroy())
			{
				continue;
			}

			vector<Component*>* components = gameObject->GetComponentVec();
			if (components == nullptr)
			{
				continue;
			}

			for (vector<Component*>::iterator componentItr = components->begin();
				componentItr != components->end();
				++componentItr)
			{
				if (*componentItr != nullptr)
				{
					(*componentItr)->OnDeviceReset();
				}
			}
		}
	}
}

void MainFrame::RemoveCollisionPairsForCollider(Collider2D* collider)
{
	if (collider == nullptr)
	{
		return;
	}

	for (std::unordered_set<CollisionPair, CollisionPairHasher>::iterator itr = g_activeCollisionPairs.begin();
		itr != g_activeCollisionPairs.end();)
	{
		if (itr->first == collider || itr->second == collider)
		{
			itr = g_activeCollisionPairs.erase(itr);
			continue;
		}

		++itr;
	}
}

void MainFrame::RemoveTriggerPairsForCollider(Collider2D* collider)
{
	if (collider == nullptr)
	{
		return;
	}

	for (std::unordered_set<TriggerPair, TriggerPairHasher>::iterator itr = g_activeTriggerPairs.begin();
		itr != g_activeTriggerPairs.end();)
	{
		if (itr->first == collider || itr->second == collider)
		{
			itr = g_activeTriggerPairs.erase(itr);
			continue;
		}

		++itr;
	}
}

void MainFrame::ResetCollisionPairsForCollider(Collider2D* collider)
{
	if (collider == nullptr)
	{
		return;
	}

	for (std::unordered_set<CollisionPair, CollisionPairHasher>::iterator itr = g_activeCollisionPairs.begin();
		itr != g_activeCollisionPairs.end();)
	{
		if (itr->first != collider && itr->second != collider)
		{
			++itr;
			continue;
		}

		Collider2D* other = itr->first == collider ? itr->second : itr->first;
		DispatchCollisionExit(collider, other);
		if (other != nullptr)
		{
			DispatchCollisionExit(other, collider);
		}

		itr = g_activeCollisionPairs.erase(itr);
	}
}

void MainFrame::ResetTriggerPairsForCollider(Collider2D* collider)
{
	if (collider == nullptr)
	{
		return;
	}

	for (std::unordered_set<TriggerPair, TriggerPairHasher>::iterator itr = g_activeTriggerPairs.begin();
		itr != g_activeTriggerPairs.end();)
	{
		if (itr->first != collider && itr->second != collider)
		{
			++itr;
			continue;
		}

		Collider2D* other = itr->first == collider ? itr->second : itr->first;
		DispatchTriggerExit(collider, other);
		if (other != nullptr)
		{
			DispatchTriggerExit(other, collider);
		}

		itr = g_activeTriggerPairs.erase(itr);
	}
}

void MainFrame::Initialize(int targetFPS, Scene* scene, RenderType type)
{
    m_released = false;
    g_activeCollisionPairs.clear();
    g_activeTriggerPairs.clear();
    m_editorPlaybackState = EditorPlaybackState::Paused;
    m_editorStepRequested = false;
    m_unscaledDeltaTime = 0.0;
    m_frameDeltaTime = 0.0;
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

    ApplyCameraProjection();
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
    if (m_type == RenderType::Edit)
    {
        // 기본 ImGui 폰트에는 한글 글리프가 없어서 에디터 문자열이 깨질 수 있다.
        LoadEditorKoreanFont(io);
    }
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // 키보드 조작 활성화
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // 게임패드 조작 활성화
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    if (m_type == RenderType::Edit)
    {
        // 에디터 모드에서는 ImGui 툴 창을 메인 창 밖으로 분리할 수 있게 멀티 뷰포트를 켠다.
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    }
    io.ConfigWindowsResizeFromEdges = false;

    ImGui::StyleColorsDark();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
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

            const double simulationDeltaTime = min(m_timer.getTotalDeltaTime(), 0.02);

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
            bool shouldRunSimulation = ShouldRunEditorSimulation(this);
            if (m_type == RenderType::Edit && !shouldRunSimulation && m_editorStepRequested)
            {
                shouldRunSimulation = true;
            }

            m_unscaledDeltaTime = simulationDeltaTime;
            const double scaledSimulationDeltaTime = shouldRunSimulation
                ? simulationDeltaTime * static_cast<double>(GetTimeScale())
                : 0.0;
            const bool shouldAdvanceSimulation = shouldRunSimulation && scaledSimulationDeltaTime > 0.0;
            m_frameDeltaTime = shouldAdvanceSimulation ? scaledSimulationDeltaTime : 0.0;

            if (shouldAdvanceSimulation)
            {
                m_pWorld->Step(static_cast<float>(scaledSimulationDeltaTime), m_velocityIterations, m_positionIterations);
                CleanupActiveCollisionPairs();
                DispatchActiveTriggerStayEvents();
                ObjectManager::GetInstance()->Update();

                if (m_type == RenderType::Edit && m_editorStepRequested)
                {
                    m_editorStepRequested = false;
                    m_editorPlaybackState = EditorPlaybackState::Paused;
                }
            }
            else if (m_type == RenderType::Edit && ObjectManager::GetInstance() != nullptr)
            {
                // Paused 상태에서도 생성/삭제 pending과 마우스 ray/hover 정보는 계속 갱신한다.
                ObjectManager::GetInstance()->FlushPendingObjects();
                ObjectManager::GetInstance()->UpdateMouseInteraction();
            }

            //RENDER
            if (m_type == RenderType::Edit)
                RenderManager::GetInstance()->EditUpdate();
            else if (m_type == RenderType::Game)
                RenderManager::GetInstance()->GameUpdate();

            m_timer.resetTotalDeltaTime(); // 업데이트, 랜더 후 토탈 델타 타임 리셋
        }

    return true;
}

void MainFrame::SetEditorPlaying(bool playing)
{
    m_editorPlaybackState = playing ? EditorPlaybackState::Playing : EditorPlaybackState::Paused;
    m_editorStepRequested = false;
    m_unscaledDeltaTime = 0.0;
    m_frameDeltaTime = 0.0;
    m_timer.Resync();
}

void MainFrame::RequestEditorStep()
{
    m_editorPlaybackState = EditorPlaybackState::Paused;
    m_editorStepRequested = true;
    m_unscaledDeltaTime = 0.0;
    m_frameDeltaTime = 0.0;
    m_timer.Resync();
}

bool MainFrame::IsEditorPlaying() const
{
    return m_editorPlaybackState == EditorPlaybackState::Playing;
}

bool MainFrame::IsEditorPaused() const
{
    return m_editorPlaybackState == EditorPlaybackState::Paused && !m_editorStepRequested;
}

bool MainFrame::IsEditorStepRequested() const
{
    return m_editorStepRequested;
}

ID3DXFont* MainFrame::GetFont()
{
    return m_pFont;
}

LPDIRECT3DDEVICE9 MainFrame::GetDevice()
{
    return m_pd3dDevice;
}

const D3DXMATRIX& MainFrame::GetProjectionMatrix() const
{
    return m_matProj;
}

int MainFrame::GetViewportWidth() const
{
    return m_width;
}

int MainFrame::GetViewportHeight() const
{
    return m_height;
}

b2World* MainFrame::GetBox2dWorld()
{
    return m_pWorld;
}

void MainFrame::RequestResize(UINT width, UINT height)
{
    if (width == 0 || height == 0)
    {
        return;
    }

    m_pendingResizeWidth = width;
    m_pendingResizeHeight = height;
    m_pendingResize = true;
}

void MainFrame::ApplyCameraProjection()
{
    if (m_pd3dDevice == nullptr || Camera::GetInstance() == nullptr)
    {
        return;
    }

    Camera* camera = Camera::GetInstance();
    const float nearClip = camera->GetNearClip();
    const float farClip = camera->GetFarClip();

    if (camera->GetProjectionMode() == CameraProjectionMode::Perspective)
    {
        const float aspectRatio = (m_type == RenderType::Game)
            ? GetGameRenderAspectRatio()
            : ClampAspectRatio(m_width, m_height);
        D3DXMatrixPerspectiveFovLH(&m_matProj, camera->GetFov(), aspectRatio, nearClip, farClip);
    }
    else
    {
        const float orthoHeight = camera->GetOrthographicSize();
        const float aspectRatio = (m_type == RenderType::Game)
            ? GetGameRenderAspectRatio()
            : ClampAspectRatio(m_width, m_height);
        const float orthoWidth = orthoHeight * aspectRatio;
        D3DXMatrixOrthoLH(&m_matProj, orthoWidth, orthoHeight, nearClip, farClip);
    }

    m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &m_matProj);
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
    if (Camera::GetInstance() != nullptr)
    {
        Camera::GetInstance()->ApplyView();
    }
    ApplyCameraProjection();
    ApplyDeviceState();
    RestoreDeviceResources();
    m_timer.Resync();
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

    BroadcastDeviceLostToComponents();

    if (ImGui::GetCurrentContext() != nullptr)
    {
        ImGui_ImplDX9_InvalidateDeviceObjects();
    }

    m_deviceResourcesInvalidated = true;
}

void MainFrame::RestoreDeviceResources()
{
    if (m_pFont != nullptr)
    {
        m_pFont->OnResetDevice();
    }

    BroadcastDeviceResetToComponents();

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
    g_activeCollisionPairs.clear();
    g_activeTriggerPairs.clear();
}

void CollisionListener::BeginContact(b2Contact* contact)
{
    b2Fixture* fixtureA = contact->GetFixtureA();
    b2Fixture* fixtureB = contact->GetFixtureB();

    Collider2D* dataA = GetColliderFromFixture(fixtureA);
    Collider2D* dataB = GetColliderFromFixture(fixtureB);
	if (dataA == nullptr || dataB == nullptr)
	{
		return;
	}

	if (IsTriggerContact(fixtureA, fixtureB))
	{
		TrackTriggerPair(dataA, dataB);
		DispatchTriggerEnter(dataA, dataB);
		DispatchTriggerEnter(dataB, dataA);
		return;
	}

	TrackCollisionPair(dataA, dataB);
	DispatchCollisionEnter(dataA, dataB);
	DispatchCollisionEnter(dataB, dataA);
}

void CollisionListener::EndContact(b2Contact* contact)
{
    b2Fixture* fixtureA = contact->GetFixtureA();
    b2Fixture* fixtureB = contact->GetFixtureB();

	Collider2D* dataA = GetColliderFromFixture(fixtureA);
	Collider2D* dataB = GetColliderFromFixture(fixtureB);
	if (IsTriggerContact(fixtureA, fixtureB))
	{
		const bool wasTracked = HasTrackedTriggerPair(dataA, dataB);
		UntrackTriggerPair(dataA, dataB);
		if (dataA != nullptr && dataB != nullptr && !wasTracked)
		{
			return;
		}
		if (dataA != nullptr)
			DispatchTriggerExit(dataA, dataB);
		if (dataB != nullptr)
			DispatchTriggerExit(dataB, dataA);
		return;
	}
	const bool wasTracked = HasTrackedCollisionPair(dataA, dataB);
	UntrackCollisionPair(dataA, dataB);
	if (dataA != nullptr && dataB != nullptr && !wasTracked)
	{
		return;
	}
	if (dataA != nullptr)
	{
		DispatchCollisionExit(dataA, dataB);
	}
	if (dataB != nullptr)
	{
		DispatchCollisionExit(dataB, dataA);
	}
}

void CollisionListener::PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
{
	b2Fixture* fixtureA = contact->GetFixtureA();
	b2Fixture* fixtureB = contact->GetFixtureB();

	UNREFERENCED_PARAMETER(oldManifold);

	Collider2D* dataA = GetColliderFromFixture(fixtureA);
	Collider2D* dataB = GetColliderFromFixture(fixtureB);
	if (dataA == nullptr || dataB == nullptr)
	{
		return;
	}

	if (IsTriggerContact(fixtureA, fixtureB))
	{
		return;
	}

	DispatchCollisionStay(dataA, dataB);
	DispatchCollisionStay(dataB, dataA);
}

void CollisionListener::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
{
}
