#include "pch.h"
#include "FBXRender.h"
#include "SceneJsonUtility.h"

// 생성자: FBX 파일 이름을 받아 초기화
FBXRender::FBXRender(std::string name) : m_fbxFileName(std::move(name)) {}

// 초기화 함수
void FBXRender::Initialize() {
    auto device = MainFrame::GetInstance()->GetDevice();  // Direct3D 디바이스 가져오기
    m_loaded = false;

    if (!m_tool.Initialize()) {
        m_logSystem.AddLog("Error: Failed to initialize FbxTool!");
        return;
    }

    if (!m_tool.Load(m_fbxFileName.c_str(), &m_models)) {
        m_logSystem.AddLog("Error: Failed to load FBX file");
        return;
    }

    m_logSystem.AddLog("FBX file loaded successfully");
    m_loaded = true;

    for (auto& model : m_models)
    {
        m_tool.CreateVertexBuffer(&model);
        m_tool.CreateIndexBuffer(&model);
    }

    RenderManager::GetInstance()->Register(this);  // 렌더 매니저에 등록
}

// 자원 해제 함수
void FBXRender::Release() {
    m_logSystem.AddLog("FBXRender resources released.");
    RenderManager::GetInstance()->Unregister(this);  // 렌더 매니저에서 해제

    for (auto& model : m_models) {
        if (model.vertexBuffer) {
            model.vertexBuffer->Release();
            model.vertexBuffer = nullptr;
        }
        if (model.indexBuffer) {
            model.indexBuffer->Release();
            model.indexBuffer = nullptr;
        }
    }
}

void FBXRender::Start() {}

// 로그 표시
void FBXRender::Update() {
    //m_logSystem.ShowLogWindow();
}

void FBXRender::Render() {
    if (m_gameObj == nullptr || !m_gameObj->GetActive() || m_gameObj->GetDestroy() || !m_loaded) {
        return;
    }

    auto device = MainFrame::GetInstance()->GetDevice();
    if (!device) {
        m_logSystem.AddLog("Error: Direct3D device is null!");
        return;
    }

    D3DXMATRIX matWorld;
    SetWorldTransform(&matWorld);

    for (auto& model : m_models) {
        if (!model.vertexBuffer || !model.indexBuffer) {
            m_logSystem.AddLog("Error: Vertex or index buffer is null for model!");
            continue;
        }

        // 정점 스트림과 인덱스 버퍼 설정
        device->SetStreamSource(0, model.vertexBuffer, 0, sizeof(CUSTOMVERTEX));
        device->SetIndices(model.indexBuffer);

        m_logSystem.AddLog("Index count: " + std::to_string(model.indices.size()));
        m_logSystem.AddLog("Vertex count: " + std::to_string(model.vertices.size()));

        for (const auto& subMesh : model.subMeshes) {
            m_logSystem.AddLog("SubMesh startIndex: " + std::to_string(subMesh.startIndex));
            m_logSystem.AddLog("SubMesh indexCount: " + std::to_string(subMesh.indexCount));

            // 유효성 검사
            if (subMesh.startIndex >= model.indices.size()) {
                m_logSystem.AddLog("Error: SubMesh startIndex exceeds model indices size!");
                continue;
            }
            if (subMesh.indexCount == 0) {
                m_logSystem.AddLog("Error: SubMesh indexCount is zero!");
                continue;
            }
            if (subMesh.indexCount % 3 != 0) {
                m_logSystem.AddLog("Error: SubMesh indexCount is not a multiple of 3!");
                continue;
            }
            if (subMesh.startIndex + subMesh.indexCount > model.indices.size()) {
                m_logSystem.AddLog("Error: SubMesh index range exceeds model indices size!");
                continue;
            }

            for (DWORD i = 0; i < 8; ++i) {
                device->SetTexture(i, nullptr);
            }

            device->SetRenderState(D3DRS_TEXTUREFACTOR, subMesh.diffuseColor);
            if (!subMesh.textures.empty() && subMesh.textures[0]) {
                device->SetTexture(0, subMesh.textures[0]);
                device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
                device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
                m_logSystem.AddLog("Setting texture for slot 0");
            }
            else {
                device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
                device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
            }

            // 서브메쉬 렌더링
            HRESULT hr = device->DrawIndexedPrimitive(
                D3DPT_TRIANGLELIST,
                0,
                0,
                model.vertexCount,
                subMesh.startIndex,
                subMesh.indexCount / 3
            );

            // DrawIndexedPrimitive 호출 결과 확인
            if (FAILED(hr)) {
                m_logSystem.AddLog("Error: DrawIndexedPrimitive failed!");
            }
        }
    }
}



// 월드 변환 설정
void FBXRender::SetWorldTransform(D3DXMATRIX* matWorld) {
    if (!matWorld)
    {
        return;
    }

    D3DXVECTOR3 angle = { m_gameObj->GetAngleX(), m_gameObj->GetAngleY(), m_gameObj->GetAngleZ() };
    D3DXVECTOR3 scale = m_gameObj->Size3D();
    D3DXVECTOR3 position = m_gameObj->Position();

    D3DXMATRIX matScale, matRotation, matTranslation;
    D3DXMatrixScaling(&matScale, scale.x, scale.y, scale.z);
    D3DXMatrixRotationYawPitchRoll(&matRotation, D3DXToRadian(angle.y), D3DXToRadian(angle.x), D3DXToRadian(angle.z));
    D3DXMatrixTranslation(&matTranslation, position.x, position.y, position.z);

    *matWorld = matScale * matRotation * matTranslation;
    MainFrame::GetInstance()->GetDevice()->SetTransform(D3DTS_WORLD, matWorld);
}

const char* FBXRender::GetInspectorName() const
{
    return "FBXRender";
}

void FBXRender::DrawInspector()
{
    int subMeshCount = 0;
    int materialCount = 0;
    int vertexCount = 0;
    int indexCount = 0;

    for (const auto& model : m_models)
    {
        subMeshCount += static_cast<int>(model.subMeshes.size());
        vertexCount += model.vertexCount;
        indexCount += model.indexCount;
        for (const auto& subMesh : model.subMeshes)
        {
            materialCount++;
        }
    }

    ImGui::Text("FBX File: %s", m_fbxFileName.c_str());
    ImGui::Text("Loaded: %s", m_loaded ? "true" : "false");
    ImGui::Text("Model Count: %d", static_cast<int>(m_models.size()));
    ImGui::Text("Mesh/SubMesh Count: %d", subMeshCount);
    ImGui::Text("Material Slot Count: %d", materialCount);
    ImGui::Text("Vertex Count: %d", vertexCount);
    ImGui::Text("Index Count: %d", indexCount);
    ImGui::TextDisabled("File replacement is deferred.");
}

const char* FBXRender::GetSerializableType() const
{
    return "FBXRender";
}

std::string FBXRender::Serialize() const
{
    std::ostringstream oss;
    oss << "{ ";
    oss << "\"fbxPath\": \"" << SceneJson::EscapeString(m_fbxFileName) << "\"";
    oss << " }";
    return oss.str();
}

bool FBXRender::Deserialize(const std::string& componentJson)
{
    std::string fbxPath;
    if (SceneJson::ReadString(componentJson, "fbxPath", fbxPath))
    {
        m_fbxFileName = fbxPath;
    }
    if (m_fbxFileName.empty())
    {
        std::cout << "FBXRender deserialize: fbxPath missing." << std::endl;
    }
    return true;
}
