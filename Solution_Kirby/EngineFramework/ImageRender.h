#pragma once
#include "Component.h"
class WindowFrame;

class ImageRender : public Component
{
private:
	IDirect3DDevice9* m_device = nullptr;
	IDirect3DTexture9* m_texture = nullptr;
	IDirect3DVertexBuffer9* m_vertexBuffer = nullptr;

	void DrawImage(int x, int y, int z, int w, int h);

	void SetupVertices();
public:
	ImageRender(IDirect3DTexture9* texture);

	void Initialize() override;
	void Release() override;
	void ChangeTexture(IDirect3DTexture9* texture);
	void Start() override;
	void Update() override;
};

