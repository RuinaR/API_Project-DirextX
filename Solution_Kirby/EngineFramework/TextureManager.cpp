#include "pch.h"
#include "TextureManager.h"

#include <thread>
#include <future>

TextureManager* TextureManager::m_Pthis = nullptr;

TextureManager::TextureManager()  {}

TextureManager::~TextureManager() {}

void TextureManager::Create()
{
    if (!m_Pthis)
    {
        m_Pthis = new TextureManager();
    }
}

TextureManager* TextureManager::GetInstance()
{
    return m_Pthis;
}

void TextureManager::Destroy()
{
    if (m_Pthis)
    {
        m_Pthis->ReleaseAllTextures();
        delete m_Pthis;
        m_Pthis = nullptr;
    }
}

IDirect3DTexture9* TextureManager::GetTexture(const std::string& filepath) {
    // �ʿ��� �ؽ�ó�� �˻�
    auto it = m_textureMap.find(filepath);
    if (it != m_textureMap.end()) {
        // �̹� �����ϴ� �ؽ�ó ��ȯ
        return it->second;
    }

    // ���ο� �ؽ�ó �ε�
    IDirect3DTexture9* texture = nullptr;
    if (SUCCEEDED(D3DXCreateTextureFromFileA(MainFrame::GetInstance()->GetDevice(), filepath.c_str(), &texture))) {
        m_textureMap[filepath] = texture; // �ʿ� �߰�
        return texture;
    }

    return nullptr; // �ε� ���� �� nullptr ��ȯ
}

IDirect3DTexture9* TextureManager::GetTexture(const std::wstring& filepath)
{
    // �ʿ��� �ؽ�ó�� �˻�
    auto it = m_textureMapW.find(filepath);
    if (it != m_textureMapW.end()) {
        // �̹� �����ϴ� �ؽ�ó ��ȯ
        return it->second;
    }

    // ���ο� �ؽ�ó �ε�
    IDirect3DTexture9* texture = nullptr;
    if (SUCCEEDED(D3DXCreateTextureFromFileW(MainFrame::GetInstance()->GetDevice(), filepath.c_str(), &texture))) {
        m_textureMapW[filepath] = texture; // �ʿ� �߰�
        return texture;
    }

    return nullptr; // �ε� ���� �� nullptr ��ȯ
}


//�񵿱� �ؽ�ó �ε� �Լ�
void TextureManager::GetTexture(const std::string& filepath, std::function<void(IDirect3DTexture9*)> func)
{
    // �ʿ��� �ؽ�ó�� �˻�
    auto it = m_textureMap.find(filepath);
    if (it != m_textureMap.end()) {
        // �̹� �����ϴ� �ؽ�ó�� �ִٸ� �ݹ� ȣ��
        func(it->second);
        return;
    }

    // ���ο� �ؽ�ó �񵿱� �ε�
    std::async(std::launch::async, [this, filepath, func]() {
        IDirect3DTexture9* texture = nullptr;
        if (SUCCEEDED(D3DXCreateTextureFromFileA(MainFrame::GetInstance()->GetDevice(), filepath.c_str(), &texture))) {
            this->m_textureMap[filepath] = texture; // �ʿ� �߰�

            // �ε尡 �Ϸ�Ǹ� �ݹ� ȣ��
            func(texture);
        }
        else {
            // �ε� ���� �� nullptr 
            func(nullptr);
        }
        });
}

void TextureManager::GetTexture(const std::wstring& filepath, std::function<void(IDirect3DTexture9*)> func)
{
    // �ʿ��� �ؽ�ó�� �˻�
    auto it = m_textureMapW.find(filepath);
    if (it != m_textureMapW.end()) {
        // �̹� �����ϴ� �ؽ�ó�� �ִٸ� �ݹ� ȣ��
        func(it->second);
        return;
    }

    // ���ο� �ؽ�ó �񵿱� �ε�
    std::async(std::launch::async, [this, filepath, func]() {
        IDirect3DTexture9* texture = nullptr;
        if (SUCCEEDED(D3DXCreateTextureFromFileW(MainFrame::GetInstance()->GetDevice(), filepath.c_str(), &texture))) {
            this->m_textureMapW[filepath] = texture; // �ʿ� �߰�

            // �ε尡 �Ϸ�Ǹ� �ݹ� ȣ��
            func(texture);
        }
        else {
            // �ε� ���� �� nullptr 
            func(nullptr);
        }
        });
}

void TextureManager::ReleaseAllTextures() 
{
    for (auto& pair : m_textureMap) {
        if (pair.second) {
            pair.second->Release();
        }
    }
    m_textureMap.clear();

    for (auto& pair : m_textureMapW) {
        if (pair.second) {
            pair.second->Release();
        }
    }
    m_textureMapW.clear();
}
