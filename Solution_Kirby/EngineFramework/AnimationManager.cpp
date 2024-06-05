#include "pch.h"
#include "AnimationRender.h"
bool AnimationManager::IsBitmapFile(const wstring& filename)
{
    size_t dotIndex = filename.find_last_of('.');
    if (dotIndex != string::npos)
    {
        wstring extension = filename.substr(dotIndex + 1);
        transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
        return extension == L"bmp";
    }
    return false;
}

Animation AnimationManager::LoadAnimation(const wstring& folderName, float time)
{
    WCHAR buffer[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, buffer);
    wstring currentDirectory = buffer;
    vector<IDirect3DTexture9*> textures;
    wstring searchPath = currentDirectory + L"\\" + folderName +L"\\*.*";
    WIN32_FIND_DATAW fileData;
    D3DXIMAGE_INFO imageInfo;
    HANDLE hFind = FindFirstFile(searchPath.c_str(), &fileData);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do {
            wstring fileName = fileData.cFileName;
            if (IsBitmapFile(fileName))
            {
                wstring filePath = currentDirectory + L"\\" + folderName + L"\\" + fileName;
                IDirect3DTexture9* texture = nullptr;
                if (SUCCEEDED(D3DXCreateTextureFromFileEx(
                    MainFrame::GetInstance()->GetDevice(),
                    filePath.c_str(),
                    D3DX_DEFAULT,
                    D3DX_DEFAULT,
                    D3DX_DEFAULT,
                    0,
                    D3DFMT_UNKNOWN,
                    D3DPOOL_MANAGED,
                    D3DX_DEFAULT,
                    D3DX_DEFAULT,
                    TRANSCOLOR,
                    &imageInfo,
                    NULL,
                    &texture)))
                {
                    textures.push_back(texture);
                }
                else
                {
                    wcout << L"Failed to load texture: " << filePath << endl;
                }
            }
        } while (FindNextFile(hFind, &fileData) != 0);
        FindClose(hFind);
    }
    else
    {
        wcout << L"Failed to find files in folder: " << folderName << endl;
    }

    Animation newAnim;
    newAnim.textures = textures;
    newAnim.time = time;

    static int identity = 1000;
    newAnim.identity = identity;
    identity++;
    return newAnim;
}

void AnimationManager::ReleaseAnimation(Animation& anim)
{
    for (IDirect3DTexture9* tex : anim.textures)
    {
        tex->Release();
    }
    anim.textures.clear();
}

IDirect3DTexture9* AnimationManager::LoadTexture(const wstring& path)
{
    IDirect3DTexture9* texture;
    D3DXIMAGE_INFO imageInfo;
    WCHAR buffer[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, buffer);
    wstring currentDirectory = buffer;
    wstring searchPath = currentDirectory + L"\\" + path + L".bmp";
    if (FAILED(D3DXCreateTextureFromFileEx(
        MainFrame::GetInstance()->GetDevice() ,
        searchPath.c_str(),
        D3DX_DEFAULT,
        D3DX_DEFAULT,
        D3DX_DEFAULT,
        0,
        D3DFMT_UNKNOWN,
        D3DPOOL_MANAGED,
        D3DX_DEFAULT,
		D3DX_DEFAULT,
		TRANSCOLOR,
		&imageInfo,
		NULL,
		&texture)))
	{
		wcout << L"Failed to load texture: " << searchPath << endl;
		return NULL;
    }
    return texture;
}

void AnimationManager::ReleaseTexture(IDirect3DTexture9* tex)
{
    tex->Release();
}
