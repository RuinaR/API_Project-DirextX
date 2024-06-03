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
	HBITMAP m_bg = NULL;
	HBITMAP m_land = NULL;
	HBITMAP m_swordObj = NULL;
	HBITMAP m_stoneObj = NULL;
	HBITMAP m_defaultObj = NULL;
	HBITMAP m_player = NULL;
	HBITMAP m_door = NULL;

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

