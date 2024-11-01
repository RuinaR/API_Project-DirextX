#include "pch.h"
#include "AnimationRender.h"
#include <atlconv.h>

bool AnimationManager::IsImageFile(const wstring& filename)
{
    size_t dotIndex = filename.find_last_of('.');
    if (dotIndex != string::npos)
    {
        wstring extension = filename.substr(dotIndex + 1);
        transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
        return (extension == L"bmp" || extension == L"jpg" || extension == L"jpeg" || extension == L"png");
    }
    return false;
}

Animation AnimationManager::LoadAnimation(const wstring& folderName, float time) {
    wchar_t wpath[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, wpath, MAX_PATH);
    //USES_CONVERSION;
    std::wstring executepath = wpath;
    executepath = executepath.substr(0, executepath.find_last_of(L"\\/"));
    wstring currentDirectory = executepath;
    vector<IDirect3DTexture9*> textures;
    wstring searchPath = currentDirectory + L"\\" + folderName + L"\\*.*";
    WIN32_FIND_DATAW fileData;
    D3DXIMAGE_INFO imageInfo;
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &fileData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            wstring fileName = fileData.cFileName;
            if (IsImageFile(fileName)) {
                wstring filePath = currentDirectory + L"\\" + folderName + L"\\" + fileName;
                cout << "텍스처 로딩 중: " << filePath.c_str() << endl; // 디버그 출력

                IDirect3DTexture9* texture = nullptr;
                texture = TextureManager::GetInstance()->GetTexture(filePath);

                if (texture) {
                    textures.push_back(texture);
                }
                else {
                    cout << "텍스처 로딩 실패: " << filePath.c_str() << endl;
                }
            }
        } while (FindNextFileW(hFind, &fileData) != 0);
        FindClose(hFind);
    }
    else {
        wcout << L"폴더에서 파일 찾기 실패: " << folderName << endl;
    }

    Animation newAnim;
    newAnim.textures = textures;
    newAnim.time = time;

    static int identity = 1000;
    newAnim.identity = identity;
    identity++;
    return newAnim;
}

IDirect3DTexture9* AnimationManager::LoadTexture(const string& path)
{
    IDirect3DTexture9* texture;
    D3DXIMAGE_INFO imageInfo;
    char wpath[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, wpath, MAX_PATH);
    //USES_CONVERSION;
    string executepath = wpath;
    executepath = executepath.substr(0, executepath.find_last_of("\\/"));
    string currentDirectory = executepath;
    string searchPath = currentDirectory + "\\" + path;
    texture = TextureManager::GetInstance()->GetTexture(searchPath);
    if (!texture)
	{
		cout << "Failed to load texture: " << searchPath << endl;
		return NULL;
	}
	return texture;
}
