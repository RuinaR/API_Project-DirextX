#include "pch.h"
#include "ImageRender.h"

void ImageRender::DrawImage(int x, int y, int z, int w, int h)
{
    if (m_gameObj->GetDestroy())
        return;

    SetupVertices();

    // Set world, view, and projection matrices here
    D3DXMATRIX matWorld, matScale, matTrans, matRotate;
    D3DXMatrixScaling(&matScale, m_gameObj->Size().x, m_gameObj->Size().y, 1.0f);
    D3DXMatrixTranslation(&matTrans, m_gameObj->Position().x, m_gameObj->Position().y, m_gameObj->Position().z);
    D3DXMatrixRotationZ(&matRotate, m_gameObj->GetAngle());
    matWorld = matScale * matRotate * matTrans;

    m_device->SetRenderState(D3DRS_LIGHTING, FALSE);

    m_device->SetFVF(D3DFVF_CUSTOMVERTEX);
    m_device->SetTransform(D3DTS_WORLD, &matWorld);

    m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    m_device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    m_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

    m_device->SetTexture(0, m_texture);
    m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    m_device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    m_device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE); // 색상과 텍스처를 곱하여 출력
    m_device->SetStreamSource(0, m_vertexBuffer, 0, sizeof(CUSTOMVERTEX));

    m_device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2);
    m_device->SetRenderState(D3DRS_LIGHTING, TRUE);
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

    // Create and lock the index buffer
    IDirect3DIndexBuffer9* indexBuffer;
    m_device->CreateIndexBuffer(6 * sizeof(WORD), 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &indexBuffer, nullptr);
    WORD* pIndices;
    indexBuffer->Lock(0, sizeof(indices), (void**)&pIndices, 0);
    memcpy(pIndices, indices, sizeof(indices));
    indexBuffer->Unlock();

    // Set the index buffer
    m_device->SetIndices(indexBuffer);

    // Release the index buffer
    indexBuffer->Release();
}


ImageRender::ImageRender(IDirect3DTexture9* texture) : Component(), m_texture(texture)
{
}

void ImageRender::Initialize()
{
	m_device = MainFrame::GetInstance()->GetDevice();

	m_device->
		CreateVertexBuffer(4 * sizeof(CUSTOMVERTEX), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_MANAGED, &m_vertexBuffer, NULL);
}

void ImageRender::Release()
{
    m_vertexBuffer->Release();
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
	if (m_texture == nullptr)
		return;

	DrawImage(
		m_gameObj->Position().x,
		m_gameObj->Position().y,
		m_gameObj->Position().z,
		m_gameObj->Size().x,
		m_gameObj->Size().y);

}
