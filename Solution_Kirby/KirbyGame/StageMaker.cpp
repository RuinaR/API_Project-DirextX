#include "pch.h"
#include "StageMaker.h"
#include "Player.h"
#include "BoxCollider.h"
#include "ChangeObject.h"

StageMaker* StageMaker::m_Pthis = nullptr;

bool EqualFloat(float f1, float f2, float d)
{
	if (f1 + d > f2 && f1 - d < f2)
		return true;

	return false;
}

vector<string> StageMaker::ReadMapData(string mapName)
{
    char buffer[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, buffer);
    string currentDirectory = buffer;
    vector<string> mapData;
    // 맵 파일 경로 설정
    string mapFilePath = currentDirectory + "\\Map\\" + mapName + ".txt";

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

void StageMaker::MakeMap(MapType t, int i, int j, vector<GameObject*>* rowGroup, vector<bool>* colRow)
{
	switch (t)
	{
	case MapType::None:
    {
        rowGroup->push_back(nullptr);
        colRow->push_back(false);
    }
		break;
	case MapType::Player:
    {
        m_playerObj->SetPosition({ (float)UNITSIZE * i,(float)-UNITSIZE * j,5.0f });
        rowGroup->push_back(m_playerObj);
        colRow->push_back(false);
	}
	break;
	case MapType::Block: //가로줄 콜라이더를 계산해서 합친 상태로 생성
	{
		GameObject* obj = new GameObject();
		obj->SetTag(TAG_LAND);
		obj->SetPosition({ (float)UNITSIZE * i,(float)-UNITSIZE * j,10.0f });
		obj->Size() = { UNITSIZE, UNITSIZE };
		obj->AddComponent(new BitmapRender(m_land));
        if (!rowGroup->empty() && (rowGroup->back() != nullptr && rowGroup->back()->GetTag() == TAG_LAND))
        {
            int startIdx = rowGroup->size() - 1;
            int endIdx = 0;
            for (int k = startIdx; k >= 0; k--)
            {
                if ((*rowGroup)[k] == nullptr || (*rowGroup)[k]->GetTag() != TAG_LAND)
				{
					endIdx = k + 1;
                    BoxCollider* bo = (*rowGroup)[endIdx]->GetComponent<BoxCollider>();
                    if (bo == nullptr)
                    {
                        BoxCollider* newBo = new BoxCollider(b2BodyType::b2_staticBody);
                        (*rowGroup)[endIdx]->AddComponent(newBo);
                        newBo->SetColSize({ (double)(UNITSIZE * (startIdx - endIdx + 2)) ,(double)UNITSIZE });
                    }
                    else
                    {
                        bo->SetColSize( { (double)(UNITSIZE * (startIdx - endIdx + 2)) ,(double)UNITSIZE });
                    }
                    for (int q = endIdx; q < startIdx + 1; q++)
                    {
                        (*colRow)[q] = true;
                    }
                    if (startIdx + 1 >= colRow->size())
                        colRow->push_back(true);
                    else
                        (*colRow)[startIdx + 1] = true;

					break;
				}
			}
		}
		obj->InitializeSet();
		rowGroup->push_back(obj);
		if (rowGroup->size() > colRow->size())
			colRow->push_back(false);
	}
	break;
	case MapType::DefaultMon:
	{
       GameObject* defaultMon = new GameObject();
       defaultMon->SetTag(TAG_MONSTER);
       defaultMon->Size() = { UNITSIZE, UNITSIZE};
       defaultMon->SetPosition({ (float)UNITSIZE * i + UNITSIZE / 4, (float)-UNITSIZE * j - UNITSIZE / 4 - 2,7.0f });
       defaultMon->AddComponent(new ChangeObject(PlayerMode::mDefault, m_player));
       defaultMon->AddComponent(new MonsterAI(m_defaultMobAnim[(int)Arrow::left], m_defaultMobAnim[(int)Arrow::right]));
       defaultMon->InitializeSet();
       rowGroup->push_back(defaultMon);
       colRow->push_back(false);
    }
        break;
    case MapType::SwordMon:
    {
        GameObject* swordMon = new GameObject();
        swordMon->SetTag(TAG_MONSTER);
        swordMon->Size() = { UNITSIZE, UNITSIZE};
        swordMon->SetPosition({ (float)UNITSIZE * i + UNITSIZE / 4 ,(float)-UNITSIZE * j - UNITSIZE / 4 - 2,7.0f});
        swordMon->AddComponent(new ChangeObject(PlayerMode::mSword, m_player));
        swordMon->AddComponent(new MonsterAI(m_swordMobAnim[(int)Arrow::left], m_swordMobAnim[(int)Arrow::right]));
        swordMon->InitializeSet();
        rowGroup->push_back(swordMon);
        colRow->push_back(false);
    }
	break;
	case MapType::StoneMon:
	{
		GameObject* stoneMon = new GameObject();
		stoneMon->SetTag(TAG_MONSTER);
		stoneMon->Size() = { UNITSIZE, UNITSIZE};
		stoneMon->SetPosition({ (float)UNITSIZE * i + UNITSIZE / 4, (float)-UNITSIZE * j - UNITSIZE / 4 - 2,7.0f});
		stoneMon->AddComponent(new ChangeObject(PlayerMode::mStone, m_player));
        stoneMon->AddComponent(new MonsterAI(m_stoneMobAnim[(int)Arrow::left], m_stoneMobAnim[(int)Arrow::right]));
		stoneMon->InitializeSet();
        rowGroup->push_back(stoneMon);
        colRow->push_back(false);
	}
        break;
    case MapType::Door:
    {
        GameObject* door = new GameObject();
        door->SetTag(TAG_DOOR);
        door->Size() = { UNITSIZE, UNITSIZE};
        door->SetPosition({ (float)UNITSIZE * i, (float)-UNITSIZE * j,7.0f});
        door->AddComponent(new BitmapRender(m_door));
        BoxCollider* bo = new BoxCollider(b2BodyType::b2_kinematicBody);
        bo->SetTrigger(true);
        door->AddComponent(bo);
        door->InitializeSet();
        rowGroup->push_back(door);
        colRow->push_back(false);
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
    m_land = AnimationManager::LoadTexture(L"Bitmaps\\obj\\land");
    m_bg = AnimationManager::LoadTexture(L"Bitmaps\\obj\\BG");
    m_defaultObj = AnimationManager::LoadTexture(L"Bitmaps\\obj\\defaultObj");
    m_swordObj = AnimationManager::LoadTexture(L"Bitmaps\\obj\\swordObj");
    m_stoneObj = AnimationManager::LoadTexture(L"Bitmaps\\obj\\stoneObj");
    m_door = AnimationManager::LoadTexture(L"Bitmaps\\obj\\door");

    string path[(int)Arrow::max];
    path[(int)Arrow::left] = "Bitmaps\\monster\\default\\left";
    path[(int)Arrow::right] = "Bitmaps\\monster\\default\\right";
    m_defaultMobAnim[(int)Arrow::left] = AnimationManager::LoadAnimation(path[(int)Arrow::left], 0.15f);
    m_defaultMobAnim[(int)Arrow::right] = AnimationManager::LoadAnimation(path[(int)Arrow::right], 0.15f);
    path[(int)Arrow::left] = "Bitmaps\\monster\\sword\\left";
    path[(int)Arrow::right] = "Bitmaps\\monster\\sword\\right";
    m_swordMobAnim[(int)Arrow::left] = AnimationManager::LoadAnimation(path[(int)Arrow::left], 0.15f);
    m_swordMobAnim[(int)Arrow::right] = AnimationManager::LoadAnimation(path[(int)Arrow::right], 0.15f);
    path[(int)Arrow::left] = "Bitmaps\\monster\\stone\\left";
    path[(int)Arrow::right] = "Bitmaps\\monster\\stone\\right";
    m_stoneMobAnim[(int)Arrow::left] = AnimationManager::LoadAnimation(path[(int)Arrow::left], 0.15f);
    m_stoneMobAnim[(int)Arrow::right] = AnimationManager::LoadAnimation(path[(int)Arrow::right], 0.15f);


	for (vector<vector<GameObject*>>::iterator itr = m_mapObj.begin(); itr != m_mapObj.end(); itr++)
		itr->clear();
	m_mapObj.clear();
    for (vector<vector<bool>>::iterator itr = m_colInfo.begin(); itr != m_colInfo.end(); itr++)
        itr->clear();
    m_colInfo.clear();

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
		vector<bool> colrow;
		for (int j = 0; j < mapData[i].size(); ++j)
		{
			MakeMap((MapType)(mapData[i][j] - '0'), j, i, &row, &colrow);
			if ((MapType)(mapData[i][j] - '0') != MapType::None)
				cout << "Create Tile Type : " << MapTypeToString((MapType)(mapData[i][j] - '0')) << endl;
		}
		m_mapObj.push_back(row);
		m_colInfo.push_back(colrow);
	}

	for (int i = 0; i < m_mapObj.size(); i++) //나머지 세로줄 콜라이더를 합쳐서 생성하는 작업
	{
		for (int j = 0; j < m_mapObj[i].size(); j++)
		{
			if (m_mapObj[i][j] != nullptr &&
				m_mapObj[i][j]->GetTag() == TAG_LAND &&
				m_mapObj[i][j]->GetComponent<BoxCollider>() == nullptr &&
                m_colInfo[i][j] == false)
            {
                int cnt = 1;
                m_colInfo[i][j] = true;
                for (int k = i + 1; k < m_mapObj.size(); k++)
                {
                    if (m_mapObj[k][j] != nullptr &&
                        m_mapObj[k][j]->GetTag() == TAG_LAND &&
                        m_mapObj[k][j]->GetComponent<BoxCollider>() == nullptr &&
                        m_colInfo[k][j] == false)
                    {
                        cnt++;
                        m_colInfo[k][j] = true;
                    }
                    else
                        break;
                }
                BoxCollider* newBo = new BoxCollider(b2BodyType::b2_staticBody);
                m_mapObj[i][j]->AddComponent(newBo);
                newBo->SetColSize({ UNITSIZE, (double)(UNITSIZE * cnt) });
            }
        }
    }

    //가로줄 콜라이더끼리 마지막으로 합치는 작업(가로줄 우선이므로 세로줄끼리 합치는 작업은 필요없음)
    for (int i = 0; i < m_mapObj.size(); i++)
    {
        for (int j = 0; j < m_mapObj[i].size(); j++)
        {
            if (m_mapObj[i][j] == nullptr || m_mapObj[i][j]->GetTag() != TAG_LAND)
                continue;
            BoxCollider* bo = m_mapObj[i][j]->GetComponent<BoxCollider>();
			int cnt = 1;
			if (bo != nullptr && EqualFloat(bo->GetColSize().y, UNITSIZE, 0.1f))
			{
				for (int k = i + 1; k < m_mapObj.size(); k++)
				{
                    if (m_mapObj[k][j] == nullptr || m_mapObj[k][j]->GetTag() != TAG_LAND)
                        break;
					BoxCollider* bo2 = m_mapObj[k][j]->GetComponent<BoxCollider>();
					if (bo2 == nullptr || !EqualFloat(bo->GetColSize().x, bo2->GetColSize().x, 0.1f))
					{
						break;
					}
					else
					{
						m_mapObj[k][j]->DeleteComponent(bo2);
                        cnt++;
					}
				}
                bo->SetColSize({ bo->GetColSize().x , (double)(UNITSIZE * cnt) });
			}
		}
	}
    return true;
}

void StageMaker::SetPlayerMode(PlayerMode mode)
{
    m_player->SetPlayerMode(mode);
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
    AnimationManager::ReleaseTexture(m_land);
    AnimationManager::ReleaseTexture(m_bg);
    AnimationManager::ReleaseTexture(m_defaultObj);
    AnimationManager::ReleaseTexture(m_swordObj);
    AnimationManager::ReleaseTexture(m_stoneObj);
    AnimationManager::ReleaseTexture(m_door);

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
