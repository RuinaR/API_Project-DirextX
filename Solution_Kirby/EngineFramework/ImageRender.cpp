#include "pch.h"
#include "ImageRender.h"

void ImageRender::DrawImage(int x, int y, int z, int w, int h)
{
    if (m_gameObj->GetDestroy())
        return;

    SetupVertices();

    // Set world, view, and projection matrices here
    D3DXMATRIX matWorld, matScale, matTrans, matRotation;
    D3DXMATRIX matRotationX, matRotationY, matRotationZ;
    D3DXMatrixScaling(&matScale, m_gameObj->Size2D().x, m_gameObj->Size2D().y, 1.0f);
    D3DXMatrixTranslation(&matTrans, m_gameObj->Position().x, m_gameObj->Position().y, m_gameObj->Position().z);

    D3DXMatrixRotationX(&matRotationX, D3DXToRadian(m_gameObj->GetAngleX()));
    D3DXMatrixRotationY(&matRotationY, D3DXToRadian(m_gameObj->GetAngleY()));
    D3DXMatrixRotationZ(&matRotationZ, D3DXToRadian(m_gameObj->GetAngleZ()));
    matRotation = matRotationZ * matRotationX * matRotationY;
    matWorld = matScale * matRotation * matTrans;

    m_device->SetTransform(D3DTS_WORLD, &matWorld);

    m_device->SetTexture(0, m_texture);
    m_device->SetStreamSource(0, m_vertexBuffer, 0, sizeof(CUSTOMVERTEX));

    m_device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2);
}

void ImageRender::SetupVertices()
{
    CUSTOMVERTEX vertices[4];
    vertices[0].x = -0.5f; // 왼쪽 상단 모서리
    vertices[0].y = 0.5f;
    vertices[0].z = 0.0f;
    vertices[0].color = 0xffffffff;
    vertices[0].tu = 0.0f;
    vertices[0].tv = 0.0f;

    vertices[1].x = -0.5f; // 왼쪽 하단 모서리
    vertices[1].y = -0.5f;
    vertices[1].z = 0.0f;
    vertices[1].color = 0xffffffff;
    vertices[1].tu = 0.0f;
    vertices[1].tv = 1.0f;

    vertices[2].x = 0.5f; // 오른쪽 상단 모서리
    vertices[2].y = 0.5f;
    vertices[2].z = 0.0f;
    vertices[2].color = 0xffffffff;
    vertices[2].tu = 1.0f;
    vertices[2].tv = 0.0f;

    vertices[3].x = 0.5f; // 오른쪽 하단 모서리
    vertices[3].y = -0.5f;
    vertices[3].z = 0.0f;
    vertices[3].color = 0xffffffff;
    vertices[3].tu = 1.0f;
    vertices[3].tv = 1.0f;

    // Lock the vertex buffer
    VOID* pVertices;
    m_vertexBuffer->Lock(0, sizeof(CUSTOMVERTEX) * 4, (void**)&pVertices, 0);
    memcpy(pVertices, vertices, sizeof(CUSTOMVERTEX) * 4);
    m_vertexBuffer->Unlock();

    // Define indices
    WORD indices[] =
    {
        0, 1, 2,    // First triangle
        2, 1, 3     // Second triangle
    };

    // index buffer
    WORD* pIndices;
    m_indexBuffer->Lock(0, sizeof(indices), (void**)&pIndices, 0);
    memcpy(pIndices, indices, sizeof(indices));
    m_indexBuffer->Unlock();

    // Set the index buffer
    m_device->SetIndices(m_indexBuffer);

    // Release the index buffer
    //m_indexBuffer->Release();
}


float ImageRender::GetZ()
{
    return m_gameObj->Position().z;
}

void ImageRender::Render()
{
    if (m_texture == nullptr)
    	return;

    DrawImage(
    	m_gameObj->Position().x,
    	m_gameObj->Position().y,
    	m_gameObj->Position().z,
    	m_gameObj->Size2D().x,
    	m_gameObj->Size2D().y);
}

ImageRender::ImageRender(IDirect3DTexture9* texture) : Component(), m_texture(texture)
{
}

void ImageRender::SetTrans(bool trans)
{
    RenderManager::GetInstance()->Unresister(this);
    m_isTrans = trans;
    RenderManager::GetInstance()->Resister(this);
}

bool ImageRender::IsTrans()
{
    return m_isTrans;
}

void ImageRender::Initialize()
{
	m_device = MainFrame::GetInstance()->GetDevice();

	m_device->
		CreateVertexBuffer(4 * sizeof(CUSTOMVERTEX), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_MANAGED, &m_vertexBuffer, NULL);
    m_device->
        CreateIndexBuffer(6 * sizeof(WORD), 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_indexBuffer, nullptr);
    RenderManager::GetInstance()->Resister(this);
}

void ImageRender::Release()
{
    RenderManager::GetInstance()->Unresister(this);

    m_vertexBuffer->Release();
    m_indexBuffer->Release();
}

void ImageRender::ChangeTexture(IDirect3DTexture9* texture)
{
    m_texture = texture;
}

void ImageRender::Start()
{
}

void ImageRender::Update()
{
}
