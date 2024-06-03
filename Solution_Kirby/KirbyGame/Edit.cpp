#include "pch.h"
#include "Edit.h"


vector<string> Edit::ReadMapData(string mapName)
{
    m_mapName = mapName;
    char buffer[MAX_PATH];
    vector<string> mapData;
    GetCurrentDirectoryA(MAX_PATH, buffer);
    string currentDirectory = buffer;

    // �� ���� ��� ����
    string mapFilePath = currentDirectory + "\\Map\\" + mapName + ".txt";

    // ���� ����
    ifstream inFile(mapFilePath);
    if (!inFile) {
        cout << "������ �������� ����, �� �� ������ ������ ����ϴ�" << endl;
        return mapData;
    }

    // ���Ͽ��� ���ڵ��� 2���� �迭�� �о����
    string line;
    while (getline(inFile, line)) 
    {
        mapData.push_back(line);
    }
    // ���� �ݱ�
    inFile.close();
    return mapData;
}

void Edit::WriteMapData()
{
    char buffer[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, buffer);
    string currentDirectory = buffer;

    // �� ���� ��� ����
    string mapFilePath = currentDirectory + "\\Map\\" + m_mapName + ".txt";
    ofstream file(mapFilePath);

    // ������ �������� �ʴ� ��� ���ο� ���� ����
    if (!file.is_open()) {
        cout << "������ ������ �� �����ϴ�." << endl;
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

    // ������ ����
    for (const auto& line : vStr) {
        file << line << endl;
    }
    

    // ���� �ݱ�
    file.close();

    cout << "���Ͽ� �� �����͸� ���������� �����߽��ϴ�." << endl;
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
        obj->SetPosition({ (double)UNITSIZE * i + UNITSIZE / 2 ,(double)UNITSIZE * j + UNITSIZE / 2 });
        obj->InitializeSet();
    }
    break;
    case MapType::Block:
    {
        obj = new GameObject();
        obj->AddComponent(new BitmapRender(m_land));
        obj->Size() = { UNITSIZE, UNITSIZE};
        obj->SetPosition({ (double)UNITSIZE * i + UNITSIZE / 2 ,(double)UNITSIZE * j + UNITSIZE / 2 });
        obj->InitializeSet();
    }
    break;
    case MapType::DefaultMon:
    {
        obj = new GameObject();
        obj->AddComponent(new BitmapRender(m_defaultObj));
        obj->Size() = { UNITSIZE, UNITSIZE};
        obj->SetPosition({ (double)UNITSIZE * i + UNITSIZE / 2 ,(double)UNITSIZE * j + UNITSIZE / 2 });
        obj->InitializeSet();
    }
    break;
    case MapType::SwordMon:
    {
        obj = new GameObject();
        obj->AddComponent(new BitmapRender(m_swordObj));
        obj->Size() = { UNITSIZE, UNITSIZE};
        obj->SetPosition({ (double)UNITSIZE * i + UNITSIZE / 2 ,(double)UNITSIZE * j + UNITSIZE / 2 });
        obj->InitializeSet();
    }
    break;
    case MapType::StoneMon:
    {
        obj = new GameObject();
        obj->AddComponent(new BitmapRender(m_stoneObj));
        obj->Size() = { UNITSIZE, UNITSIZE};
        obj->SetPosition({ (double)UNITSIZE * i + UNITSIZE / 2 ,(double)UNITSIZE * j + UNITSIZE / 2 });
        obj->InitializeSet();
    }
    break;
    case MapType::Door:
    {
        obj = new GameObject();
        obj->AddComponent(new BitmapRender(m_door));
        obj->Size() = { UNITSIZE, UNITSIZE };
        obj->SetPosition({ (double)UNITSIZE * i + UNITSIZE / 2 ,(double)UNITSIZE * j + UNITSIZE / 2 });
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
	newObj->SetPosition({ (double)UNITSIZE * indexX + UNITSIZE / 2 ,(double)UNITSIZE * indexY + UNITSIZE / 2 });
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
    m_land = AnimationManager::LoadHBitmap("Bitmaps\\obj\\land");
    m_bg = AnimationManager::LoadHBitmap("Bitmaps\\obj\\BG");
    m_defaultObj = AnimationManager::LoadHBitmap("Bitmaps\\obj\\defaultObj");
    m_swordObj = AnimationManager::LoadHBitmap("Bitmaps\\obj\\swordObj");
    m_stoneObj = AnimationManager::LoadHBitmap("Bitmaps\\obj\\stoneObj");
    m_player = AnimationManager::LoadHBitmap("Bitmaps\\obj\\player");
    m_door = AnimationManager::LoadHBitmap("Bitmaps\\obj\\door");

    for (int i = 0; i < (int)MapType::max; i++)
    {
        GameObject* obj = new GameObject();
        m_selectBtn[i] = new ColorButton();
        obj->AddComponent(m_selectBtn[i]);
        m_selectBtn[i]->SetUIPos({ 200.0 + i * 100, 10 });
        m_selectBtn[i]->SetUISize({ 90,50 });
        obj->SetOrderInLayer(10);
        obj->InitializeSet();
    }
    m_selectBtn[(int)MapType::None]->SetText(TEXT("���"));
    m_selectBtn[(int)MapType::Block]->SetText(TEXT("��"));
    m_selectBtn[(int)MapType::Player]->SetText(TEXT("Ŀ��"));
    m_selectBtn[(int)MapType::DefaultMon]->SetText(TEXT("�⺻ ��"));
    m_selectBtn[(int)MapType::SwordMon]->SetText(TEXT("�ҵ� ��"));
    m_selectBtn[(int)MapType::StoneMon]->SetText(TEXT("���� ��"));
    m_selectBtn[(int)MapType::Door]->SetText(TEXT("��"));

    GameObject* obj = new GameObject();
    m_InitMapBtn = new ColorButton();
    obj->AddComponent(m_InitMapBtn);
    m_InitMapBtn->SetUIPos({900, 10});
    m_InitMapBtn->SetUISize({ 90,50 });
    obj->SetOrderInLayer(10);
    obj->InitializeSet();
    m_InitMapBtn->SetText(TEXT("�ʱ�ȭ"));
    m_InitMapBtn->SetEvent(bind(&Edit::InitMap, this));

    GameObject* objUndo = new GameObject();
    ColorButton* btnUndo = new ColorButton();
    objUndo->AddComponent(btnUndo);
    btnUndo->SetUIPos({ 1000, 10 });
    btnUndo->SetUISize({ 90,50 });
    objUndo->SetOrderInLayer(10);
    objUndo->InitializeSet();
    btnUndo->SetText(TEXT("UnDo"));
    btnUndo->SetEvent(bind(&Edit::Undo, this));

    GameObject* objRedo = new GameObject();
    ColorButton* btnRedo = new ColorButton();
    objRedo->AddComponent(btnRedo);
    btnRedo->SetUIPos({ 1100, 10 });
    btnRedo->SetUISize({ 90,50 });
    objRedo->SetOrderInLayer(10);
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
    AnimationManager::ReleaseHBitmap(m_land);
    AnimationManager::ReleaseHBitmap(m_bg);
    AnimationManager::ReleaseHBitmap(m_defaultObj);
    AnimationManager::ReleaseHBitmap(m_swordObj);
    AnimationManager::ReleaseHBitmap(m_stoneObj);
    AnimationManager::ReleaseHBitmap(m_player);
    AnimationManager::ReleaseHBitmap(m_door);

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
	for (int i = 0; i < m_count; i++)
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
    }

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
        if (Mouse::GetInstance()->GetPos().Y < UNITSIZE)
            return;

        int x = Mouse::GetInstance()->GetPos().X + Camera::GetInstance()->GetPos().x - UNITSIZE / 2;
        int y = Mouse::GetInstance()->GetPos().Y + Camera::GetInstance()->GetPos().y - UNITSIZE / 2;

        int indexX = x / UNITSIZE;
        int indexY = y / UNITSIZE;
        if (m_mapTypeData[indexY][indexX] == MapType::Player)
        {
            cout << "�÷��̾�� ���� �� �����ϴ�." << endl;
            return;
        }
        if (m_select == MapType::Player && m_mapTypeData[indexY][indexX] != MapType::None)
        {
            cout << "�÷��̾�� �� ������ ��ġ�� �� �ֽ��ϴ�." << endl;
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
    WindowFrame::GetInstance()->GetBuffer()->SetBG(m_bg);

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