#include "pch.h"
#include "Edit.h"
#include "UIButton.h"
#include "UILabel.h"

namespace
{
	const float kPaletteX = 40.0f;
	const float kPaletteY = 40.0f;
	const float kPaletteButtonW = 118.0f;
	const float kPaletteButtonH = 34.0f;
	const float kPaletteGap = 8.0f;
	const float kGridX = 40.0f;
	const float kGridY = 150.0f;
	const float kTileSize = 22.0f;
	const float kTileGap = 2.0f;

	const wchar_t* ToWideTileName(MapType type)
	{
		switch (type)
		{
		case MapType::None:
			return L"Empty";
		case MapType::Player:
			return L"Player";
		case MapType::Block:
			return L"Block";
		case MapType::DefaultMon:
			return L"DefaultMon";
		case MapType::SwordMon:
			return L"SwordMon";
		case MapType::StoneMon:
			return L"StoneMon";
		case MapType::Door:
			return L"Door";
		default:
			return L"Unknown";
		}
	}
}

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
	CreatePaletteUI();
	CreateTileMapUI();
}

void Edit::Update()
{
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
	RefreshTileUI();
}

void Edit::CreatePaletteUI()
{
	if (!m_paletteButtons.empty())
	{
		return;
	}

	for (int i = 0; i < (int)MapType::max; i++)
	{
		const float x = kPaletteX + (i * (kPaletteButtonW + kPaletteGap));
		D3DXVECTOR2 buttonPosition(x, kPaletteY);
		D3DXVECTOR2 buttonSize(kPaletteButtonW, kPaletteButtonH);
		UIButton* button = CreateUIButton(&buttonPosition, &buttonSize, 100);
		button->SetOnClick(bind(&Edit::SelectTileType, this, i));
		m_paletteButtons.push_back(button);

		D3DXVECTOR2 labelPosition(x + 10.0f, kPaletteY + 8.0f);
		D3DXVECTOR2 labelSize(kPaletteButtonW - 20.0f, 20.0f);
		UILabel* label = CreateUILabel(&labelPosition, &labelSize, ToWideTileName((MapType)i), 110);
		m_paletteLabels.push_back(label);
	}

	RefreshPaletteUI();
}

void Edit::CreateTileMapUI()
{
	if (!m_tileButtons.empty())
	{
		RefreshTileUI();
		return;
	}

	m_tileButtons.resize(m_count * m_count, nullptr);
	for (int i = 0; i < m_count; i++)
	{
		for (int j = 0; j < m_count; j++)
		{
			const int index = i * m_count + j;
			const float x = kGridX + (j * (kTileSize + kTileGap));
			const float y = kGridY + (i * (kTileSize + kTileGap));
			D3DXVECTOR2 buttonPosition(x, y);
			D3DXVECTOR2 buttonSize(kTileSize, kTileSize);
			UIButton* button = CreateUIButton(&buttonPosition, &buttonSize, 90);
			button->SetOnClick(bind(&Edit::SetTile, this, i, j));
			m_tileButtons[index] = button;
			RefreshTileButton(i, j);
		}
	}
}

void Edit::RefreshPaletteUI()
{
	for (int i = 0; i < m_paletteButtons.size(); i++)
	{
		UIButton* button = m_paletteButtons[i];
		if (!button)
		{
			continue;
		}

		const bool selected = i == m_SelectedTileIndex;
		IDirect3DTexture9* texture = TextureHandle[i];
		if (texture)
		{
			button->SetTexture(texture);
			button->SetUseTexture(true);
			button->SetStateColors(
				selected ? D3DCOLOR_ARGB(255, 255, 245, 185) : D3DCOLOR_ARGB(255, 255, 255, 255),
				selected ? D3DCOLOR_ARGB(255, 255, 250, 210) : D3DCOLOR_ARGB(255, 230, 230, 230),
				selected ? D3DCOLOR_ARGB(255, 230, 210, 120) : D3DCOLOR_ARGB(255, 190, 190, 190));
		}
		else
		{
			D3DCOLOR baseColor = selected ? D3DCOLOR_ARGB(255, 255, 235, 130) : D3DCOLOR_ARGB(255, 245, 245, 245);
			D3DCOLOR hoverColor = selected ? D3DCOLOR_ARGB(255, 255, 245, 170) : D3DCOLOR_ARGB(255, 220, 235, 255);
			D3DCOLOR pressedColor = selected ? D3DCOLOR_ARGB(255, 230, 200, 80) : D3DCOLOR_ARGB(255, 190, 210, 230);
			button->SetUseTexture(false);
			button->SetStateColors(baseColor, hoverColor, pressedColor);
		}
	}
}

void Edit::RefreshTileUI()
{
	for (int i = 0; i < m_count; i++)
	{
		for (int j = 0; j < m_count; j++)
		{
			RefreshTileButton(i, j);
		}
	}
}

void Edit::RefreshTileButton(int i, int j)
{
	if (i < 0 || j < 0 || i >= m_count || j >= m_count)
	{
		return;
	}

	const int index = i * m_count + j;
	if (index < 0 || index >= m_tileButtons.size() || !m_tileButtons[index])
	{
		return;
	}

	UIButton* button = m_tileButtons[index];
	MapType type = m_mapTypeData[i][j];
	IDirect3DTexture9* texture = TextureHandle[(int)type];
	if (texture)
	{
		button->SetTexture(texture);
		button->SetUseTexture(true);
		button->SetStateColors(D3DCOLOR_ARGB(255, 255, 255, 255), D3DCOLOR_ARGB(255, 230, 255, 230), D3DCOLOR_ARGB(255, 190, 220, 190));
	}
	else
	{
		button->SetUseTexture(false);
		D3DCOLOR color = GetTileColor(type);
		button->SetStateColors(color, D3DCOLOR_ARGB(255, 210, 225, 240), D3DCOLOR_ARGB(255, 170, 190, 210));
	}
}

void Edit::SetTile(int i, int j)
{
	if (i < 0 || j < 0 || i >= m_count || j >= m_count)
	{
		return;
	}

	if ((MapType)m_SelectedTileIndex == MapType::Player)
	{
		m_mapTypeData[playerI][playerJ] = MapType::None;
		RefreshTileButton(playerI, playerJ);
		playerI = i;
		playerJ = j;
		m_mapTypeData[i][j] = MapType::Player;
		RefreshTileButton(i, j);
	}
	else if (m_mapTypeData[i][j] != MapType::Player)
	{
		m_mapTypeData[i][j] = (MapType)m_SelectedTileIndex;
		RefreshTileButton(i, j);
	}
}

void Edit::SelectTileType(int tileType)
{
	m_SelectedTileIndex = tileType;
	RefreshPaletteUI();
}

UIButton* Edit::CreateUIButton(const D3DXVECTOR2* position, const D3DXVECTOR2* size, int orderInLayer)
{
	if (!position || !size)
	{
		return nullptr;
	}

	GameObject* obj = new GameObject();
	UIButton* button = new UIButton();
	obj->AddComponent(button);
	obj->InitializeSet();
	obj->SetParent(m_gameObj);
	button->SetPosition(position);
	button->SetSize(size);
	button->SetUseTexture(false);
	button->SetOrderInLayer(orderInLayer);
	return button;
}

UILabel* Edit::CreateUILabel(const D3DXVECTOR2* position, const D3DXVECTOR2* size, const wchar_t* text, int orderInLayer)
{
	if (!position || !size)
	{
		return nullptr;
	}

	GameObject* obj = new GameObject();
	UILabel* label = new UILabel();
	obj->AddComponent(label);
	obj->InitializeSet();
	obj->SetParent(m_gameObj);
	label->SetPosition(position);
	label->SetSize(size);
	label->SetText(text);
	label->SetColor(D3DCOLOR_ARGB(255, 35, 35, 35));
	label->SetFontSize(14);
	label->SetOrderInLayer(orderInLayer);
	return label;
}

D3DCOLOR Edit::GetTileColor(MapType type)
{
	switch (type)
	{
	case MapType::None:
		return D3DCOLOR_ARGB(180, 60, 70, 80);
	case MapType::Player:
		return D3DCOLOR_ARGB(255, 120, 210, 255);
	case MapType::Block:
		return D3DCOLOR_ARGB(255, 120, 120, 120);
	case MapType::DefaultMon:
		return D3DCOLOR_ARGB(255, 230, 120, 120);
	case MapType::SwordMon:
		return D3DCOLOR_ARGB(255, 180, 120, 230);
	case MapType::StoneMon:
		return D3DCOLOR_ARGB(255, 160, 150, 130);
	case MapType::Door:
		return D3DCOLOR_ARGB(255, 210, 165, 80);
	default:
		return D3DCOLOR_ARGB(255, 255, 255, 255);
	}
}
