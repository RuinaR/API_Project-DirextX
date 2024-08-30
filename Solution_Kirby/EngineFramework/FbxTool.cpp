#include "pch.h"
#include "FbxTool.h"

FbxTool::FbxTool() {}

FbxTool::~FbxTool() {
    Release();
}

bool FbxTool::Initialize() {
    return InitializeSdkObjects();
}

bool FbxTool::InitializeSdkObjects() {
    m_sdkManager = FbxManager::Create();
    if (!m_sdkManager) {
        std::cerr << "Error: Unable to create FBX Manager!" << std::endl;
        return false;
    }

    FbxIOSettings* ios = FbxIOSettings::Create(m_sdkManager, IOSROOT);
    m_sdkManager->SetIOSettings(ios);

    m_scene = FbxScene::Create(m_sdkManager, "MyScene");
    if (!m_scene) {
        std::cerr << "Error: Unable to create FBX scene!" << std::endl;
        return false;
    }

    return true;
}

void FbxTool::DestroySdkObjects() {
    if (m_sdkManager) {
        m_sdkManager->Destroy();
        m_sdkManager = nullptr;
    }
}

bool FbxTool::Load(const char* fileName) {
    if (!m_sdkManager || !m_scene) {
        std::cerr << "Error: FBX SDK not initialized!" << std::endl;
        return false;
    }

    FbxImporter* importer = FbxImporter::Create(m_sdkManager, "");
    if (!importer->Initialize(fileName, -1, m_sdkManager->GetIOSettings())) {
        std::cerr << "Error: Unable to open FBX file!" << std::endl;
        return false;
    }

    if (!importer->Import(m_scene)) {
        std::cerr << "Error: Failed to import scene!" << std::endl;
        return false;
    }

    importer->Destroy();

    // Process the loaded scene
    FbxNode* rootNode = m_scene->GetRootNode();
    if (rootNode) {
        for (int i = 0; i < rootNode->GetChildCount(); ++i) {
            ProcessNode(rootNode->GetChild(i));
        }
    }

    return true;
}

void FbxTool::ProcessNode(FbxNode* node) {
    FbxMesh* mesh = node->GetMesh();
    if (mesh) {
        ProcessMesh(mesh);
    }

    for (int i = 0; i < node->GetChildCount(); ++i) {
        ProcessNode(node->GetChild(i));
    }
}

void FbxTool::ProcessMesh(FbxMesh* mesh) {
    m_vertexCount = mesh->GetControlPointsCount();
    m_pos = new XMFLOAT3[m_vertexCount];

    // Copy vertex positions
    for (int i = 0; i < m_vertexCount; ++i) {
        FbxVector4 position = mesh->GetControlPointAt(i);
        m_pos[i] = XMFLOAT3(static_cast<float>(position[0]),
            static_cast<float>(position[1]),
            static_cast<float>(position[2]));
    }

    m_indexCount = mesh->GetPolygonVertexCount();
    m_idx = new unsigned int[m_indexCount];

    // Copy indices
    const int* indices = mesh->GetPolygonVertices();
    for (int i = 0; i < m_indexCount; ++i) {
        m_idx[i] = static_cast<unsigned int>(indices[i]);
    }
}

bool FbxTool::Release() {
    if (m_pos) {
        delete[] m_pos;
        m_pos = nullptr;
    }

    if (m_idx) {
        delete[] m_idx;
        m_idx = nullptr;
    }

    m_vertexCount = 0;
    m_indexCount = 0;

    DestroySdkObjects();

    return true;
}