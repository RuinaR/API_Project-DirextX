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
	bool m_isTrans = true;
	

	void SetupVertices();
	void DrawImage(int x, int y, int z, int w, int h);
public:
	float GetZ();
	void Render();
	ImageRender(IDirect3DTexture9* texture);
	void SetTrans(bool trans);
	bool IsTrans();
	void Initialize() override;
	void Release() override;
	void ChangeTexture(IDirect3DTexture9* texture);
	void Start() override;
	void Update() override;
};

