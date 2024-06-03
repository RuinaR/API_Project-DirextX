#pragma once
#include "Component.h"
class WindowFrame;

class BitmapRender : public Component
{
private:
	IDirect3DDevice9* m_device = nullptr;
	IDirect3DTexture9* m_texture = nullptr;
	IDirect3DVertexBuffer9* m_vertexBuffer = nullptr;

	void DrawBitmap(int x, int y, int z, int w, int h);

	void SetupVertices(float x, float y, float z, float width, float height);
public:
	BitmapRender(IDirect3DTexture9* texture);

	void Initialize() override;
	void Release() override;
	void ChangeBitmap(IDirect3DTexture9* texture);
	void Start() override;
	void Update() override;
};

