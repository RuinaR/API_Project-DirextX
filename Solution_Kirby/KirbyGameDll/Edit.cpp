#include "pch.h"
#include "Edit.h"

vector<string> Edit::ReadMapData(string mapName)
{
	m_mapName = mapName;
	//char buffer[MAX_PATH];
	vector<string> mapData;
	//GetCurrentDirectoryA(MAX_PATH, buffer);
	//string currentDirectory = buffer;


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
	wchar_t path[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, path, MAX_PATH);
	USES_CONVERSION;
	std::string executepath = W2A(path);
	executepath = executepath.substr(0, executepath.find_last_of("\\/"));
	string currentDirectory = executepath;

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
	m_land = AnimationManager::LoadTexture("Bitmaps\\obj\\land.bmp");
	m_bg = AnimationManager::LoadTexture("Bitmaps\\obj\\BG.bmp");
	m_defaultObj = AnimationManager::LoadTexture("Bitmaps\\obj\\defaultObj.bmp");
	m_swordObj = AnimationManager::LoadTexture("Bitmaps\\obj\\swordObj.bmp");
	m_stoneObj = AnimationManager::LoadTexture("Bitmaps\\obj\\stoneObj.bmp");
	m_player = AnimationManager::LoadTexture("Bitmaps\\obj\\player.bmp");
	m_door = AnimationManager::LoadTexture("Bitmaps\\obj\\door.bmp");

	//타일셋 데이터
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
	// 타일셋 선택 상자
	ImGui::Text("Tileset:");
	ImGui::SameLine();
	ImGui::Combo("##TilesetCombo", &m_SelectedTileIndex, Tileset, (int)MapType::max);
	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.6f, 0.6f, 0.6f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.7f, 0.7f, 0.7f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.8f, 0.8f, 0.8f));
	// 타일 맵 그리드
	for (int i = 0; i < m_mapTypeData.size(); ++i)
	{
		for (int j = 0; j < m_mapTypeData[i].size(); ++j)
		{
			ImGui::PushID(j * m_mapTypeData.size() + i);
			// 각 타일을 버튼으로 그리기
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
	for (int i = 0; i < strMapData.size() && i < m_mapTypeData.size(); i++)
	{
		for (int j = 0; j < strMapData[i].size() && j < m_mapTypeData.size(); j++)
		{
			m_mapTypeData[i][j] = (MapType)(strMapData[i][j] - '0');
		}
	}
}