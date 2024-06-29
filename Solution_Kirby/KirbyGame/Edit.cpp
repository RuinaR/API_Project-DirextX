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

void Edit::InitMap()
{
    m_mapTypeData.clear();

    for (int i = 0; i < m_count; i++)
    {
        vector<MapType> row;
        for (int j = 0; j < m_count; j++)
        {
            row.push_back(MapType::None);
        }
        m_mapTypeData.push_back(row);
    }
    m_mapTypeData[PLAYER_DEFAULT_X][PLAYER_DEFAULT_Y] = MapType::Player;
}

void Edit::Initialize()
{
    m_land = AnimationManager::LoadTexture(L"Bitmaps\\obj\\land.bmp");
    m_bg = AnimationManager::LoadTexture(L"Bitmaps\\obj\\BG.bmp");
    m_defaultObj = AnimationManager::LoadTexture(L"Bitmaps\\obj\\defaultObj.bmp");
    m_swordObj = AnimationManager::LoadTexture(L"Bitmaps\\obj\\swordObj.bmp");
    m_stoneObj = AnimationManager::LoadTexture(L"Bitmaps\\obj\\stoneObj.bmp");
    m_player = AnimationManager::LoadTexture(L"Bitmaps\\obj\\player.bmp");
	m_door = AnimationManager::LoadTexture(L"Bitmaps\\obj\\door.bmp");

    //Ÿ�ϼ� ������
    Tileset[(int)MapType::None] = "Empty";
    Tileset[(int)MapType::Block] = "Block";
    Tileset[(int)MapType::Player] = "Player";
    Tileset[(int)MapType::DefaultMon] = "DefaultMon";
    Tileset[(int)MapType::SwordMon] = "SwordMon";
    Tileset[(int)MapType::StoneMon] = "StoneMon";
    Tileset[(int)MapType::Door] = "Door";

    TextureHandle[(int)MapType::None] = nullptr;
    TextureHandle[(int)MapType::Block] = m_land;
    TextureHandle[(int)MapType::Player] = m_player;
    TextureHandle[(int)MapType::DefaultMon] = m_defaultObj;
    TextureHandle[(int)MapType::SwordMon] = m_swordObj;
    TextureHandle[(int)MapType::StoneMon] = m_stoneObj;
    TextureHandle[(int)MapType::Door] = m_door;
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
    Camera::GetInstance()->SetPos(UNITSIZE * 10, -UNITSIZE * 5);
}

void Edit::Update()
{
    ImGui::Begin("Tile Map Editor");
    // Ÿ�ϼ� ���� ����
    ImGui::Text("Tileset:");
    ImGui::SameLine();
    ImGui::Combo("##TilesetCombo", &m_SelectedTileIndex, Tileset, (int)MapType::max);
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.6f, 0.6f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.7f, 0.7f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.8f, 0.8f, 0.8f));
    // Ÿ�� �� �׸���
    for (int i = 0; i < m_mapTypeData.size(); ++i)
    {
        for (int j = 0; j < m_mapTypeData[i].size(); ++j)
        {
            ImGui::PushID(j * m_mapTypeData.size() + i);
            // �� Ÿ���� ��ư���� �׸���
            if (m_mapTypeData[i][j] != MapType::None)
            {
                if (ImGui::ImageButton(
                    (ImTextureID)TextureHandle[(int)m_mapTypeData[i][j]],
                    ImVec2(42, 42),
                    ImVec2(0, 0),
					ImVec2(1, 1),
					-1,
					ImVec4(0, 0, 0, 0),
					ImVec4(1, 1, 1, 1)
				))
				{
                    if ((MapType)m_SelectedTileIndex == MapType::Player)
                    {
                        m_mapTypeData[playerI][playerJ] = MapType::None;
                        playerI = i;
                        playerJ = j;
                        m_mapTypeData[i][j] = MapType::Player;
                    }
					else if (m_mapTypeData[i][j] != MapType::Player)
						m_mapTypeData[i][j] = (MapType)m_SelectedTileIndex;
				}
			}
			else
			{
				if (ImGui::Button(Tileset[(int)m_mapTypeData[i][j]], ImVec2(50, 50)))
				{
                    if ((MapType)m_SelectedTileIndex == MapType::Player)
                    {
                        m_mapTypeData[playerI][playerJ] = MapType::None;
                        playerI = i;
                        playerJ = j;
                        m_mapTypeData[i][j] = MapType::Player;
                    }
                    else if (m_mapTypeData[i][j] != MapType::Player)
                        m_mapTypeData[i][j] = (MapType)m_SelectedTileIndex;
				}
			}


            ImGui::PopID();
			if (j + 1 != m_count)
				ImGui::SameLine();
		}
	}
    ImGui::PopStyleColor(3);
	ImGui::End();
}

void Edit::SetMap(string mapName)
{
    InitMap();
    vector<string> strMapData = ReadMapData(mapName);
    for (int i = 0; i < strMapData.size(); i++)
    {
        for (int j = 0; j < strMapData[i].size(); j++)
        {
            m_mapTypeData[i][j] = (MapType)(strMapData[i][j] - '0');
        }
    }
}