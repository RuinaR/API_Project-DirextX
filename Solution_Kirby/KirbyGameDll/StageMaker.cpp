#include "pch.h"
#include "StageMaker.h"
#include "Player.h"
#include "BoxCollider.h"
#include "ChangeObject.h"
#include "Door.h"
#include <atlconv.h>


StageMaker* StageMaker::m_Pthis = nullptr;

bool EqualFloat(float f1, float f2, float d)
{
	if (f1 + d > f2 && f1 - d < f2)
		return true;

	return false;
}

vector<string> StageMaker::ReadMapData(string mapName)
{
    //char buffer[MAX_PATH];
    //GetCurrentDirectoryA(MAX_PATH, buffer);
    //string currentDirectory = buffer;
    vector<string> mapData;

    wchar_t path[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, path, MAX_PATH);
    USES_CONVERSION;
    std::string executepath = W2A(path);
    executepath = executepath.substr(0, executepath.find_last_of("\\/"));
    // 맵 파일 경로 설정
    string mapFilePath = executepath + "\\Map\\" + mapName + ".txt";

    // 파일 열기
    ifstream inFile(mapFilePath);
    if (!inFile) {
        cout << "맵 파일이 없습니다." << endl;
        return mapData;
    }

    // 파일에서 숫자들을 2차원 배열로 읽어오기
    
    string line;
    while (getline(inFile, line)) 
    {
        mapData.push_back('0' + line);
    }

    // 파일 닫기
    inFile.close();
    return mapData;
}

void StageMaker::MakeMap(MapType t, int i, int j, vector<GameObject*>* rowGroup)
{
	switch (t)
	{
	case MapType::None:
    {
        rowGroup->push_back(nullptr);
    }
		break;
	case MapType::Player:
    {
        m_playerObj->SetPosition({ (float)UNITSIZE * i,(float)-UNITSIZE * j,5.0f });
        rowGroup->push_back(m_playerObj);
	}
	break;
	case MapType::Block: 
	{
        GameObject* block = new GameObject();
        block->SetTag(TAG_LAND);
        block->Size3D() = { UNITSIZE, UNITSIZE, 1.0f };
        block->SetPosition({ (float)UNITSIZE * i,(float)-UNITSIZE * j,10.0f });
        BoxCollider* box = new BoxCollider(b2BodyType::b2_staticBody);
        block->AddComponent(box);
        box->CreateBody({ 0,0 }, { block->Size3D().x, block->Size3D().y }, true);
        ImageRender* ir = new ImageRender(m_land);
        block->AddComponent(ir);
        ir->SetTrans(false);
        block->InitializeSet();
        rowGroup->push_back(block);
	}
	break;
	case MapType::DefaultMon:
	{
       GameObject* defaultMon = new GameObject();
       defaultMon->SetTag(TAG_MONSTER);
       defaultMon->Size3D() = { UNITSIZE, UNITSIZE, 1.0f};
       defaultMon->SetPosition({ (float)UNITSIZE * i + UNITSIZE / 4, (float)-UNITSIZE * j - UNITSIZE / 4 - 2,7.0f });
       defaultMon->AddComponent(new ChangeObject(PlayerMode::mDefault, m_player));
       defaultMon->AddComponent(new MonsterAI(m_defaultMobAnim[(int)Arrow::left], m_defaultMobAnim[(int)Arrow::right]));
       defaultMon->InitializeSet();
       rowGroup->push_back(defaultMon);
    }
        break;
    case MapType::SwordMon:
    {
        GameObject* swordMon = new GameObject();
        swordMon->SetTag(TAG_MONSTER);
        swordMon->Size3D() = { UNITSIZE, UNITSIZE, 1.0f};
        swordMon->SetPosition({ (float)UNITSIZE * i + UNITSIZE / 4 ,(float)-UNITSIZE * j - UNITSIZE / 4 - 2,7.0f});
        swordMon->AddComponent(new ChangeObject(PlayerMode::mSword, m_player));
        swordMon->AddComponent(new MonsterAI(m_swordMobAnim[(int)Arrow::left], m_swordMobAnim[(int)Arrow::right]));
        swordMon->InitializeSet();
        rowGroup->push_back(swordMon);
    }
	break;
	case MapType::StoneMon:
	{
		GameObject* stoneMon = new GameObject();
		stoneMon->SetTag(TAG_MONSTER);
		stoneMon->Size3D() = { UNITSIZE, UNITSIZE, 1.0f};
		stoneMon->SetPosition({ (float)UNITSIZE * i + UNITSIZE / 4, (float)-UNITSIZE * j - UNITSIZE / 4 - 2,7.0f});
		stoneMon->AddComponent(new ChangeObject(PlayerMode::mStone, m_player));
        stoneMon->AddComponent(new MonsterAI(m_stoneMobAnim[(int)Arrow::left], m_stoneMobAnim[(int)Arrow::right]));
		stoneMon->InitializeSet();
        rowGroup->push_back(stoneMon);
	}
        break;
    case MapType::Door:
    {
        GameObject* door = new GameObject();
        door->SetTag(TAG_DOOR);
        door->Size3D() = { UNITSIZE, UNITSIZE, 1.0f};
        door->SetPosition({ (float)UNITSIZE * i, (float)-UNITSIZE * j,7.0f});
        BoxCollider* box = new BoxCollider(b2BodyType::b2_kinematicBody);
        door->AddComponent(box);
        door->AddComponent(new ImageRender(m_door));
        box->SetTrigger(true);  
        door->AddComponent(new Door());
        box->CreateBody({ 0,0 }, { door->Size3D().x, door->Size3D().y }, false);
        door->InitializeSet();
        rowGroup->push_back(door);
    }
    break;
    }
}

StageMaker* StageMaker::GetInstance()
{
    return m_Pthis;
}

void StageMaker::Create()
{
    if (m_Pthis == nullptr)
    {
        m_Pthis = new StageMaker();
        GameObject* obj = new GameObject();
        obj->AddComponent(m_Pthis);
        obj->SetTag("StageMaker");
        obj->InitializeSet();
    }
}

void StageMaker::Destroy()
{
    if (m_Pthis != nullptr)
    {
        if (m_Pthis->m_gameObj != nullptr)
        {
            m_Pthis->m_gameObj->SetDestroy(true);
        }
        else
        {
            delete m_Pthis;
        }
        m_Pthis = nullptr;
    }
}

void StageMaker::StageStart()
{
    if (m_playerObj && m_playerObj->GetActive() == false && !m_mapObj.empty())
        m_playerObj->SetActive(true);
}

bool StageMaker::SetMap(string mapName)
{
    m_land = AnimationManager::LoadTexture("Bitmaps\\obj\\land.bmp");
    m_bg = AnimationManager::LoadTexture("Bitmaps\\obj\\BG.bmp");
    m_defaultObj = AnimationManager::LoadTexture("Bitmaps\\obj\\defaultObj.bmp");
    m_swordObj = AnimationManager::LoadTexture("Bitmaps\\obj\\swordObj.bmp");
    m_stoneObj = AnimationManager::LoadTexture("Bitmaps\\obj\\stoneObj.bmp");
    m_door = AnimationManager::LoadTexture("Bitmaps\\obj\\door.bmp");

    wstring path[(int)Arrow::max];
    path[(int)Arrow::left] = L"Bitmaps\\monster\\default\\left";
    path[(int)Arrow::right] = L"Bitmaps\\monster\\default\\right";
    m_defaultMobAnim[(int)Arrow::left] = AnimationManager::LoadAnimation(path[(int)Arrow::left], 0.15f);
    m_defaultMobAnim[(int)Arrow::right] = AnimationManager::LoadAnimation(path[(int)Arrow::right], 0.15f);
    path[(int)Arrow::left] = L"Bitmaps\\monster\\sword\\left";
    path[(int)Arrow::right] = L"Bitmaps\\monster\\sword\\right";
    m_swordMobAnim[(int)Arrow::left] = AnimationManager::LoadAnimation(path[(int)Arrow::left], 0.15f);
    m_swordMobAnim[(int)Arrow::right] = AnimationManager::LoadAnimation(path[(int)Arrow::right], 0.15f);
    path[(int)Arrow::left] = L"Bitmaps\\monster\\stone\\left";
    path[(int)Arrow::right] = L"Bitmaps\\monster\\stone\\right";
    m_stoneMobAnim[(int)Arrow::left] = AnimationManager::LoadAnimation(path[(int)Arrow::left], 0.15f);
    m_stoneMobAnim[(int)Arrow::right] = AnimationManager::LoadAnimation(path[(int)Arrow::right], 0.15f);


	for (vector<vector<GameObject*>>::iterator itr = m_mapObj.begin(); itr != m_mapObj.end(); itr++)
		itr->clear();
	m_mapObj.clear();

    vector<string> mapData = ReadMapData(mapName);
    if (mapData.empty())
    {
        cout << "맵 데이터 없음" << endl;
        m_name = "";
        return false;
    }
    m_name = mapName;
    for (int i = 0; i < mapData.size(); ++i)
	{
		vector<GameObject*> row;
		for (int j = 0; j < mapData[i].size(); ++j)
		{
			MakeMap((MapType)(mapData[i][j] - '0'), j, i, &row);
			if ((MapType)(mapData[i][j] - '0') != MapType::None)
				cout << "Create Tile Type : " << MapTypeToString((MapType)(mapData[i][j] - '0')) << endl;
		}
		m_mapObj.push_back(row);
	}
    return true;
}

void StageMaker::SetPlayerMode(PlayerMode mode)
{
    //m_player->SetPlayerMode(mode);
}

void StageMaker::Initialize()
{
    m_playerObj = new GameObject();
    m_player = new Player();
    m_playerObj->AddComponent(m_player);
    m_playerObj->InitializeSet();

    m_playerObj->SetActive(false);
}

void StageMaker::Release()
{

    if(m_land)AnimationManager::ReleaseTexture(m_land);
    if(m_bg)AnimationManager::ReleaseTexture(m_bg);
    if(m_defaultObj)AnimationManager::ReleaseTexture(m_defaultObj);
    if(m_swordObj)AnimationManager::ReleaseTexture(m_swordObj);
    if(m_stoneObj)AnimationManager::ReleaseTexture(m_stoneObj);
    if(m_door)AnimationManager::ReleaseTexture(m_door);
 
   AnimationManager::ReleaseAnimation(m_defaultMobAnim[(int)Arrow::left]);
   AnimationManager::ReleaseAnimation(m_defaultMobAnim[(int)Arrow::right]);

   AnimationManager::ReleaseAnimation(m_swordMobAnim[(int)Arrow::left]);
   AnimationManager::ReleaseAnimation(m_swordMobAnim[(int)Arrow::right]);
   
   AnimationManager::ReleaseAnimation(m_stoneMobAnim[(int)Arrow::left]);
   AnimationManager::ReleaseAnimation(m_stoneMobAnim[(int)Arrow::right]);

}

void StageMaker::Start()
{
}

void StageMaker::Update()
{
   
}

string StageMaker::GetMapName()
{
    return m_name;
}
