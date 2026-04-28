#include "pch.h"
#include "ImageRender.h"
#include "SceneJsonUtility.h"

void ImageRender::DrawImage(int x, int y, int z, int w, int h)
{
    if (m_gameObj->GetDestroy())
        return;

    SetupVertices();

    // 월드 변환 행렬을 구성한다.
    D3DXMATRIX matWorld, matScale, matTrans, matRotation;
    D3DXMATRIX matRotationX, matRotationY, matRotationZ;
    D3DXMatrixScaling(&matScale, m_gameObj->Size2D().x, m_gameObj->Size2D().y, 1.0f);
    D3DXMatrixTranslation(&matTrans, m_gameObj->Position().x, m_gameObj->Position().y, m_gameObj->Position().z);

    D3DXMatrixRotationX(&matRotationX, m_gameObj->GetAngleX());
    D3DXMatrixRotationY(&matRotationY, m_gameObj->GetAngleY());
    D3DXMatrixRotationZ(&matRotationZ, m_gameObj->GetAngleZ());
    matRotation = matRotationZ * matRotationX * matRotationY;
    matWorld = matScale * matRotation * matTrans;

    m_device->SetTransform(D3DTS_WORLD, &matWorld);

    if (m_useTexture && m_texture)
    {
        m_device->SetTexture(0, m_texture);
        m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        m_device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        m_device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    }
    else
    {
        m_device->SetTexture(0, nullptr);
        m_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        m_device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
    }
    m_device->SetStreamSource(0, m_vertexBuffer, 0, sizeof(CUSTOMVERTEX));

    m_device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2);
}

void ImageRender::InitGameObj(GameObject* obj)
{
    Component::InitGameObj(obj);

    //추가 잡업 필요?
}

void ImageRender::LoadTextureCallback(IDirect3DTexture9* tex)
{
    m_texture = tex;
}

void ImageRender::SetupVertices()
{
    const float upperV = m_isUIRender ? 1.0f : 0.0f;
    const float lowerV = m_isUIRender ? 0.0f : 1.0f;

    CUSTOMVERTEX vertices[4];
    vertices[0].x = -0.5f; // 왼쪽 상단 모서리
    vertices[0].y = 0.5f;
    vertices[0].z = 0.0f;
    vertices[0].color = m_color;
    vertices[0].tu = 0.0f;
    vertices[0].tv = upperV;

    vertices[1].x = -0.5f; // 왼쪽 하단 모서리
    vertices[1].y = -0.5f;
    vertices[1].z = 0.0f;
    vertices[1].color = m_color;
    vertices[1].tu = 0.0f;
    vertices[1].tv = lowerV;

    vertices[2].x = 0.5f; // 오른쪽 상단 모서리
    vertices[2].y = 0.5f;
    vertices[2].z = 0.0f;
    vertices[2].color = m_color;
    vertices[2].tu = 1.0f;
    vertices[2].tv = upperV;

    vertices[3].x = 0.5f; // 오른쪽 하단 모서리
    vertices[3].y = -0.5f;
    vertices[3].z = 0.0f;
    vertices[3].color = m_color;
    vertices[3].tu = 1.0f;
    vertices[3].tv = lowerV;

    // 정점 버퍼에 사각형 정점 정보를 갱신한다.
    VOID* pVertices;
    m_vertexBuffer->Lock(0, sizeof(CUSTOMVERTEX) * 4, (void**)&pVertices, 0);
    memcpy(pVertices, vertices, sizeof(CUSTOMVERTEX) * 4);
    m_vertexBuffer->Unlock();

    // 사각형을 삼각형 두 개로 그리기 위한 인덱스
    WORD indices[] =
    {
        0, 1, 2,    // 첫 번째 삼각형
        2, 1, 3     // 두 번째 삼각형
    };

    // 인덱스 버퍼에 인덱스 정보를 갱신한다.
    WORD* pIndices;
    m_indexBuffer->Lock(0, sizeof(indices), (void**)&pIndices, 0);
    memcpy(pIndices, indices, sizeof(indices));
    m_indexBuffer->Unlock();

    // 갱신된 인덱스 버퍼를 장치에 바인딩한다.
    m_device->SetIndices(m_indexBuffer);

    // 인덱스 버퍼 해제는 Release에서 처리한다.
    //m_indexBuffer->Release();
}


float ImageRender::GetZ()
{
    return m_gameObj->Position().z;
}

void ImageRender::Render()
{
    if (!m_renderEnabled)
        return;

    if (m_useTexture && m_texture == nullptr)
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
    if (m_isTrans == trans)
        return;

    if (m_isUIRender)
    {
        m_isTrans = trans;
        return;
    }

    RenderManager::GetInstance()->Unregister(this);
    m_isTrans = trans;
    RenderManager::GetInstance()->Register(this);
}

bool ImageRender::IsTrans()
{
    return m_isTrans;
}

void ImageRender::SetUIRender(bool isUIRender)
{
    if (m_isUIRender == isUIRender)
        return;

    m_isUIRender = isUIRender;

    if (m_isUIRender)
    {
        RenderManager::GetInstance()->RegisterUI(this);
    }
    else
    {
        RenderManager::GetInstance()->UnregisterUI(this);
        RenderManager::GetInstance()->Register(this);
    }
}

bool ImageRender::IsUIRender()
{
    return m_isUIRender;
}

void ImageRender::SetOrderInLayer(int orderInLayer)
{
    if (m_orderInLayer == orderInLayer)
        return;

    m_orderInLayer = orderInLayer;

    if (m_isUIRender)
    {
        RenderManager::GetInstance()->RefreshUIOrder(this);
    }
}

int ImageRender::GetOrderInLayer()
{
    return m_orderInLayer;
}

void ImageRender::SetRenderEnabled(bool renderEnabled)
{
    m_renderEnabled = renderEnabled;
}

bool ImageRender::IsRenderEnabled()
{
    return m_renderEnabled;
}

void ImageRender::SetUseTexture(bool useTexture)
{
    m_useTexture = useTexture;
}

bool ImageRender::IsUseTexture()
{
    return m_useTexture;
}

void ImageRender::SetColor(D3DCOLOR color)
{
    m_color = color;
}

D3DCOLOR ImageRender::GetColor()
{
    return m_color;
}

void ImageRender::Initialize()
{
	m_device = MainFrame::GetInstance()->GetDevice();

	m_device->
	CreateVertexBuffer(4 * sizeof(CUSTOMVERTEX), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_MANAGED, &m_vertexBuffer, NULL);
    m_device->
        CreateIndexBuffer(6 * sizeof(WORD), 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_indexBuffer, nullptr);
    if (m_isUIRender)
    {
        RenderManager::GetInstance()->RegisterUI(this);
    }
    else
    {
        RenderManager::GetInstance()->Register(this);
    }
}

void ImageRender::Release()
{
    if (m_isUIRender)
    {
        RenderManager::GetInstance()->UnregisterUI(this);
    }
    else
    {
        RenderManager::GetInstance()->Unregister(this);
    }

    m_vertexBuffer->Release();
    m_indexBuffer->Release();
}

void ImageRender::ChangeTexture(IDirect3DTexture9* texture)
{
    m_texture = texture;
}

void ImageRender::SetTexturePath(const std::string& path)
{
    m_texturePath = path;
    if (!m_texturePath.empty())
    {
        m_texture = TextureManager::GetInstance()->GetTexture(m_texturePath);
    }
}

const std::string& ImageRender::GetTexturePath() const
{
    return m_texturePath;
}

void ImageRender::Start()
{
}

void ImageRender::Update()
{
}

const char* ImageRender::GetInspectorName() const
{
    return "ImageRender";
}

void ImageRender::DrawInspector()
{
    bool uiRender = m_isUIRender;
    if (ImGui::Checkbox("UI Render", &uiRender))
    {
        SetUIRender(uiRender);
    }

    bool renderEnabled = m_renderEnabled;
    if (ImGui::Checkbox("Render Enabled", &renderEnabled))
    {
        SetRenderEnabled(renderEnabled);
    }

    bool useTexture = m_useTexture;
    if (ImGui::Checkbox("Use Texture", &useTexture))
    {
        SetUseTexture(useTexture);
    }

    bool trans = m_isTrans;
    if (ImGui::Checkbox("Transparent Queue", &trans))
    {
        SetTrans(trans);
    }

    int orderInLayer = m_orderInLayer;
    if (ImGui::DragInt("Order In Layer", &orderInLayer))
    {
        SetOrderInLayer(orderInLayer);
    }

    float color[4] =
    {
        static_cast<float>((m_color >> 16) & 0xff) / 255.0f,
        static_cast<float>((m_color >> 8) & 0xff) / 255.0f,
        static_cast<float>(m_color & 0xff) / 255.0f,
        static_cast<float>((m_color >> 24) & 0xff) / 255.0f
    };
    if (ImGui::ColorEdit4("Color", color))
    {
        SetColor(D3DCOLOR_COLORVALUE(color[0], color[1], color[2], color[3]));
    }

    ImGui::Text("Texture: %s", m_texture ? "Loaded" : "None");
    ImGui::Text("Texture Path: %s", m_texturePath.empty() ? "(none)" : m_texturePath.c_str());
}

const char* ImageRender::GetSerializableType() const
{
    return "ImageRender";
}

std::string ImageRender::Serialize() const
{
    std::ostringstream oss;
    oss << "{ ";
    oss << "\"renderEnabled\": " << (m_renderEnabled ? "true" : "false") << ", ";
    oss << "\"useTexture\": " << (m_useTexture ? "true" : "false") << ", ";
    oss << "\"color\": " << static_cast<DWORD>(m_color) << ", ";
    oss << "\"isUIRender\": " << (m_isUIRender ? "true" : "false") << ", ";
    oss << "\"orderInLayer\": " << m_orderInLayer << ", ";
    oss << "\"trans\": " << (m_isTrans ? "true" : "false") << ", ";
    oss << "\"texturePath\": \"" << SceneJson::EscapeString(m_texturePath) << "\"";
    oss << " }";
    return oss.str();
}

bool ImageRender::Deserialize(const std::string& componentJson)
{
    bool renderEnabled = m_renderEnabled;
    bool useTexture = m_useTexture;
    bool isUIRender = m_isUIRender;
    bool trans = m_isTrans;
    int orderInLayer = m_orderInLayer;
    DWORD color = static_cast<DWORD>(m_color);
    std::string texturePath;

    SceneJson::ReadBool(componentJson, "renderEnabled", renderEnabled);
    SceneJson::ReadBool(componentJson, "useTexture", useTexture);
    SceneJson::ReadBool(componentJson, "isUIRender", isUIRender);
    SceneJson::ReadBool(componentJson, "trans", trans);
    SceneJson::ReadInt(componentJson, "orderInLayer", orderInLayer);
    SceneJson::ReadDword(componentJson, "color", color);
    SceneJson::ReadString(componentJson, "texturePath", texturePath);

    SetRenderEnabled(renderEnabled);
    SetUseTexture(useTexture);
    SetColor(static_cast<D3DCOLOR>(color));
    SetTrans(trans);
    SetOrderInLayer(orderInLayer);
    if (!texturePath.empty())
    {
        SetTexturePath(texturePath);
    }
    SetUIRender(isUIRender);
    return true;
}
