#include "pch.h"
#include "FBXRender.h"

// 생성자: FBX 파일 이름을 받아 초기화
FBXRender::FBXRender(std::string name) : m_fbxFileName(std::move(name)) {}

// 초기화 함수
void FBXRender::Initialize() {
    auto device = MainFrame::GetInstance()->GetDevice();  // Direct3D 디바이스 가져오기

    if (!m_tool.Initialize()) {
        m_logSystem.AddLog("Error: Failed to initialize FbxTool!");
        return;
    }

    if (!m_tool.Load(m_fbxFileName.c_str(), m_models)) {
        m_logSystem.AddLog("Error: Failed to load FBX file");
        return;
    }

    m_logSystem.AddLog("FBX file loaded successfully");

    for (auto& model : m_models)
    {
        m_tool.CreateVertexBuffer(model);
        m_tool.CreateIndexBuffer(model);
    }

    RenderManager::GetInstance()->Register(this);  // Register로 수정
}

// 자원 해제 함수
void FBXRender::Release() {
    m_logSystem.AddLog("FBXRender resources released.");
    RenderManager::GetInstance()->Unregister(this);  // Unregister로 수정

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
    auto device = MainFrame::GetInstance()->GetDevice();
    if (!device) {
        m_logSystem.AddLog("Error: Direct3D device is null!");
        return;
    }

    D3DXMATRIX matWorld;
    SetWorldTransform(matWorld);

    for (auto& model : m_models) {
        if (!model.vertexBuffer || !model.indexBuffer) {
            m_logSystem.AddLog("Error: Vertex or index buffer is null for model!");
            continue;
        }

        // Stream source와 인덱스 버퍼 설정
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

            // 서브메쉬 텍스처 설정
            for (size_t i = 0; i < subMesh.textures.size() && i < 8; ++i) {
                if (subMesh.textures[i]) {
                    device->SetTexture(static_cast<DWORD>(i), subMesh.textures[i]);
                    m_logSystem.AddLog("Setting texture for slot " + std::to_string(i));
                }
                else {
                    device->SetTexture(static_cast<DWORD>(i), nullptr);
                    m_logSystem.AddLog("Setting texture for slot " + std::to_string(i) + ": nullptr");
                }
            }

            // 서브메쉬 렌더링
            HRESULT hr = device->DrawIndexedPrimitive(
                D3DPT_TRIANGLELIST,
                subMesh.vertexStart,
                0,
                model.vertexCount,
                subMesh.startIndex,
                subMesh.indexCount / 3
            );

            // DrawIndexedPrimitive 결과 확인
            if (FAILED(hr)) {
                m_logSystem.AddLog("Error: DrawIndexedPrimitive failed!");
            }
        }
    }
}



// 월드 변환 설정
void FBXRender::SetWorldTransform(D3DXMATRIX& matWorld) {
    D3DXVECTOR3 angle = { m_gameObj->GetAngleX(), m_gameObj->GetAngleY(), m_gameObj->GetAngleZ() };
    D3DXVECTOR3 scale = m_gameObj->Size3D();
    D3DXVECTOR3 position = m_gameObj->Position();

    D3DXMATRIX matScale, matRotation, matTranslation;
    D3DXMatrixScaling(&matScale, scale.x, scale.y, scale.z);
    D3DXMatrixRotationYawPitchRoll(&matRotation, D3DXToRadian(angle.y), D3DXToRadian(angle.x), D3DXToRadian(angle.z));
    D3DXMatrixTranslation(&matTranslation, position.x, position.y, position.z);

    matWorld = matScale * matRotation * matTranslation;
    MainFrame::GetInstance()->GetDevice()->SetTransform(D3DTS_WORLD, &matWorld);
}
