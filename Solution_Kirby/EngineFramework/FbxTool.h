#pragma once
#include <fbxsdk.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <map>
#include <string>
#include <vector>
#include <unordered_map> 

struct CUSTOMVERTEX;
struct Material;
struct SubMesh;
struct Model;

class FbxTool
{
public:
    FbxTool() : m_sdkManager(nullptr), m_scene(nullptr) {}
    ~FbxTool() { Cleanup(); }

    bool Initialize();
    bool Load(const char* fileName, std::vector<Model>& outModels);

    std::vector<std::string> m_vertexInfo;
    std::vector<std::string> m_indexInfo;

    void CreateIndexBuffer(Model& model);
    void CreateVertexBuffer(Model& model);
private:
    FbxManager* m_sdkManager;
    FbxScene* m_scene;
    FbxGeometryConverter* converter;

    void ProcessNode(FbxNode* node, std::vector<Model>& outModels);
    void ProcessMesh(FbxMesh* mesh, Model& model);
    void LoadMaterial(FbxSurfaceMaterial* material, SubMesh& subMesh);
    void Cleanup();

    std::unordered_map<std::string, IDirect3DTexture9*> textureCache; // 텍스처 캐시 추가
};

