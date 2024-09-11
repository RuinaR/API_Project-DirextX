#pragma once
#include "FBXRender.h"
#include "ImageRender.h"

#define WINDOWTEXT "Render Target Window"

using namespace std;
class ImageRender;
class Button;
class RenderManager
{
private:
	static RenderManager* m_Pthis;
	vector<ImageRender*>* m_noTransVec;
	vector<FBXRender*>* m_fbxVec;
	vector<ImageRender*>* m_transVec;
	vector<Button*>* m_btnVec;
	vector<DebugRender*>* m_debugVec;

	ImVec2 m_winPos;
	LPDIRECT3DTEXTURE9 renderTargetTexture = nullptr;
public:
	static void Create();
	static RenderManager* GetInstance();
	static void Destroy();

public:
	void Resister(ImageRender* ir);
	void Unresister(ImageRender* ir);
	void Resister(FBXRender* fbxr);
	void Unresister(FBXRender* fbxr);
	void ResisterBtn(Button* btn);
	void UnresisterBtn(Button* btn);
	void ResisterDebug(DebugRender* db);
	void UnresisterDebug(DebugRender* db);

	void Initialize();
	void EditUpdate();
	void GameUpdate();
	void Release();

	ImVec2 GetWinPos();
};

