#pragma once

using namespace std;
class ImageRender;
class ColorButton;
class RenderManager
{
private:
	static RenderManager* m_Pthis;
	vector<ImageRender*>* m_noTransVec;
	vector<ImageRender*>* m_transVec;
	vector<ColorButton*>* m_btnVec;
	vector<DebugRender*>* m_debugVec;
public:
	static void Create();
	static RenderManager* GetInstance();
	static void Destroy();

public:
	void Resister(ImageRender* ir);
	void Unresister(ImageRender* ir);
	void ResisterBtn(ColorButton* btn);
	void UnresisterBtn(ColorButton* btn);
	void ResisterDebug(DebugRender* db);
	void UnresisterDebug(DebugRender* db);

	void Initialize();
	void Update();
	void Release();
};

