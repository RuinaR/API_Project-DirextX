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
    // 맵에서 텍스처를 검색
    auto it = m_textureMap.find(filepath);
    if (it != m_textureMap.end()) {
        // 이미 존재하는 텍스처 반환
        return it->second;
    }

    // 새로운 텍스처 로드
    IDirect3DTexture9* texture = nullptr;
    if (SUCCEEDED(D3DXCreateTextureFromFileA(MainFrame::GetInstance()->GetDevice(), filepath.c_str(), &texture))) {
        m_textureMap[filepath] = texture; // 맵에 추가
        return texture;
    }

    return nullptr; // 로드 실패 시 nullptr 반환
}

IDirect3DTexture9* TextureManager::GetTexture(const std::wstring& filepath)
{
    // 맵에서 텍스처를 검색
    auto it = m_textureMapW.find(filepath);
    if (it != m_textureMapW.end()) {
        // 이미 존재하는 텍스처 반환
        return it->second;
    }

    // 새로운 텍스처 로드
    IDirect3DTexture9* texture = nullptr;
    if (SUCCEEDED(D3DXCreateTextureFromFileW(MainFrame::GetInstance()->GetDevice(), filepath.c_str(), &texture))) {
        m_textureMapW[filepath] = texture; // 맵에 추가
        return texture;
    }

    return nullptr; // 로드 실패 시 nullptr 반환
}


//비동기 텍스처 로딩 함수
void TextureManager::GetTexture(const std::string& filepath, std::function<void(IDirect3DTexture9*)> func)
{
    // 맵에서 텍스처를 검색
    auto it = m_textureMap.find(filepath);
    if (it != m_textureMap.end()) {
        // 이미 존재하는 텍스처가 있다면 콜백 호출
        func(it->second);
        return;
    }

    // 새로운 텍스처 비동기 로드
    std::async(std::launch::async, [this, filepath, func]() {
        IDirect3DTexture9* texture = nullptr;
        if (SUCCEEDED(D3DXCreateTextureFromFileA(MainFrame::GetInstance()->GetDevice(), filepath.c_str(), &texture))) {
            this->m_textureMap[filepath] = texture; // 맵에 추가

            // 로드가 완료되면 콜백 호출
            func(texture);
        }
        else {
            // 로드 실패 시 nullptr 
            func(nullptr);
        }
        });
}

void TextureManager::GetTexture(const std::wstring& filepath, std::function<void(IDirect3DTexture9*)> func)
{
    // 맵에서 텍스처를 검색
    auto it = m_textureMapW.find(filepath);
    if (it != m_textureMapW.end()) {
        // 이미 존재하는 텍스처가 있다면 콜백 호출
        func(it->second);
        return;
    }

    // 새로운 텍스처 비동기 로드
    std::async(std::launch::async, [this, filepath, func]() {
        IDirect3DTexture9* texture = nullptr;
        if (SUCCEEDED(D3DXCreateTextureFromFileW(MainFrame::GetInstance()->GetDevice(), filepath.c_str(), &texture))) {
            this->m_textureMapW[filepath] = texture; // 맵에 추가

            // 로드가 완료되면 콜백 호출
            func(texture);
        }
        else {
            // 로드 실패 시 nullptr 
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
