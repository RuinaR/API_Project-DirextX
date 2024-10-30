#include "pch.h"
#include "FbxTool.h"
#include <fbxsdk/fileio/fbximporter.h>
#include <atlconv.h>
#include <utility>

// FBX ����� DirectX ��ķ� ��ȯ
D3DXMATRIX FbxToD3DXMatrix(const FbxAMatrix& fbxMatrix) {
    D3DXMATRIX d3dMatrix;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            d3dMatrix.m[i][j] = static_cast<FLOAT>(fbxMatrix.Get(i, j));
        }
    }
    return d3dMatrix;
}

// �۷ι� ��ȯ�� ��� ������ ����
void ApplyGlobalTransform(FbxNode* node, std::vector<CUSTOMVERTEX>& vertices) {
    FbxAMatrix globalTransform = node->EvaluateGlobalTransform();
    D3DXMATRIX worldMatrix = FbxToD3DXMatrix(globalTransform);

    for (auto& vertex : vertices) {
        D3DXVECTOR3 position(vertex.x, vertex.y, vertex.z);
        D3DXVec3TransformCoord(&position, &position, &worldMatrix);
        vertex.x = position.x;
        vertex.y = position.y;
        vertex.z = position.z;
    }
}

bool FbxTool::Initialize() {
    m_sdkManager = FbxManager::Create();
    if (!m_sdkManager) {
        std::cerr << "Error: Unable to create FBX Manager!" << std::endl;
        return false;
    }

    FbxIOSettings* ios = FbxIOSettings::Create(m_sdkManager, IOSROOT);
    m_sdkManager->SetIOSettings(ios);
    m_scene = FbxScene::Create(m_sdkManager, "MyScene");

    converter = new FbxGeometryConverter(m_sdkManager);
    converter->Triangulate(m_scene, true);
    return true;
}

bool FbxTool::Load(const char* fileName, std::vector<Model>& outModels) {
    if (!m_sdkManager || !m_scene) {
        std::cerr << "Error: FBX SDK not initialized!" << std::endl;
        return false;
    }

    // FBX ���� ��� ����
    wchar_t path[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, path, MAX_PATH);
    USES_CONVERSION;
    std::string executepath = W2A(path);
    executepath = executepath.substr(0, executepath.find_last_of("\\/"));
    std::string FilePath = executepath + "\\FBX\\" + fileName + ".fbx";

    // FBX ���� �ε�
    FbxImporter* importer = FbxImporter::Create(m_sdkManager, "");
    if (!importer->Initialize(FilePath.c_str(), -1, m_sdkManager->GetIOSettings())) {
        std::cerr << "Error: Unable to open FBX file! : "
            << importer->GetStatus().GetErrorString() << std::endl;
        return false;
    }

    if (!importer->Import(m_scene)) {
        std::cerr << "Error: Failed to import FBX scene!" << std::endl;
        return false;
    }
    importer->Destroy();

    // ��Ʈ ��忡�� �����Ͽ� ��� ��� Ž��
    FbxNode* rootNode = m_scene->GetRootNode();
    if (rootNode) {
        ProcessNode(rootNode, outModels);
    }
    return true;
}


void FbxTool::ProcessNode(FbxNode* node, std::vector<Model>& outModels)
{
    // ��尡 �޽� �Ӽ��� ������ �ִ��� Ȯ��
    if (node->GetNodeAttribute() &&
        node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eMesh) {

        Model model;
        converter->Triangulate(node->GetNodeAttribute(), true);
        ProcessMesh(node->GetMesh(), model);

        // �۷ι� ��ȯ ����
        ApplyGlobalTransform(node, model.vertices);

        // Material �ε� �� �Ҵ�
        int materialCount = node->GetMaterialCount();
        int subMeshCount = model.subMeshes.size();

        for (int i = 0; i < materialCount; ++i) {
            if (i < subMeshCount) {  // �ε��� ���� üũ
                LoadMaterial(node->GetMaterial(i), model.subMeshes[i]);
            }
            else {
                std::cerr << "Warning: Material index " << i << " exceeds subMesh count." << std::endl;
            }
        }

        // ���� ��ȿ�� ��쿡�� �߰�
        if (model.vertexCount > 0 && model.indexCount > 0) {
            outModels.push_back(std::move(model));
        }
        else {
            std::cerr << "Warning: Model has no vertices or indices." << std::endl;
        }
    }

    // ��������� �ڽ� ��� Ž��
    for (int i = 0; i < node->GetChildCount(); ++i) {
        ProcessNode(node->GetChild(i), outModels);
    }
}


void FbxTool::ProcessMesh(FbxMesh* mesh, Model& model) {
    std::vector<CUSTOMVERTEX> vertices;
    std::vector<unsigned int> indices;
    std::unordered_map<int, SubMesh> subMeshMap;

    FbxVector4* controlPoints = mesh->GetControlPoints();
    int polygonCount = mesh->GetPolygonCount();

    // UV ��ǥ ��������
    FbxLayerElementUV* uvElement = (mesh->GetElementUVCount() > 0) ? mesh->GetElementUV(0) : nullptr;
    bool hasUV = (uvElement != nullptr);

    FbxLayerElementMaterial* materialElement = mesh->GetElementMaterial();

    std::map<std::tuple<int, int, int>, unsigned int> vertexMap;  // �ߺ� ���Ÿ� ���� ��
    int nextIndex = 0;

    for (int i = 0; i < polygonCount; ++i) {
        int polygonSize = mesh->GetPolygonSize(i);
        int materialIndex = materialElement ? materialElement->GetIndexArray().GetAt(i) : 0;

        // ����޽� �ʱ�ȭ
        if (subMeshMap.find(materialIndex) == subMeshMap.end()) {
            subMeshMap[materialIndex] = { static_cast<unsigned int>(indices.size()), 0, 0 };
        }

        for (int j = 0; j < polygonSize; ++j) {
            int controlPointIndex = mesh->GetPolygonVertex(i, j);

            // UV �ε��� Ȯ�� (������ ���� ����)
            int uvIndex = -1;
            if (hasUV) {
                if (uvElement->GetMappingMode() == FbxLayerElement::eByPolygonVertex) {
                    uvIndex = uvElement->GetIndexArray().GetAt(i * polygonSize + j);
                }
                else if (uvElement->GetMappingMode() == FbxLayerElement::eByControlPoint) {
                    uvIndex = controlPointIndex; // ���� �ε����� ���
                }
            }

            // UV �ε����� ��ȿ���� Ȯ��
            if (hasUV && (uvIndex != -1) && (uvIndex < uvElement->GetDirectArray().GetCount())) {
                FbxVector2 uv = uvElement->GetDirectArray().GetAt(uvIndex);

                // ���ο� ���� ����

                //DirectX������ Y���� �ݴ� �������� ����
                uv[1] = 1.0f - uv[1];
                FbxVector4 pos = controlPoints[controlPointIndex];
                CUSTOMVERTEX vertex = {
                    static_cast<FLOAT>(pos[0]),
                    static_cast<FLOAT>(pos[1]),
                    static_cast<FLOAT>(pos[2]),
                    D3DCOLOR_XRGB(255, 255, 255), // �⺻ ����
                    static_cast<FLOAT>(uv[0]), // UV ��ǥ ����
                    static_cast<FLOAT>(uv[1])  // UV ��ǥ ����
                };

                // UV �ε����� �����Ͽ� Ű ����
                auto key = std::make_tuple(controlPointIndex, uvIndex, materialIndex);
                if (vertexMap.find(key) == vertexMap.end()) {
                    // ���� �߰�
                    vertices.push_back(vertex);
                    vertexMap[key] = nextIndex++;
                }

                // �ε��� �߰�
                indices.push_back(vertexMap[key]);
                subMeshMap[materialIndex].indexCount++;
            }
            else {
                // UV�� ���� ��� �⺻�� ����
                CUSTOMVERTEX vertex = {
                    static_cast<FLOAT>(controlPoints[controlPointIndex][0]),
                    static_cast<FLOAT>(controlPoints[controlPointIndex][1]),
                    static_cast<FLOAT>(controlPoints[controlPointIndex][2]),
                    D3DCOLOR_XRGB(255, 255, 255), // �⺻ ����
                    0.0f, 0.0f                    // �⺻ UV ��ǥ
                };

                auto key = std::make_tuple(controlPointIndex, -1, materialIndex);
                if (vertexMap.find(key) == vertexMap.end()) {
                    // ���� �߰�
                    vertices.push_back(vertex);
                    vertexMap[key] = nextIndex++;
                }

                // �ε��� �߰�
                indices.push_back(vertexMap[key]);
                subMeshMap[materialIndex].indexCount++;
            }
        }
    }

    // ����޽� ���� �� ���
    for (auto& pair : subMeshMap) {
        pair.second.vertexCount = static_cast<unsigned int>(vertexMap.size());
        model.subMeshes.push_back(pair.second);
    }

    // �𵨿� ���� ���� �� �ε��� ���� ����
    model.vertices = std::move(vertices);
    model.indices = std::move(indices);
    model.vertexCount = static_cast<unsigned int>(model.vertices.size());
    model.indexCount = static_cast<unsigned int>(model.indices.size());
}



void FbxTool::CreateVertexBuffer(Model& model)
{
    MainFrame::GetInstance()->GetDevice()->CreateVertexBuffer(
        model.vertexCount * sizeof(CUSTOMVERTEX),
        0,
        D3DFVF_CUSTOMVERTEX,
        D3DPOOL_MANAGED,
        &model.vertexBuffer,
        nullptr
    );

    void* pVertices;
    model.vertexBuffer->Lock(0, model.vertexCount * sizeof(CUSTOMVERTEX), (void**)&pVertices, 0);
    memcpy(pVertices, model.vertices.data(), model.vertexCount * sizeof(CUSTOMVERTEX));
    model.vertexBuffer->Unlock();
}

void FbxTool::CreateIndexBuffer(Model& model)
{
    MainFrame::GetInstance()->GetDevice()->CreateIndexBuffer(
        model.indexCount * sizeof(unsigned int),
        0,
        D3DFMT_INDEX32,
        D3DPOOL_MANAGED,
        &model.indexBuffer,
        nullptr
    );

    void* pIndices;
    model.indexBuffer->Lock(0, model.indexCount * sizeof(unsigned int), (void**)&pIndices, 0);
    memcpy(pIndices, model.indices.data(), model.indexCount * sizeof(unsigned int));
    model.indexBuffer->Unlock();
}


void FbxTool::LoadMaterial(FbxSurfaceMaterial* material, SubMesh& subMesh) {
    if (!material) {
        std::cerr << "Error: Material is null!" << std::endl;
        return; // ��ȿ���� ���� ���� ó��
    }

    FbxProperty diffuseProp = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
    if (diffuseProp.IsValid()) {
        FbxDouble3 color = diffuseProp.Get<FbxDouble3>();
        subMesh.diffuseColor = D3DCOLOR_COLORVALUE(
            static_cast<float>(color[0]),
            static_cast<float>(color[1]),
            static_cast<float>(color[2]),
            1.0f
        );
    }
    else {
        std::cerr << "Warning: Diffuse property is not valid for material." << std::endl;
    }

    int textureCount = diffuseProp.GetSrcObjectCount<FbxFileTexture>();
    for (int i = 0; i < textureCount; ++i) {
        FbxFileTexture* texture = diffuseProp.GetSrcObject<FbxFileTexture>(i);
        if (texture) {
            std::string texturePath = texture->GetFileName();

            // �̹� �ε�� �ؽ�ó�� �ִ��� Ȯ��
            auto it = textureCache.find(texturePath);
            if (it != textureCache.end()) {
                subMesh.textures.push_back(it->second);
                std::cout << "Using cached texture: " << texturePath << std::endl;
                continue; // ĳ�õ� �ؽ�ó ���
            }

            // �ؽ�ó �ε� �õ�
            IDirect3DTexture9* d3dTexture = nullptr;
            HRESULT result = D3DXCreateTextureFromFileA(
                MainFrame::GetInstance()->GetDevice(), texturePath.c_str(), &d3dTexture
            );

            if (SUCCEEDED(result)) {
                subMesh.textures.push_back(d3dTexture);
                textureCache[texturePath] = d3dTexture; // �ؽ�ó ĳ�ÿ� �߰�
                std::cout << "Loaded texture: " << texturePath << std::endl;
            }
            else {
                std::cerr << "Error: Failed to load texture from " << texturePath << std::endl;
            }
        }
        else {
            std::cerr << "Warning: Texture is null for material." << std::endl;
        }
    }
}


void FbxTool::Cleanup() {
    // �ؽ�ó ĳ�� �޸� ����
    for (auto& pair : textureCache) {
        if (pair.second) {
            pair.second->Release();
            pair.second = nullptr;
        }
    }
    textureCache.clear(); // ĳ�� ����

    if (m_scene) m_scene->Destroy();
    if (m_sdkManager) m_sdkManager->Destroy();
    m_scene = nullptr;
    m_sdkManager = nullptr;
    delete converter;
    converter = nullptr;
}