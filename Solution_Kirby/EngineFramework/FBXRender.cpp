#include "pch.h"
#include "FBXRender.h"

FBXRender::FBXRender(std::string name)
    : m_name(name), m_tool()
{
}

void FBXRender::Initialize()
{
    m_tool.Initialize();
    m_tool.Load(m_name.c_str());

    m_vertexCount = (int)m_tool.GetVertexCount();
    m_indexCount = (int)m_tool.GetIndexCount();

    // VertexBuffer와 IndexBuffer 생성
    MainFrame::GetInstance()->GetDevice()->CreateVertexBuffer(
        m_vertexCount * sizeof(CUSTOMVERTEX), // XMFLOAT3 대신 CUSTOMVERTEX 사용
        0,
        D3DFVF_CUSTOMVERTEX,
        D3DPOOL_MANAGED,
        &m_pVertexBuffer,
        nullptr
    );

    MainFrame::GetInstance()->GetDevice()->CreateIndexBuffer(
        m_indexCount * sizeof(unsigned int),
        0,
        D3DFMT_INDEX32,
        D3DPOOL_MANAGED,
        &m_pIndexBuffer,
        nullptr
    );

    RenderManager::GetInstance()->Resister(this);
}

void FBXRender::Release()
{
    RenderManager::GetInstance()->Unresister(this);

    m_tool.Release();

    if (m_pVertexBuffer) m_pVertexBuffer->Release();
    if (m_pIndexBuffer) m_pIndexBuffer->Release();
}

void FBXRender::Start()
{
}

void FBXRender::Update()
{
    m_tool.ImguiUpdate();
}

void FBXRender::Render()
{
    // 버텍스 데이터 복사
    CUSTOMVERTEX* vertices;
    m_pVertexBuffer->Lock(0, m_vertexCount * sizeof(CUSTOMVERTEX), (void**)&vertices, 0);
    memcpy(vertices, m_tool.GetPositions(), m_vertexCount * sizeof(CUSTOMVERTEX));
    m_pVertexBuffer->Unlock();

    // 인덱스 데이터 복사
    unsigned int* indices;
    m_pIndexBuffer->Lock(0, m_indexCount * sizeof(unsigned int), (void**)&indices, 0);
    memcpy(indices, m_tool.GetIndices(), m_indexCount * sizeof(unsigned int));
    m_pIndexBuffer->Unlock();

    // 월드 변환 행렬 설정
    D3DXMATRIX matWorld, matScale, matTrans, matRotation;
    D3DXMATRIX matRotationX, matRotationY, matRotationZ;

    D3DXMatrixScaling(&matScale, m_gameObj->Size3D().x, m_gameObj->Size3D().y, m_gameObj->Size3D().z);
    D3DXMatrixTranslation(&matTrans, m_gameObj->Position().x, m_gameObj->Position().y, m_gameObj->Position().z);

    D3DXMatrixRotationX(&matRotationX, D3DXToRadian(m_gameObj->GetAngleX()));
    D3DXMatrixRotationY(&matRotationY, D3DXToRadian(m_gameObj->GetAngleY()));
    D3DXMatrixRotationZ(&matRotationZ, D3DXToRadian(m_gameObj->GetAngleZ()));

    matRotation = matRotationX * matRotationY * matRotationZ;
    matWorld = matScale * matRotation * matTrans;

    MainFrame::GetInstance()->GetDevice()->SetTransform(D3DTS_WORLD, &matWorld);

    // DirectX 장비 설정
    MainFrame::GetInstance()->GetDevice()->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(CUSTOMVERTEX));
    MainFrame::GetInstance()->GetDevice()->SetIndices(m_pIndexBuffer);
    MainFrame::GetInstance()->GetDevice()->SetFVF(D3DFVF_CUSTOMVERTEX);

    MainFrame::GetInstance()->GetDevice()->DrawIndexedPrimitive(
        D3DPT_TRIANGLELIST, 0, 0, m_vertexCount, 0, m_indexCount / 3
    );
}