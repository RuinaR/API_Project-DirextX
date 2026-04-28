#include "pch.h"
#include "FbxTool.h"
#include <fbxsdk/fileio/fbximporter.h>
#include <algorithm>
#include <utility>

namespace
{
bool FileExists(const std::string& path)
{
	return GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES;
}

std::string GetDirectoryName(const std::string& path)
{
	size_t pos = path.find_last_of("\\/");
	if (pos == std::string::npos)
	{
		return std::string();
	}
	return path.substr(0, pos);
}

std::string GetFileNameOnly(const std::string& path)
{
	size_t pos = path.find_last_of("\\/");
	if (pos == std::string::npos)
	{
		return path;
	}
	return path.substr(pos + 1);
}

std::string JoinPath(const std::string& lhs, const std::string& rhs)
{
	if (lhs.empty())
	{
		return rhs;
	}
	if (lhs.back() == '\\' || lhs.back() == '/')
	{
		return lhs + rhs;
	}
	return lhs + "\\" + rhs;
}

std::string GetExeDirectory()
{
	wchar_t path[MAX_PATH] = { 0 };
	GetModuleFileNameW(NULL, path, MAX_PATH);
	return GetDirectoryName(WideToAnsi(path));
}

std::string ResolveFbxPath(const char* fileName)
{
	std::string name = fileName ? fileName : "";
	if (name.empty())
	{
		return name;
	}

	if (name.find(".fbx") == std::string::npos && name.find(".FBX") == std::string::npos)
	{
		name += ".fbx";
	}

	if (FileExists(name))
	{
		return name;
	}

	std::string exeDir = GetExeDirectory();
	std::vector<std::string> candidates;
	candidates.push_back(JoinPath(JoinPath(exeDir, "FBX"), name));
	candidates.push_back(JoinPath(JoinPath(exeDir, "..\\..\\FBX"), name));
	candidates.push_back(JoinPath(JoinPath(exeDir, "..\\..\\..\\FBX"), name));

	for (const std::string& candidate : candidates)
	{
		if (FileExists(candidate))
		{
			return candidate;
		}
	}

	return candidates.front();
}

std::string ResolveTexturePath(const std::string& texturePath, const std::string& fbxDirectory)
{
	if (texturePath.empty() || FileExists(texturePath))
	{
		return texturePath;
	}

	const std::string textureName = GetFileNameOnly(texturePath);
	std::string exeDir = GetExeDirectory();
	std::vector<std::string> candidates;
	candidates.push_back(JoinPath(fbxDirectory, textureName));
	candidates.push_back(JoinPath(JoinPath(fbxDirectory, "Texture"), textureName));
	candidates.push_back(JoinPath(JoinPath(fbxDirectory, "Textures"), textureName));
	candidates.push_back(JoinPath(JoinPath(exeDir, "FBX"), textureName));
	candidates.push_back(JoinPath(JoinPath(exeDir, "FBX\\Texture"), textureName));
	candidates.push_back(JoinPath(JoinPath(exeDir, "FBX\\Textures"), textureName));
	candidates.push_back(JoinPath(JoinPath(exeDir, "..\\..\\FBX"), textureName));
	candidates.push_back(JoinPath(JoinPath(exeDir, "..\\..\\FBX\\Texture"), textureName));
	candidates.push_back(JoinPath(JoinPath(exeDir, "..\\..\\FBX\\Textures"), textureName));

	for (const std::string& candidate : candidates)
	{
		if (FileExists(candidate))
		{
			return candidate;
		}
	}

	return texturePath;
}

int GetUVIndex(FbxMesh* mesh, FbxLayerElementUV* uvElement, int polygonIndex, int polygonVertexIndex, int controlPointIndex)
{
	if (!uvElement)
	{
		return -1;
	}

	int directIndex = -1;
	if (uvElement->GetMappingMode() == FbxLayerElement::eByPolygonVertex)
	{
		if (uvElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
		{
			return mesh->GetTextureUVIndex(polygonIndex, polygonVertexIndex);
		}
		directIndex = mesh->GetPolygonVertexIndex(polygonIndex) + polygonVertexIndex;
	}
	else if (uvElement->GetMappingMode() == FbxLayerElement::eByControlPoint)
	{
		directIndex = controlPointIndex;
	}

	if (directIndex < 0)
	{
		return -1;
	}

	if (uvElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
	{
		if (directIndex >= uvElement->GetIndexArray().GetCount())
		{
			return -1;
		}
		return uvElement->GetIndexArray().GetAt(directIndex);
	}

	return directIndex;
}

int GetMaterialIndex(FbxLayerElementMaterial* materialElement, int polygonIndex)
{
	if (!materialElement)
	{
		return 0;
	}

	if (materialElement->GetMappingMode() == FbxLayerElement::eAllSame)
	{
		if (materialElement->GetIndexArray().GetCount() > 0)
		{
			return materialElement->GetIndexArray().GetAt(0);
		}
		return 0;
	}

	if (materialElement->GetMappingMode() == FbxLayerElement::eByPolygon)
	{
		if (polygonIndex < materialElement->GetIndexArray().GetCount())
		{
			return materialElement->GetIndexArray().GetAt(polygonIndex);
		}
	}

	return 0;
}
}

// FBX 행렬을 DirectX 행렬로 변환
D3DXMATRIX FbxToD3DXMatrix(const FbxAMatrix& fbxMatrix) {
    D3DXMATRIX d3dMatrix;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            d3dMatrix.m[i][j] = static_cast<FLOAT>(fbxMatrix.Get(i, j));
        }
    }
    return d3dMatrix;
}

// 글로벌 변환을 모든 정점에 적용
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
    return true;
}

bool FbxTool::Load(const char* fileName, std::vector<Model>* outModels) {
    if (!outModels) {
        return false;
    }

    if (!m_sdkManager || !m_scene) {
        std::cerr << "Error: FBX SDK not initialized!" << std::endl;
        return false;
    }

    std::string FilePath = ResolveFbxPath(fileName);
    m_currentFbxDirectory = GetDirectoryName(FilePath);

    // FBX 파일 로드
    FbxImporter* importer = FbxImporter::Create(m_sdkManager, "");
    if (!importer->Initialize(FilePath.c_str(), -1, m_sdkManager->GetIOSettings())) {
        std::cerr << "Error: Unable to open FBX file! : "
            << importer->GetStatus().GetErrorString() << std::endl;
        importer->Destroy();
        return false;
    }

    if (!importer->Import(m_scene)) {
        std::cerr << "Error: Failed to import FBX scene!" << std::endl;
        importer->Destroy();
        return false;
    }
    importer->Destroy();
    converter->Triangulate(m_scene, true);

    // 루트 노드에서 시작하여 모든 노드 탐색
    FbxNode* rootNode = m_scene->GetRootNode();
    if (rootNode) {
        ProcessNode(rootNode, outModels);
    }
    return true;
}


void FbxTool::ProcessNode(FbxNode* node, std::vector<Model>* outModels)
{
    if (!node || !outModels)
    {
        return;
    }

    // 노드가 메쉬 속성을 가지고 있는지 확인
    if (node->GetNodeAttribute() &&
        node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eMesh) {

        Model model;
        converter->Triangulate(node->GetNodeAttribute(), true);
        ProcessMesh(node->GetMesh(), &model);

        // 글로벌 변환 적용
        ApplyGlobalTransform(node, model.vertices);

        // Material 로드 및 할당
        int materialCount = node->GetMaterialCount();

        for (auto& subMesh : model.subMeshes) {
            if (subMesh.materialIndex < static_cast<unsigned int>(materialCount)) {
                LoadMaterial(node->GetMaterial(subMesh.materialIndex), &subMesh);
            }
            else {
                std::cerr << "Warning: Material index " << subMesh.materialIndex << " exceeds material count." << std::endl;
            }
        }

        // 모델이 유효할 경우에만 추가
        if (model.vertexCount > 0 && model.indexCount > 0) {
            outModels->push_back(std::move(model));
        }
        else {
            std::cerr << "Warning: Model has no vertices or indices." << std::endl;
        }
    }

    // 재귀적으로 자식 노드 탐색
    for (int i = 0; i < node->GetChildCount(); ++i) {
        ProcessNode(node->GetChild(i), outModels);
    }
}


void FbxTool::ProcessMesh(FbxMesh* mesh, Model* model) {
    if (!mesh || !model)
    {
        return;
    }

    std::vector<CUSTOMVERTEX> vertices;
    std::vector<unsigned int> indices;
    std::map<int, std::vector<unsigned int>> materialIndices;

    FbxVector4* controlPoints = mesh->GetControlPoints();
    int polygonCount = mesh->GetPolygonCount();

    // UV 좌표 가져오기
    FbxLayerElementUV* uvElement = (mesh->GetElementUVCount() > 0) ? mesh->GetElementUV(0) : nullptr;
    bool hasUV = (uvElement != nullptr);

    FbxLayerElementMaterial* materialElement = mesh->GetElementMaterial();

    std::map<std::tuple<int, int, int>, unsigned int> vertexMap;  // 중복 제거를 위한 맵
    int nextIndex = 0;

    for (int i = 0; i < polygonCount; ++i) {
        int polygonSize = mesh->GetPolygonSize(i);
        int materialIndex = GetMaterialIndex(materialElement, i);

        for (int j = 0; j < polygonSize; ++j) {
            int controlPointIndex = mesh->GetPolygonVertex(i, j);

            // UV 인덱스 확인 (폴리곤 정점 기준)
            int uvIndex = -1;
            if (hasUV) {
                uvIndex = GetUVIndex(mesh, uvElement, i, j, controlPointIndex);
            }

            // UV 인덱스가 유효한지 확인
            if (hasUV && (uvIndex != -1) && (uvIndex < uvElement->GetDirectArray().GetCount())) {
                FbxVector2 uv = uvElement->GetDirectArray().GetAt(uvIndex);

                // 새로운 정점 생성

                //DirectX에서는 Y축이 반대 방향으로 매핑
                uv[1] = 1.0f - uv[1];
                FbxVector4 pos = controlPoints[controlPointIndex];
                CUSTOMVERTEX vertex = {
                    static_cast<FLOAT>(pos[0]),
                    static_cast<FLOAT>(pos[1]),
                    static_cast<FLOAT>(pos[2]),
                    D3DCOLOR_XRGB(255, 255, 255), // 기본 색상
                    static_cast<FLOAT>(uv[0]), // UV 좌표 설정
                    static_cast<FLOAT>(uv[1])  // UV 좌표 설정
                };

                // UV 인덱스를 포함하여 키 생성
                auto key = std::make_tuple(controlPointIndex, uvIndex, materialIndex);
                if (vertexMap.find(key) == vertexMap.end()) {
                    // 정점 추가
                    vertices.push_back(vertex);
                    vertexMap[key] = nextIndex++;
                }

                // 인덱스 추가
                materialIndices[materialIndex].push_back(vertexMap[key]);
            }
            else {
                // UV가 없는 경우 기본값 설정
                CUSTOMVERTEX vertex = {
                    static_cast<FLOAT>(controlPoints[controlPointIndex][0]),
                    static_cast<FLOAT>(controlPoints[controlPointIndex][1]),
                    static_cast<FLOAT>(controlPoints[controlPointIndex][2]),
                    D3DCOLOR_XRGB(255, 255, 255), // 기본 색상
                    0.0f, 0.0f                    // 기본 UV 좌표
                };

                auto key = std::make_tuple(controlPointIndex, -1, materialIndex);
                if (vertexMap.find(key) == vertexMap.end()) {
                    // 정점 추가
                    vertices.push_back(vertex);
                    vertexMap[key] = nextIndex++;
                }

                // 인덱스 추가
                materialIndices[materialIndex].push_back(vertexMap[key]);
            }
        }
    }

    for (auto& pair : materialIndices) {
        SubMesh subMesh = {};
        subMesh.startIndex = static_cast<unsigned int>(indices.size());
        subMesh.indexCount = static_cast<unsigned int>(pair.second.size());
        subMesh.vertexStart = 0;
        subMesh.vertexCount = static_cast<unsigned int>(vertices.size());
        subMesh.materialIndex = static_cast<unsigned int>(pair.first);
        subMesh.diffuseColor = D3DCOLOR_XRGB(255, 255, 255);
        subMesh.uvScaleU = 1.0f;
        subMesh.uvScaleV = 1.0f;
        subMesh.uvOffsetU = 0.0f;
        subMesh.uvOffsetV = 0.0f;

        indices.insert(indices.end(), pair.second.begin(), pair.second.end());
        model->subMeshes.push_back(subMesh);
    }

    // 모델에 최종 정점 및 인덱스 정보 저장
    model->vertices = std::move(vertices);
    model->indices = std::move(indices);
    model->vertexCount = static_cast<unsigned int>(model->vertices.size());
    model->indexCount = static_cast<unsigned int>(model->indices.size());
}



void FbxTool::CreateVertexBuffer(Model* model)
{
    if (!model)
    {
        return;
    }

    MainFrame::GetInstance()->GetDevice()->CreateVertexBuffer(
        model->vertexCount * sizeof(CUSTOMVERTEX),
        0,
        D3DFVF_CUSTOMVERTEX,
        D3DPOOL_MANAGED,
        &model->vertexBuffer,
        nullptr
    );

    void* pVertices;
    model->vertexBuffer->Lock(0, model->vertexCount * sizeof(CUSTOMVERTEX), (void**)&pVertices, 0);
    memcpy(pVertices, model->vertices.data(), model->vertexCount * sizeof(CUSTOMVERTEX));
    model->vertexBuffer->Unlock();
}

void FbxTool::CreateIndexBuffer(Model* model)
{
    if (!model)
    {
        return;
    }

    MainFrame::GetInstance()->GetDevice()->CreateIndexBuffer(
        model->indexCount * sizeof(unsigned int),
        0,
        D3DFMT_INDEX32,
        D3DPOOL_MANAGED,
        &model->indexBuffer,
        nullptr
    );

    void* pIndices;
    model->indexBuffer->Lock(0, model->indexCount * sizeof(unsigned int), (void**)&pIndices, 0);
    memcpy(pIndices, model->indices.data(), model->indexCount * sizeof(unsigned int));
    model->indexBuffer->Unlock();
}


void FbxTool::LoadMaterial(FbxSurfaceMaterial* material, SubMesh* subMesh) {
    if (!subMesh) {
        return;
    }

    if (!material) {
        std::cerr << "Error: Material is null!" << std::endl;
        return; // 유효하지 않은 재질 처리
    }

    FbxProperty diffuseProp = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
    if (diffuseProp.IsValid()) {
        FbxDouble3 color = diffuseProp.Get<FbxDouble3>();
        subMesh->diffuseColor = D3DCOLOR_COLORVALUE(
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
            std::string texturePath = ResolveTexturePath(texture->GetFileName(), m_currentFbxDirectory);

            // 이미 로드된 텍스처가 있는지 확인
            auto it = textureCache.find(texturePath);
            if (it != textureCache.end()) {
                subMesh->textures.push_back(it->second);
                std::cout << "Using cached texture: " << texturePath << std::endl;
                continue; // 캐시된 텍스처 사용
            }

            // 텍스처 로드 시도
            IDirect3DTexture9* d3dTexture = nullptr;
            d3dTexture = TextureManager::GetInstance()->GetTexture(texturePath.c_str());

            if (d3dTexture) {
                subMesh->textures.push_back(d3dTexture);
                textureCache[texturePath] = d3dTexture; // 텍스처 캐시에 추가
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
    // 텍스처 캐시 메모리 해제
    textureCache.clear(); // 캐시 비우기

    if (m_scene) m_scene->Destroy();
    if (m_sdkManager) m_sdkManager->Destroy();
    m_scene = nullptr;
    m_sdkManager = nullptr;
    delete converter;
    converter = nullptr;
}
