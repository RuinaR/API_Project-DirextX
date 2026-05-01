#pragma once
#include "FBXRender.h"
#include "ImageRender.h"

#define WINDOWTEXT "Render Target Window"

using namespace std;
class ImageRender;
class ImguiButton;
class UIElement;
class RenderManager
{
private:
	static RenderManager* m_Pthis;
	vector<ImageRender*>* m_noTransVec;
	vector<FBXRender*>* m_fbxVec;
	vector<ImageRender*>* m_transVec;
	vector<ImguiButton*>* m_btnVec;
	vector<DebugRender*>* m_debugVec;

	struct UIRenderEntry
	{
		ImageRender* render = nullptr;
		UIElement* element = nullptr;
		int orderInLayer = 0;
		unsigned int registrationOrder = 0;
	};

	struct DebugLine
	{
		D3DXVECTOR3 start = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		D3DXVECTOR3 end = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		D3DCOLOR color = D3DCOLOR_XRGB(255, 0, 0);
	};

	vector<UIRenderEntry>* m_uiRenderVec;
	vector<DebugLine>* m_immediateDebugLines;
	unsigned int m_nextUIRenderRegistrationOrder = 0;

	ImVec2 m_winPos;
	D3DXVECTOR2 m_gameViewPos = D3DXVECTOR2(0.0f, 0.0f);
	D3DXVECTOR2 m_gameViewScreenPos = D3DXVECTOR2(0.0f, 0.0f);
	D3DXVECTOR2 m_gameViewSize = D3DXVECTOR2(0.0f, 0.0f);
	bool m_useScreenSpaceUIMouse = false;
	bool m_showColliderDebug = false;
	LPDIRECT3DTEXTURE9 renderTargetTexture = nullptr;

	float CalculateCameraDepth(ImageRender* ir);
	void SortWorldRenderQueues();
	void SortUIQueue();
	void RenderImmediateDebugLines();
	void RenderSelectedObjectMarker(const ImVec2& imageScreenPos);
	void RenderUIQueue();
public:
	static void Create();
	static RenderManager* GetInstance();
	static void Destroy();

public:
	void Register(ImageRender* ir);
	void Unregister(ImageRender* ir);
	void RegisterUI(ImageRender* ir);
	void UnregisterUI(ImageRender* ir);
	void RefreshUIOrder(ImageRender* ir);
	void RegisterUI(UIElement* element);
	void UnregisterUI(UIElement* element);
	void RefreshUIOrder(UIElement* element);
	bool IsTopUIRenderAt(ImageRender* ir, const D3DXVECTOR2* point);
	void Register(FBXRender* fbxr);
	void Unregister(FBXRender* fbxr);
	void RegisterBtn(ImguiButton* btn);
	void UnregisterBtn(ImguiButton* btn);
	void RegisterDebug(DebugRender* db);
	void UnregisterDebug(DebugRender* db);
	void SetColliderDebugVisible(bool visible);
	bool IsColliderDebugVisible() const;
	void ClearImmediateDebugLines();
	void AddImmediateDebugLine(const D3DXVECTOR3& start, const D3DXVECTOR3& end, D3DCOLOR color = D3DCOLOR_XRGB(255, 0, 0));
	void AddImmediateDebugCross(const D3DXVECTOR3& center, float halfSize = 5.0f, D3DCOLOR color = D3DCOLOR_XRGB(255, 0, 0));

	void Initialize();
	void EditUpdate();
	void GameUpdate();
	void ReleaseRenderTargetTexture();
	bool CreateRenderTargetTexture();
	void Release();

	ImVec2 GetWinPos();
	D3DXVECTOR2 ScreenToUICoordinate(const D3DXVECTOR2* screenPosition);
	D3DXVECTOR2 GetMouseUICoordinate();
	bool IsUIMouseLeftDown();
	D3DXVECTOR2 GetGameViewPos();
	D3DXVECTOR2 GetGameViewScreenPos();
	D3DXVECTOR2 GetGameViewSize();
	bool IsUsingScreenSpaceUIMouse();

	static int FrameCount;
};

