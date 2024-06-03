#include "pch.h"
#include "Edit.h"


vector<string> Edit::ReadMapData(string mapName)
{
    m_mapName = mapName;
    char buffer[MAX_PATH];
    vector<string> mapData;
    GetCurrentDirectoryA(MAX_PATH, buffer);
    string currentDirectory = buffer;

    // 맵 파일 경로 설정
    string mapFilePath = currentDirectory + "\\Map\\" + mapName + ".txt";

    // 파일 열기
    ifstream inFile(mapFilePath);
    if (!inFile) {
        cout << "파일이 존재하지 않음, 새 맵 데이터 파일을 만듭니다" << endl;
        return mapData;
    }

    // 파일에서 숫자들을 2차원 배열로 읽어오기
    string line;
    while (getline(inFile, line)) 
    {
        mapData.push_back(line);
    }
    // 파일 닫기
    inFile.close();
    return mapData;
}

void Edit::WriteMapData()
{
    char buffer[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, buffer);
    string currentDirectory = buffer;

    // 맵 파일 경로 설정
    string mapFilePath = currentDirectory + "\\Map\\" + m_mapName + ".txt";
    ofstream file(mapFilePath);

    // 파일이 존재하지 않는 경우 새로운 파일 생성
    if (!file.is_open()) {
        cout << "파일을 생성할 수 없습니다." << endl;
        return;
    }

    vector<string> vStr;
    for (int i = 0; i < m_mapTypeData.size(); i++)
    {
        string row = "";
        for (int j = 0; j < m_mapTypeData[i].size(); j++)
        {
            row.append(to_string((int)m_mapTypeData[i][j]));
        }
        vStr.push_back(row);
    }

    // 데이터 쓰기
    for (const auto& line : vStr) {
        file << line << endl;
    }
    

    // 파일 닫기
    file.close();

    cout << "파일에 맵 데이터를 성공적으로 저장했습니다." << endl;
}

GameObject* Edit::DrawMap(MapType t, int i, int j)
{
    GameObject* obj = nullptr;
    switch (t)
    {
    case MapType::None:
    {
        return nullptr;
    }
    break;
    case MapType::Player:
    {
        obj = new GameObject();
        obj->AddComponent(new BitmapRender(m_player));
        obj->Size() = { UNITSIZE, UNITSIZE};
        obj->SetPosition({ (float)(UNITSIZE * i + UNITSIZE / 2) ,(float)(UNITSIZE * j + UNITSIZE / 2), 0.0f });
        obj->InitializeSet();
    }
    break;
    case MapType::Block:
    {
        obj = new GameObject();
        obj->AddComponent(new BitmapRender(m_land));
        obj->Size() = { UNITSIZE, UNITSIZE};
        obj->SetPosition({ (float)(UNITSIZE * i + UNITSIZE / 2) ,(float)(UNITSIZE * j + UNITSIZE / 2), 0.0f });
        obj->InitializeSet();
    }
    break;
    case MapType::DefaultMon:
    {
        obj = new GameObject();
        obj->AddComponent(new BitmapRender(m_defaultObj));
        obj->Size() = { UNITSIZE, UNITSIZE};
        obj->SetPosition({ (float)(UNITSIZE * i + UNITSIZE / 2) ,(float)(UNITSIZE * j + UNITSIZE / 2), 0.0f });
        obj->InitializeSet();
    }
    break;
    case MapType::SwordMon:
    {
        obj = new GameObject();
        obj->AddComponent(new BitmapRender(m_swordObj));
        obj->Size() = { UNITSIZE, UNITSIZE};
        obj->SetPosition({ (float)(UNITSIZE * i + UNITSIZE / 2) ,(float)(UNITSIZE * j + UNITSIZE / 2), 0.0f });
        obj->InitializeSet();
    }
    break;
    case MapType::StoneMon:
    {
        obj = new GameObject();
        obj->AddComponent(new BitmapRender(m_stoneObj));
        obj->Size() = { UNITSIZE, UNITSIZE};
        obj->SetPosition({ (float)(UNITSIZE * i + UNITSIZE / 2) ,(float)(UNITSIZE * j + UNITSIZE / 2), 0.0f });
        obj->InitializeSet();
    }
    break;
    case MapType::Door:
    {
        obj = new GameObject();
        obj->AddComponent(new BitmapRender(m_door));
        obj->Size() = { UNITSIZE, UNITSIZE };
        obj->SetPosition({ (float)(UNITSIZE * i + UNITSIZE / 2) ,(float)(UNITSIZE * j + UNITSIZE / 2), 0.0f });
        obj->InitializeSet();
    }
    break;
    }
    return obj;
}

void Edit::SelectNone()
{
    m_select = MapType::None;
}

void Edit::SelectLand()
{
    m_select = MapType::Block;
}

void Edit::SelectDefaultMon()
{
    m_select = MapType::DefaultMon;
}

void Edit::SelectSwordMon()
{
    m_select = MapType::SwordMon;
}

void Edit::SelectStoneMon()
{
    m_select = MapType::StoneMon;
}

void Edit::SelectPlayer()
{
    m_select = MapType::Player;
}

void Edit::SelectDoor()
{
    m_select = MapType::Door;
}

void Edit::InitMap()
{
    for (int i = 0; i < m_count; ++i)
    {
        for (int j = 0; j < m_count; ++j)
        {
            if (m_mapData[i][j])
            {
                m_mapData[i][j]->SetDestroy(true);
                m_mapData[i][j] = nullptr;
            }
			m_mapTypeData[i][j] = MapType::None;
		}
	}
	while (!m_redoStack.empty())
		m_redoStack.pop();
	while (!m_undoStack.empty())
		m_undoStack.pop();

    m_mapData[PLAYER_DEFAULT_Y][PLAYER_DEFAULT_X] = DrawMap(MapType::Player, PLAYER_DEFAULT_X, PLAYER_DEFAULT_Y);
    m_mapTypeData[PLAYER_DEFAULT_Y][PLAYER_DEFAULT_X] = MapType::Player;
}

void Edit::ReDrawMapObj(int indexX, int indexY, MapType type)
{
    cout << "RedoStackSize : " << m_redoStack.size() << endl;
    cout << "UndoStackSize : " << m_undoStack.size() << endl;

	if (m_mapTypeData[indexY][indexX] != MapType::None)
	{
		m_mapData[indexY][indexX]->SetDestroy(true);
		if (type == MapType::None)
		{
			m_mapData[indexY][indexX] = nullptr;
			m_mapTypeData[indexY][indexX] = MapType::None;
			return;
		}
	}

	GameObject* newObj = new GameObject();
	switch (type)
	{
	case MapType::None:
		break;
	case MapType::Block:
		newObj->AddComponent(new BitmapRender(m_land));
		break;
	case MapType::Player:
		newObj->AddComponent(new BitmapRender(m_player));
		for (int i = 0; i < m_count; ++i)
		{
			for (int j = 0; j < m_count; ++j)
			{
				if (m_mapTypeData[i][j] == MapType::Player)
				{
					m_mapData[i][j]->SetDestroy(true);
					m_mapData[i][j] = nullptr;
					m_mapTypeData[i][j] = MapType::None;
				}
			}
		}
		break;
	case MapType::DefaultMon:
		newObj->AddComponent(new BitmapRender(m_defaultObj));
		break;
	case MapType::SwordMon:
		newObj->AddComponent(new BitmapRender(m_swordObj));
		break;
	case MapType::StoneMon:
		newObj->AddComponent(new BitmapRender(m_stoneObj));
		break;
    case MapType::Door:
        newObj->AddComponent(new BitmapRender(m_door));
        break;
	}

	newObj->Size() = { UNITSIZE, UNITSIZE };
	newObj->SetPosition({ (float)(UNITSIZE * indexX + UNITSIZE / 2) ,(float)(UNITSIZE * indexY + UNITSIZE / 2), 0.0f });
	newObj->InitializeSet();

	m_mapData[indexY][indexX] = newObj;
	m_mapTypeData[indexY][indexX] = type;
}

void Edit::Undo()
{
    if (m_undoStack.empty())
        return;

    UndoRedoData data = m_undoStack.top();
    m_undoStack.pop();

    UndoRedoData newData = data; 
    newData.isErase = !newData.isErase;
    if (data.isErase)
    {
        m_redoStack.push(newData);
        ReDrawMapObj(data.x, data.y, MapType::None);
    }
	else if (data.type == MapType::Player)
	{
		newData.isErase = false;
		for (int i = 0; i < m_mapTypeData.size(); i++)
		{
			bool isFind = false;
			for (int j = 0; j < m_mapTypeData[i].size(); j++)
			{
				if (m_mapTypeData[i][j] == MapType::Player)
				{
					newData.x = j;
					newData.y = i;
					isFind = true;
					break;
				}
			}
			if (isFind)
				break;
		}
		newData.type = MapType::Player;
		m_redoStack.push(newData);
		ReDrawMapObj(data.x, data.y, data.type);
	}
    else
    {
        m_redoStack.push(newData);
        ReDrawMapObj(data.x, data.y, data.type);
    }
}

void Edit::Redo()
{
    if (m_redoStack.empty())
        return;

    UndoRedoData data = m_redoStack.top();
    m_redoStack.pop();

    UndoRedoData newData = data; 
    newData.isErase = !newData.isErase;

    if (data.isErase)
    {
        m_undoStack.push(newData);
        ReDrawMapObj(data.x, data.y, MapType::None);
    }
    else if (data.type == MapType::Player)
    {
        newData.isErase = false;
        for (int i = 0; i < m_mapTypeData.size(); i++)
        {
            bool isFind = false;
            for (int j = 0; j < m_mapTypeData[i].size(); j++)
            {
                if (m_mapTypeData[i][j] == MapType::Player)
                {
                    newData.x = j;
                    newData.y = i;
                    isFind = true;
                    break;
                }
            }
            if (isFind)
                break;
        }
        newData.type = MapType::Player;
        m_undoStack.push(newData);
        ReDrawMapObj(data.x, data.y, data.type);
    }
    else
    {
        m_undoStack.push(newData);
        ReDrawMapObj(data.x, data.y, data.type);
    }
}

void Edit::Initialize()
{
    m_land = AnimationManager::LoadTexture(L"Bitmaps\\obj\\land");
    m_bg = AnimationManager::LoadTexture(L"Bitmaps\\obj\\BG");
    m_defaultObj = AnimationManager::LoadTexture(L"Bitmaps\\obj\\defaultObj");
    m_swordObj = AnimationManager::LoadTexture(L"Bitmaps\\obj\\swordObj");
    m_stoneObj = AnimationManager::LoadTexture(L"Bitmaps\\obj\\stoneObj");
    m_player = AnimationManager::LoadTexture(L"Bitmaps\\obj\\player");
    m_door = AnimationManager::LoadTexture(L"Bitmaps\\obj\\door");

    for (int i = 0; i < (int)MapType::max; i++)
    {
        GameObject* obj = new GameObject();
        m_selectBtn[i] = new ColorButton();
        obj->AddComponent(m_selectBtn[i]);
        m_selectBtn[i]->SetUIPos({ 200.0f + i * 100, 10, -1.0f });
        m_selectBtn[i]->SetUISize({ 90,50 });
        obj->SetOrderInLayer(10);
        obj->InitializeSet();
    }
    m_selectBtn[(int)MapType::None]->SetText(TEXT("빈곳"));
    m_selectBtn[(int)MapType::Block]->SetText(TEXT("땅"));
    m_selectBtn[(int)MapType::Player]->SetText(TEXT("커비"));
    m_selectBtn[(int)MapType::DefaultMon]->SetText(TEXT("기본 몹"));
    m_selectBtn[(int)MapType::SwordMon]->SetText(TEXT("소드 몹"));
    m_selectBtn[(int)MapType::StoneMon]->SetText(TEXT("스톤 몹"));
    m_selectBtn[(int)MapType::Door]->SetText(TEXT("문"));

    GameObject* obj = new GameObject();
    m_InitMapBtn = new ColorButton();
    obj->AddComponent(m_InitMapBtn);
    m_InitMapBtn->SetUIPos({900, 10, -1.0f});
    m_InitMapBtn->SetUISize({ 90,50 });
    obj->SetOrderInLayer(-10);
    obj->InitializeSet();
    m_InitMapBtn->SetText(TEXT("초기화"));
    m_InitMapBtn->SetEvent(bind(&Edit::InitMap, this));

    GameObject* objUndo = new GameObject();
    ColorButton* btnUndo = new ColorButton();
    objUndo->AddComponent(btnUndo);
    btnUndo->SetUIPos({ 1000, 10, -1.0f });
    btnUndo->SetUISize({ 90,50 });
    objUndo->SetOrderInLayer(-10);
    objUndo->InitializeSet();
    btnUndo->SetText(TEXT("UnDo"));
    btnUndo->SetEvent(bind(&Edit::Undo, this));

    GameObject* objRedo = new GameObject();
    ColorButton* btnRedo = new ColorButton();
    objRedo->AddComponent(btnRedo);
    btnRedo->SetUIPos({ 1100, 10, -1.0f });
    btnRedo->SetUISize({ 90,50 });
    objRedo->SetOrderInLayer(-10);
    objRedo->InitializeSet();
    btnRedo->SetText(TEXT("ReDo"));
    btnRedo->SetEvent(bind(&Edit::Redo, this));

    m_selectBtn[(int)MapType::None]->SetEvent(bind(&Edit::SelectNone, this));
    m_selectBtn[(int)MapType::Block]->SetEvent(bind(&Edit::SelectLand, this));
    m_selectBtn[(int)MapType::Player]->SetEvent(bind(&Edit::SelectPlayer, this));
    m_selectBtn[(int)MapType::DefaultMon]->SetEvent(bind(&Edit::SelectDefaultMon, this));
    m_selectBtn[(int)MapType::SwordMon]->SetEvent(bind(&Edit::SelectSwordMon, this));
    m_selectBtn[(int)MapType::StoneMon]->SetEvent(bind(&Edit::SelectStoneMon, this));
    m_selectBtn[(int)MapType::Door]->SetEvent(bind(&Edit::SelectDoor, this));


    m_mapData = vector<vector<GameObject*>>(m_count);
    m_mapTypeData = vector<vector<MapType>>(m_count);
    for (int i = 0; i < m_count; i++)
    {
        m_mapData[i] = vector<GameObject*>(m_count);
        m_mapTypeData[i] = vector<MapType>(m_count);
    }

}

void Edit::Release()
{
    AnimationManager::ReleaseTexture(m_land);
    AnimationManager::ReleaseTexture(m_bg);
    AnimationManager::ReleaseTexture(m_defaultObj);
    AnimationManager::ReleaseTexture(m_swordObj);
    AnimationManager::ReleaseTexture(m_stoneObj);
    AnimationManager::ReleaseTexture(m_player);
    AnimationManager::ReleaseTexture(m_door);

    WriteMapData();
}

void Edit::Start()
{
    RECT rect;
    GetClientRect(WindowFrame::GetInstance()->GetHWND(), &rect);
    Camera::GetInstance()->SetPos(UNITSIZE + UNITSIZE / 2, 0);
}

void Edit::Update()
{
	/*for (int i = 0; i < m_count; i++)
	{
        MoveToEx(WindowFrame::GetInstance()->GetBuffer()->GetHDC(), 
            i * UNITSIZE - UNITSIZE / 2 - Camera::GetInstance()->GetPos().x, -Camera::GetInstance()->GetPos().y, NULL);
        LineTo(WindowFrame::GetInstance()->GetBuffer()->GetHDC(), 
            i * UNITSIZE - UNITSIZE / 2 - Camera::GetInstance()->GetPos().x, UNITSIZE * 100 - Camera::GetInstance()->GetPos().y);
	}
    for (int i = 0; i < m_count; i++)
    {
        MoveToEx(WindowFrame::GetInstance()->GetBuffer()->GetHDC(), 
            - Camera::GetInstance()->GetPos().x, i * UNITSIZE - UNITSIZE / 2 - Camera::GetInstance()->GetPos().y, NULL);
        LineTo(WindowFrame::GetInstance()->GetBuffer()->GetHDC(), 
            UNITSIZE * 100 - Camera::GetInstance()->GetPos().x, i * UNITSIZE - UNITSIZE / 2 - Camera::GetInstance()->GetPos().y);
    }*/

	int cameraSpd = 10;
	if (GetAsyncKeyState(0x57)) //W
	{
		if (Camera::GetInstance()->GetPos().y - cameraSpd >= 0)
			Camera::GetInstance()->SetPos(Camera::GetInstance()->GetPos().x, Camera::GetInstance()->GetPos().y - cameraSpd);
	}
	if (GetAsyncKeyState(0x53)) //S
	{
		if (Camera::GetInstance()->GetPos().y + cameraSpd <= UNITSIZE * m_count - UNITSIZE * 15)
			Camera::GetInstance()->SetPos(Camera::GetInstance()->GetPos().x, Camera::GetInstance()->GetPos().y + cameraSpd);
	}
	if (GetAsyncKeyState(0x41)) //A
	{
		if (Camera::GetInstance()->GetPos().x - cameraSpd >= UNITSIZE + UNITSIZE / 2)
			Camera::GetInstance()->SetPos(Camera::GetInstance()->GetPos().x - cameraSpd, Camera::GetInstance()->GetPos().y);
	}
	if (GetAsyncKeyState(0x44)) //D
	{
		if (Camera::GetInstance()->GetPos().x + cameraSpd <= UNITSIZE * m_count - UNITSIZE * 20)
			Camera::GetInstance()->SetPos(Camera::GetInstance()->GetPos().x + cameraSpd, Camera::GetInstance()->GetPos().y);
	}


	if (Mouse::GetInstance()->IsLeftDown())
    {
        if (Mouse::GetInstance()->GetPos().y < UNITSIZE)
            return;

        int x = Mouse::GetInstance()->GetPos().x + Camera::GetInstance()->GetPos().x - UNITSIZE / 2;
        int y = Mouse::GetInstance()->GetPos().y + Camera::GetInstance()->GetPos().y - UNITSIZE / 2;

        int indexX = x / UNITSIZE;
        int indexY = y / UNITSIZE;
        if (m_mapTypeData[indexY][indexX] == MapType::Player)
        {
            cout << "플레이어는 지울 수 없습니다." << endl;
            return;
        }
        if (m_select == MapType::Player && m_mapTypeData[indexY][indexX] != MapType::None)
        {
            cout << "플레이어는 빈 곳에만 위치할 수 있습니다." << endl;
            return;
        }
        if (m_mapTypeData[indexY][indexX] != m_select)
        {    
            while (!m_redoStack.empty())
            {
                m_redoStack.pop();
            }
 
            UndoRedoData undoData;
            undoData.x = indexX;
            undoData.y = indexY;
            if (m_select == MapType::None )
            {
                undoData.isErase = false;
                undoData.type = m_mapTypeData[indexY][indexX];
                m_undoStack.push(undoData);
                ReDrawMapObj(indexX, indexY, m_select);
            }
            else if (m_select == MapType::Player)
            {
                undoData.isErase = false;
                for (int i = 0; i < m_mapTypeData.size(); i++)
                {
                    bool isFind = false;
                    for (int j = 0; j < m_mapTypeData[i].size(); j++)
                    {
                        if (m_mapTypeData[i][j] == MapType::Player)
                        {
                            undoData.x = j;
                            undoData.y = i;
                            isFind = true;
                            break;
                        }
                    }
                    if (isFind)
                        break;
                }
                undoData.type = MapType::Player;
                m_undoStack.push(undoData);
                ReDrawMapObj(indexX, indexY, m_select);
            }
            else
            {
                ReDrawMapObj(indexX, indexY, m_select);
                undoData.isErase = true;
                undoData.type = m_mapTypeData[indexY][indexX];
                m_undoStack.push(undoData);
            }
        }
	}
}

void Edit::SetMap(string mapName)
{
    vector<string> strMapData = ReadMapData(mapName);
    if (strMapData.empty())
    {
        m_mapTypeData[PLAYER_DEFAULT_Y][PLAYER_DEFAULT_X] = MapType::Player;
        m_mapData[PLAYER_DEFAULT_Y][PLAYER_DEFAULT_X] = DrawMap(MapType::Player, PLAYER_DEFAULT_X, PLAYER_DEFAULT_Y);
        return;
    }

	for (int i = 0; i < m_count; ++i)
	{
		vector<GameObject*> row;
		for (int j = 0; j < m_count; ++j)
		{
			m_mapData[i][j] = DrawMap((MapType)(strMapData[i][j] - '0'), j, i);
			m_mapTypeData[i][j] = (MapType)(strMapData[i][j] - '0');
			if ((MapType)(strMapData[i][j] - '0') != MapType::None)
				cout << "Draw Tile Type : " << MapTypeToString((MapType)(strMapData[i][j] - '0')) << endl;
		}
	}
}