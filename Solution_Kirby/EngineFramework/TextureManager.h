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
    // �����ڿ� �Ҹ���
    TextureManager();
    ~TextureManager();

    // �ؽ�ó �ε� �� ��ȯ �Լ�
    IDirect3DTexture9* GetTexture(const std::string& filepath);
    IDirect3DTexture9* GetTexture(const std::wstring& filepath);

    //�񵿱� �ؽ�ó �ε� �Լ�
    void GetTexture(const std::string& filepath, std::function<void(IDirect3DTexture9*)> func);
    void GetTexture(const std::wstring& filepath, std::function<void(IDirect3DTexture9*)> func);

    // ��� �ؽ�ó ���� �Լ�
    void ReleaseAllTextures();

private:
    static TextureManager* m_Pthis;
    std::unordered_map<std::string, IDirect3DTexture9*> m_textureMap; // ��θ� Ű��, �ؽ�ó �����͸� ������ �ϴ� ��
    std::unordered_map<std::wstring, IDirect3DTexture9*> m_textureMapW; // ��θ� Ű��, �ؽ�ó �����͸� ������ �ϴ� ��

};
