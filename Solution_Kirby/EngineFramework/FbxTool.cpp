#include "pch.h"
#include "FbxTool.h"
#include <fbxsdk/fileio/fbximporter.h>
#include <atlconv.h>

FbxTool::FbxTool() {}

FbxTool::~FbxTool() {}

bool FbxTool::Initialize()
{
    return InitializeSdkObjects();
}

void FbxTool::ImguiUpdate()
{
    if (ImGui::Begin("FBX Vertex/Index Data"))
    {
        // Display vertex count
        ImGui::Text("Vertex Count: %llu", (unsigned long long)m_vertexCount);

        // Display each vertex position
        if (ImGui::TreeNode("Vertices"))
        {
            for (size_t i = 0; i < m_vertexCount; ++i)
            {
                ImGui::Text("Vertex %llu: (%.3f, %.3f, %.3f)",
                    (unsigned long long)i,
                    m_pos[i].x, m_pos[i].y, m_pos[i].z);
            }
            ImGui::TreePop();
        }

        // Display index count
        ImGui::Text("Index Count: %llu", (unsigned long long)m_indexCount);

        // Display each index
        if (ImGui::TreeNode("Indices"))
        {
            for (size_t i = 0; i < m_indexCount; ++i)
            {
                ImGui::Text("Index %llu: %u", (unsigned long long)i, m_idx[i]);
            }
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

bool FbxTool::InitializeSdkObjects()
{
    m_sdkManager = FbxManager::Create();
    if (!m_sdkManager)
    {
        std::cerr << "Error: Unable to create FBX Manager!" << std::endl;
        return false;
    }

    FbxIOSettings* ios = FbxIOSettings::Create(m_sdkManager, IOSROOT);
    ios->SetBoolProp(IMP_FBX_MATERIAL, true);
    ios->SetBoolProp(IMP_FBX_TEXTURE, true);
    ios->SetBoolProp(IMP_FBX_LINK, true);
    ios->SetBoolProp(IMP_FBX_SHAPE, true);
    ios->SetBoolProp(IMP_FBX_GOBO, true);
    ios->SetBoolProp(IMP_FBX_ANIMATION, true);
    ios->SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);
    m_sdkManager->SetIOSettings(ios);

    m_scene = FbxScene::Create(m_sdkManager, "MyScene");
    if (!m_scene)
    {
        std::cerr << "Error: Unable to create FBX scene!" << std::endl;
        return false;
    }

    return true;
}

void FbxTool::DestroySdkObjects()
{
    if (m_sdkManager)
    {
        m_sdkManager->Destroy();
        m_sdkManager = nullptr;
    }
}

bool FbxTool::Load(const char* fileName)
{
    if (!m_sdkManager || !m_scene)
    {
        std::cerr << "Error: FBX SDK not initialized!" << std::endl;
        return false;
    }

    wchar_t path[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, path, MAX_PATH);
    USES_CONVERSION;
    std::string executepath = W2A(path);
    executepath = executepath.substr(0, executepath.find_last_of("\\/"));
    // ���� ��� ����
    std::string FilePath = executepath + "\\FBX\\" + fileName + ".fbx";

    FbxImporter* importer = FbxImporter::Create(m_sdkManager, "");
    if (!importer->Initialize(FilePath.c_str(), -1, m_sdkManager->GetIOSettings()))
    {
        std::cerr << "Error: Unable to open FBX file! : " << importer->GetStatus().GetErrorString() << std::endl;
        return false;
    }

    if (!importer->Import(m_scene))
    {
        std::cerr << "Error: Failed to import scene!" << std::endl;
        return false;
    }

    // ��Ʈ ��带 ó��
    FbxNode* rootNode = m_scene->GetRootNode();
    if (rootNode) {
        std::vector<CUSTOMVERTEX> vertices;
        std::vector<unsigned int> indices;

        // �ʱ� �θ� ��ȯ ����� ���� ��ķ� ����
        FbxMatrix identityMatrix;

        ProcessNode(rootNode, vertices, indices, identityMatrix);

        // ���յ� ���ؽ� �� �ε����� �ϳ��� ���۷� ��ȯ
        m_vertexCount = vertices.size();
        m_pos = new CUSTOMVERTEX[m_vertexCount];
        for (size_t i = 0; i < m_vertexCount; i++) {
            m_pos[i] = vertices[i];
        }

        m_indexCount = indices.size();
        m_idx = new unsigned int[m_indexCount];
        for (size_t i = 0; i < m_indexCount; i++) {
            m_idx[i] = indices[i];
        }
    }

    importer->Destroy();
    return true;
}

void FbxTool::ProcessNode(FbxNode* node, std::vector<CUSTOMVERTEX>& vertices, std::vector<unsigned int>& indices, FbxMatrix parentTransform)
{
    // ���� ����� �۷ι� ��ȯ ��� ���
    FbxMatrix nodeTransform = node->EvaluateGlobalTransform();
    FbxMatrix globalTransform = parentTransform * nodeTransform;

    FbxNodeAttribute* attribute = node->GetNodeAttribute();
    if (attribute && attribute->GetAttributeType() == FbxNodeAttribute::eMesh) 
    {
        ProcessMesh(node->GetMesh(), vertices, indices, globalTransform);

        // ��忡 ����(Material)�� ������ ó��
        for (int i = 0; i < node->GetMaterialCount(); ++i)
        {
            ProcessMaterial(node->GetMaterial(i));  // �ؽ�ó ó��
        }
    }

    // �ڽ� ���鵵 ó��
    for (int i = 0; i < node->GetChildCount(); i++) 
    {
        ProcessNode(node->GetChild(i), vertices, indices, globalTransform);
    }
}

void FbxTool::ProcessMesh(FbxMesh* mesh, std::vector<CUSTOMVERTEX>& vertices, std::vector<unsigned int>& indices, const FbxMatrix& transform)
{
    if (!mesh) return;

    // FBX �޽��� �ﰢ������ ��ȯ
    FbxGeometryConverter converter(m_sdkManager);
    if (!mesh->IsTriangleMesh()) {
        mesh = static_cast<FbxMesh*>(converter.Triangulate(mesh, true));
    }

    // ���� ���ؽ� ���� ���� (�ε��� ������ ����)
    size_t vertexOffset = vertices.size();

    // ���ؽ� ������ �߰�
    int vertexCount = mesh->GetControlPointsCount();
    for (int i = 0; i < vertexCount; i++) {
        FbxVector4 pos = mesh->GetControlPointAt(i);
        FbxVector4 transformedPos = transform.MultNormalize(pos);
        CUSTOMVERTEX vertex = {
            static_cast<FLOAT>(transformedPos[0]),
            static_cast<FLOAT>(transformedPos[1]),
            static_cast<FLOAT>(transformedPos[2]),
            0xFFFFFFFF,    // �⺻ ��� (���� ó��)
            0.0f, 0.0f     // �⺻ �ؽ�ó ��ǥ
        };
        vertices.push_back(vertex);
    }

    // �ε��� ������ �߰�
    int polygonCount = mesh->GetPolygonCount();
    for (int i = 0; i < polygonCount; i++) {
        for (int j = 0; j < 3; j++) {
            indices.push_back(vertexOffset + static_cast<unsigned int>(mesh->GetPolygonVertex(i, j)));
        }
    }
}

void FbxTool::ProcessMaterial(FbxSurfaceMaterial* material)
{
    if (!material) return;

    /*732 ���� Ȯ��*/
    // ��ǻ�� �ؽ�ó �Ӽ��� ó��
    // EngineFramework ���� ���� -> KirbyGameDll lib���� ���� ���� ����
    // LNK2001 ����
    FbxProperty prop = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
    int textureCount = prop.GetSrcObjectCount<FbxFileTexture>();
    for (int i = 0; i < textureCount; ++i)
    {
        FbxFileTexture* texture = FbxCast<FbxFileTexture>(prop.GetSrcObject<FbxFileTexture>(i));
        if (texture)
        {
            const char* texturePath = texture->GetFileName();
            const char* texturePath2 = texture->GetRelativeFileName();

            m_texturePaths.push_back(texturePath); // �ؽ�ó ��� ����
        }
    }
    
    /*732 ���� Ȯ�� end*/
    
}

bool FbxTool::Release()
{
    if (m_pos)
    {
        delete[] m_pos;
        m_pos = nullptr;
    }

    if (m_idx)
    {
        delete[] m_idx;
        m_idx = nullptr;
    }

    m_vertexCount = 0;
    m_indexCount = 0;

    DestroySdkObjects();

    return true;
}
