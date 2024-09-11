#pragma once


class FBXRender :public Component
{
private:
	FbxTool m_tool;
	std::string m_name;

	IDirect3DVertexBuffer9* m_pVertexBuffer = nullptr;
	IDirect3DIndexBuffer9* m_pIndexBuffer = nullptr;
	int m_vertexCount = 0;
	int m_indexCount = 0;

public:
	FBXRender(std::string name);
	void Initialize() override;
	void Release() override;
	void Start() override;
	void Update() override;
	void Render();
};

