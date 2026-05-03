#pragma once
#include <d3dx9.h>
#include <functional>
#include <string>
#include <unordered_map>
#include "Resource/IResourceTypeManager.h"

class TextureManager : public IResourceTypeManager {

public:
    // 생성자와 소멸자
    TextureManager();
    ~TextureManager();

    // 텍스처를 불러오고 돌려주는 함수
    IDirect3DTexture9* GetTexture(const std::string& filepath);
    IDirect3DTexture9* GetTexture(const std::string& filepath, bool useMagentaColorKey);
    IDirect3DTexture9* GetTexture(const std::wstring& filepath);

    // 비동기로 텍스처를 불러오는 함수
    void GetTexture(const std::string& filepath, std::function<void(IDirect3DTexture9*)> func);
    void GetTexture(const std::wstring& filepath, std::function<void(IDirect3DTexture9*)> func);

    // 들고 있는 텍스처를 모두 해제하는 함수
    void ReleaseAllTextures();
    void ReleaseAllResources() override;

private:
    std::unordered_map<std::string, IDirect3DTexture9*> m_textureMap; // 경로별 텍스처 보관 맵
    std::unordered_map<std::string, IDirect3DTexture9*> m_colorKeyTextureMap;
    std::unordered_map<std::wstring, IDirect3DTexture9*> m_textureMapW; 

};
