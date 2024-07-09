#pragma once

#define WINDOWTEXT "Render Target Window"
using namespace std;
class ImageRender;
class Button;
class RenderManager
{
private:
	static RenderManager* m_Pthis;
	SharedPointer<vector<ImageRender*>> m_noTransVec;
	SharedPointer<vector<ImageRender*>> m_transVec;
	SharedPointer<vector<Button*>> m_btnVec;
	SharedPointer<vector<DebugRender*>> m_debugVec;

	ImVec2 m_winPos;
	LPDIRECT3DTEXTURE9 renderTargetTexture = nullptr;
public:
	static void Create();
	static RenderManager* GetInstance();
	static void Destroy();

public:
	void Resister(ImageRender* ir);
	void Unresister(ImageRender* ir);
	void ResisterBtn(Button* btn);
	void UnresisterBtn(Button* btn);
	void ResisterDebug(DebugRender* db);
	void UnresisterDebug(DebugRender* db);

	void Initialize();
	void Update();
	void Release();

	ImVec2 GetWinPos();
};

