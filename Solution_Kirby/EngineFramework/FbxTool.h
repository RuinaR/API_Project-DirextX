#pragma once
#include <fbxsdk.h>
struct CUSTOMVERTEX;

struct sMaterial
{
    std::vector<std::string> texturePaths;
    std::vector<IDirect3DTexture9*> textures;
    //std::vector<std::pair<float, float>> uvs; 
};

class FbxTool
{
public:
    FbxTool();
    ~FbxTool();

    bool Initialize();
    bool Load(const char* fileName);
    bool Release();

    CUSTOMVERTEX* GetPositions() const { return m_pos; }
    unsigned int* GetIndices() const { return m_idx; }
    size_t GetVertexCount() const { return m_vertexCount; }
    size_t GetIndexCount() const { return m_indexCount; }

    // UV 좌표와 텍스처를 가져오는 함수
    //std::vector<std::pair<float, float>> GetUVs() const { return m_uvs; }
    //std::vector<std::string> GetTexturePaths() const { return m_texturePaths; }
    //std::vector<IDirect3DTexture9*> GetTextures() const { return m_textures; }
    std::vector<sMaterial> GetMaterial() const { return m_sMats; }

    void ImguiUpdate();
private:
    bool InitializeSdkObjects();
    void DestroySdkObjects();

    void ProcessNode(FbxNode* node, std::vector<CUSTOMVERTEX>& vertices, std::vector<unsigned int>& indices, FbxMatrix parentTransform);
    void ProcessMesh(FbxMesh* mesh, std::vector<CUSTOMVERTEX>& vertices, std::vector<unsigned int>& indices, const FbxMatrix& transform);
    void ProcessMaterial(FbxSurfaceMaterial* material);

    // Variables to store vertex and index data
    CUSTOMVERTEX* m_pos = nullptr;
    unsigned int* m_idx = nullptr;
    size_t m_vertexCount = 0;
    size_t m_indexCount = 0;

    // FBX SDK objects
    FbxManager* m_sdkManager = nullptr;
    FbxScene* m_scene = nullptr;

    std::vector<sMaterial> m_sMats;
};

