#pragma once
#include "Component.h"
#include "RenderManager.h"
class WindowFrame;

class ImageRender : public Component
{
private:
	IDirect3DDevice9* m_device = nullptr;
	IDirect3DTexture9* m_texture = nullptr;
	IDirect3DVertexBuffer9* m_vertexBuffer = nullptr;
	IDirect3DIndexBuffer9* m_indexBuffer = nullptr;
	bool m_isTrans = true;
	bool m_isUIRender = false;
	bool m_renderEnabled = true;
	bool m_useTexture = true;
	int m_orderInLayer = 0;
	D3DCOLOR m_color = 0xffffffff;
	

	void SetupVertices();
	void DrawImage(int x, int y, int z, int w, int h);

public:
	void InitGameObj(GameObject* obj) override;
	void LoadTextureCallback(IDirect3DTexture9* tex);
	float GetZ();
	void Render();
	ImageRender(IDirect3DTexture9* texture);
	void SetTrans(bool trans);
	bool IsTrans();
	void SetUIRender(bool isUIRender);
	bool IsUIRender();
	void SetOrderInLayer(int orderInLayer);
	int GetOrderInLayer();
	void SetRenderEnabled(bool renderEnabled);
	bool IsRenderEnabled();
	void SetUseTexture(bool useTexture);
	bool IsUseTexture();
	void SetColor(D3DCOLOR color);
	D3DCOLOR GetColor();
	void Initialize() override;
	void Release() override;
	void ChangeTexture(IDirect3DTexture9* texture);
	void Start() override;
	void Update() override;
};

