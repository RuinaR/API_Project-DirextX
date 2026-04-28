#pragma once
#include "game.h"

class UIButton;
class UILabel;

class Edit : public Component
{
private:
	string m_mapName = "";
	vector<vector<MapType>> m_mapTypeData = vector<vector<MapType>>();
	vector<UIButton*> m_paletteButtons;
	vector<UIButton*> m_tileButtons;
	vector<UILabel*> m_paletteLabels;

	vector<string> ReadMapData(string mapName);
	int m_count = MAXUNITCNT;
	IDirect3DTexture9* m_bg = NULL;
	IDirect3DTexture9* m_land = NULL;
	IDirect3DTexture9* m_swordObj = NULL;
	IDirect3DTexture9* m_stoneObj = NULL;
	IDirect3DTexture9* m_defaultObj = NULL;
	IDirect3DTexture9* m_player = NULL;
	IDirect3DTexture9* m_door = NULL;
	int m_SelectedTileIndex = 0;
	const char* Tileset[(int)MapType::max];
	IDirect3DTexture9* TextureHandle[(int)MapType::max];
	int playerI = PLAYER_DEFAULT_X;
	int playerJ = PLAYER_DEFAULT_Y;

	void WriteMapData();
	void InitMap();
	void CreateTileMapUI();
	void CreatePaletteUI();
	void RefreshPaletteUI();
	void RefreshTileUI();
	void RefreshTileButton(int i, int j);
	void SetTile(int i, int j);
	void SelectTileType(int tileType);
	UIButton* CreateUIButton(const D3DXVECTOR2& position, const D3DXVECTOR2& size, int orderInLayer);
	UILabel* CreateUILabel(const D3DXVECTOR2& position, const D3DXVECTOR2& size, const wchar_t* text, int orderInLayer);
	D3DCOLOR GetTileColor(MapType type);
public:
	void Initialize() override;
	void Release() override;
	void Start() override;
	void Update() override;

	void SetMap(string mapName);
};

