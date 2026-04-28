#include "pch.h"
#include "AnimationRender.h"

namespace
{
    wstring GetExecutableDirectoryW()
    {
        wchar_t path[MAX_PATH] = { 0 };
        GetModuleFileNameW(NULL, path, MAX_PATH);
        wstring executePath = path;
        return executePath.substr(0, executePath.find_last_of(L"\\/"));
    }

    string GetExecutableDirectoryA()
    {
        char path[MAX_PATH] = { 0 };
        GetModuleFileNameA(NULL, path, MAX_PATH);
        string executePath = path;
        return executePath.substr(0, executePath.find_last_of("\\/"));
    }

    bool FileExistsA(const string& path)
    {
        DWORD attributes = GetFileAttributesA(path.c_str());
        return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
    }

    bool DirectoryExistsW(const wstring& path)
    {
        DWORD attributes = GetFileAttributesW(path.c_str());
        return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
    }

    wstring ResolveAssetDirectory(const wstring& path)
    {
        wstring currentDirectory = GetExecutableDirectoryW();
        wstring searchPath = currentDirectory + L"\\" + path;
        if (DirectoryExistsW(searchPath))
        {
            return searchPath;
        }

        wstring debugAssetPath = currentDirectory + L"\\..\\..\\Debug\\" + path;
        if (DirectoryExistsW(debugAssetPath))
        {
            return debugAssetPath;
        }

        wstring solutionAssetPath = currentDirectory + L"\\..\\..\\" + path;
        if (DirectoryExistsW(solutionAssetPath))
        {
            return solutionAssetPath;
        }

        return searchPath;
    }

    string ResolveTexturePath(const string& path)
    {
        string currentDirectory = GetExecutableDirectoryA();
        string searchPath = currentDirectory + "\\" + path;
        if (FileExistsA(searchPath))
        {
            return searchPath;
        }

        string debugAssetPath = currentDirectory + "\\..\\..\\Debug\\" + path;
        if (FileExistsA(debugAssetPath))
        {
            return debugAssetPath;
        }

        string solutionAssetPath = currentDirectory + "\\..\\..\\" + path;
        if (FileExistsA(solutionAssetPath))
        {
            return solutionAssetPath;
        }

        return searchPath;
    }
}

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
    wstring currentDirectory = ResolveAssetDirectory(folderName);
    vector<IDirect3DTexture9*> textures;
    wstring searchPath = currentDirectory + L"\\*.*";
    WIN32_FIND_DATAW fileData;
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &fileData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            wstring fileName = fileData.cFileName;
            if (IsImageFile(fileName)) {
                wstring filePath = currentDirectory + L"\\" + fileName;
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
    string searchPath = ResolveTexturePath(path);
    texture = TextureManager::GetInstance()->GetTexture(searchPath);
    if (!texture)
	{
		cout << "Failed to load texture: " << searchPath << endl;
		return NULL;
	}
	return texture;
}

void AnimationManager::LoadTexture(const string& path, std::function<void(IDirect3DTexture9*)> func)
{
    string searchPath = ResolveTexturePath(path);
    TextureManager::GetInstance()->GetTexture(searchPath, func);
}

void AnimationManager::LoadTexture(const string& path, ImageRender* ir)
{
    string searchPath = ResolveTexturePath(path);
    TextureManager::GetInstance()->GetTexture(searchPath, bind(&ImageRender::LoadTextureCallback, ir, std::placeholders::_1));
}
