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

	IDirect3DTexture9* m_bg = NULL;
	IDirect3DTexture9* m_land = NULL;
	IDirect3DTexture9* m_swordObj = NULL;
	IDirect3DTexture9* m_stoneObj = NULL;
	IDirect3DTexture9* m_defaultObj = NULL;
	IDirect3DTexture9* m_door = NULL;

	Player* m_player = nullptr;
	GameObject* m_playerObj = nullptr;
	vector<vector<GameObject*>> m_mapObj = vector<vector<GameObject*>>();
	vector<vector<bool>> m_colInfo = vector<vector<bool>>();

	string m_name;

	Animation m_defaultMobAnim[(int)Arrow::max];
	Animation m_swordMobAnim[(int)Arrow::max];
	Animation m_stoneMobAnim[(int)Arrow::max];

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

