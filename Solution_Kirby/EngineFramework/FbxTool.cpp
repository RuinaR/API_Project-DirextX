#include "pch.h"
#include "FbxTool.h"
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
	GetModuleFileName(NULL, path, MAX_PATH);
	USES_CONVERSION;
	std::string executepath = W2A(path);
	executepath = executepath.substr(0, executepath.find_last_of("\\/"));
	//파일 경로 설정
	string FilePath = executepath + "\\FBX\\" + fileName + ".fbx";

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

    importer->Destroy();

    // Process the loaded scene
    FbxNode* rootNode = m_scene->GetRootNode();
    if (rootNode) 
    {
        for (int i = 0; i < rootNode->GetChildCount(); ++i)
        {
            ProcessNode(rootNode->GetChild(i));
        }
    }

    return true;
}

void FbxTool::ProcessNode(FbxNode* node) 
{
    FbxMesh* mesh = node->GetMesh();
    if (mesh) 
    {
        ProcessMesh(mesh);
    }

    for (int i = 0; i < node->GetChildCount(); ++i) 
    {
        ProcessNode(node->GetChild(i));
    }
}

void FbxTool::ProcessMesh(FbxMesh* mesh) 
{
    m_vertexCount = mesh->GetControlPointsCount();
    m_pos = new XMFLOAT3[m_vertexCount];

    // Copy vertex positions
    for (int i = 0; i < m_vertexCount; ++i) 
    {
        FbxVector4 position = mesh->GetControlPointAt(i);
        m_pos[i] = XMFLOAT3(static_cast<float>(position[0]),
            static_cast<float>(position[1]),
            static_cast<float>(position[2]));
    }

    m_indexCount = mesh->GetPolygonVertexCount();
    m_idx = new unsigned int[m_indexCount];

    // Copy indices
    const int* indices = mesh->GetPolygonVertices();
    for (int i = 0; i < m_indexCount; ++i) 
    {
        m_idx[i] = static_cast<unsigned int>(indices[i]);
    }
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