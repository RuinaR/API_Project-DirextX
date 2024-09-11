#pragma once
#include <fbxsdk.h>
class FbxTool
{
public:
    FbxTool();
    ~FbxTool();

    bool Initialize();
    bool Load(const char* fileName);
    bool Release();

    // Getters to access vertex and index data
    DirectX::XMFLOAT3* GetPositions() const { return m_pos; }
    unsigned int* GetIndices() const { return m_idx; }
    size_t GetVertexCount() const { return m_vertexCount; }
    size_t GetIndexCount() const { return m_indexCount; }

    void ImguiUpdate();
private:
    bool InitializeSdkObjects();
    void DestroySdkObjects();

    void ProcessNode(FbxNode* node);
    void ProcessMesh(FbxMesh* mesh);

    // Variables to store vertex and index data
    DirectX::XMFLOAT3* m_pos = nullptr;
    unsigned int* m_idx = nullptr;
    size_t m_vertexCount = 0;
    size_t m_indexCount = 0;

    // FBX SDK objects
    FbxManager* m_sdkManager = nullptr;
    FbxScene* m_scene = nullptr;
};

