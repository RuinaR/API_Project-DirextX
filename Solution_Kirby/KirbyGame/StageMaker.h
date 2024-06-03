#pragma once
#include "Player.h"
#include "MonsterAI.h"
class StageMaker : public Component
{
protected:
	static StageMaker* m_Pthis;
protected:
	vector<string> ReadMapData(string mapName);
	void MakeMap(MapType t, int i, int j, vector<GameObject*>* rowGroup, vector<bool>* colRow);

	HBITMAP m_bg = NULL;
	HBITMAP m_land = NULL;
	HBITMAP m_swordObj = NULL;
	HBITMAP m_stoneObj = NULL;
	HBITMAP m_defaultObj = NULL;
	HBITMAP m_door = NULL;

	Player* m_player = nullptr;
	GameObject* m_playerObj = nullptr;
	vector<vector<GameObject*>> m_mapObj = vector<vector<GameObject*>>();
	vector<vector<bool>> m_colInfo = vector<vector<bool>>();

	string m_name;
public:
	static StageMaker* GetInstance();
	static void Create();
	static void Destroy();
public:
	void StageStart();
	bool SetMap(string mapName);
	void SetPlayerMode(PlayerMode mode);
	void Initialize() override;
	void Release() override;
	void Start() override;
	void Update() override;
	string GetMapName();
};

