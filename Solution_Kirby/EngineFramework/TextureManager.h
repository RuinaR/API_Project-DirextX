#pragma once
#include <d3dx9.h>
#include <string>
#include <unordered_map>

class TextureManager {

public:
    static void Create();
    static TextureManager* GetInstance();
    static void Destroy();
public:
    // 생성자와 소멸자
    TextureManager();
    ~TextureManager();

    // 텍스처 로딩 및 반환 함수
    IDirect3DTexture9* GetTexture(const std::string& filepath);
    IDirect3DTexture9* GetTexture(const std::wstring& filepath);

    //비동기 텍스처 로딩 함수
    void GetTexture(const std::string& filepath, std::function<void(IDirect3DTexture9*)> func);
    void GetTexture(const std::wstring& filepath, std::function<void(IDirect3DTexture9*)> func);

    // 모든 텍스처 해제 함수
    void ReleaseAllTextures();

private:
    static TextureManager* m_Pthis;
    std::unordered_map<std::string, IDirect3DTexture9*> m_textureMap; // 경로를 키로, 텍스처 포인터를 값으로 하는 맵
    std::unordered_map<std::wstring, IDirect3DTexture9*> m_textureMapW; // 경로를 키로, 텍스처 포인터를 값으로 하는 맵

};
