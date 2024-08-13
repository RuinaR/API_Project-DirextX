#pragma once
#include "Button.h"
#include "game.h"

class Edit : public Component
{
private:
	string m_mapName = "";
	vector<vector<MapType>> m_mapTypeData = vector<vector<MapType>>();

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
public:
	void Initialize() override;
	void Release() override;
	void Start() override;
	void Update() override;

	void SetMap(string mapName);
};

