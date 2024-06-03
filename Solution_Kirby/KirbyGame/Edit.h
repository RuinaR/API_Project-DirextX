#pragma once
#include "ColorButton.h"
#include "game.h"

class Edit : public Component
{
private:
	string m_mapName = "";
	vector<vector<GameObject*>> m_mapData = vector<vector<GameObject*>>();
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

	MapType m_select = MapType::None;

	stack<UndoRedoData> m_undoStack = stack<UndoRedoData>();
	stack<UndoRedoData> m_redoStack = stack<UndoRedoData>();

	ColorButton* m_selectBtn[(int)MapType::max];
	ColorButton* m_InitMapBtn = nullptr;
	void WriteMapData();
	GameObject* DrawMap(MapType t, int i, int j);
	void SelectNone();
	void SelectLand();
	void SelectDefaultMon();
	void SelectSwordMon();
	void SelectStoneMon();
	void SelectPlayer();
	void SelectDoor();
	void InitMap();
	void ReDrawMapObj(int indexX, int indexY, MapType type);
	void Undo();
	void Redo();
public:
	void Initialize() override;
	void Release() override;
	void Start() override;
	void Update() override;

	void SetMap(string mapName);
};

