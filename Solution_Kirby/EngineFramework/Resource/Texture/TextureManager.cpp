#include "pch.h"
#include "TextureManager.h"
#include "Resource/AssetDatabase.h"

#include <algorithm>
#include <cwctype>
#include <unordered_set>

namespace
{
    bool FileExists(const std::string& filepath)
    {
        if (filepath.empty())
        {
            return false;
        }

        const DWORD attributes = GetFileAttributesA(filepath.c_str());
        return attributes != INVALID_FILE_ATTRIBUTES
            && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
    }

    bool FileExists(const std::wstring& filepath)
    {
        if (filepath.empty())
        {
            return false;
        }

        const DWORD attributes = GetFileAttributesW(filepath.c_str());
        return attributes != INVALID_FILE_ATTRIBUTES
            && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
    }

    std::string JoinPath(const std::string& lhs, const std::string& rhs)
    {
        if (lhs.empty())
        {
            return rhs;
        }
        if (lhs.back() == '\\' || lhs.back() == '/')
        {
            return lhs + rhs;
        }
        return lhs + "\\" + rhs;
    }

    std::string GetExeDirectory()
    {
        char path[MAX_PATH] = { 0 };
        GetModuleFileNameA(NULL, path, MAX_PATH);
        std::string exePath = path;
        const size_t pos = exePath.find_last_of("\\/");
        return pos == std::string::npos ? std::string(".") : exePath.substr(0, pos);
    }

    std::string GetFullPath(const std::string& path)
    {
        char fullPath[MAX_PATH] = { 0 };
        const DWORD length = GetFullPathNameA(path.c_str(), MAX_PATH, fullPath, nullptr);
        if (length == 0 || length >= MAX_PATH)
        {
            return path;
        }
        return fullPath;
    }

    std::wstring GetFullPath(const std::wstring& path)
    {
        wchar_t fullPath[MAX_PATH] = { 0 };
        const DWORD length = GetFullPathNameW(path.c_str(), MAX_PATH, fullPath, nullptr);
        if (length == 0 || length >= MAX_PATH)
        {
            return path;
        }
        return fullPath;
    }

    std::string NormalizeCacheKey(std::string path)
    {
        path = GetFullPath(path);
        std::replace(path.begin(), path.end(), '\\', '/');
        std::transform(path.begin(), path.end(), path.begin(),
            [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
        return path;
    }

    std::wstring NormalizeCacheKey(std::wstring path)
    {
        path = GetFullPath(path);
        std::replace(path.begin(), path.end(), L'\\', L'/');
        std::transform(path.begin(), path.end(), path.begin(),
            [](wchar_t ch) { return static_cast<wchar_t>(std::towlower(ch)); });
        return path;
    }

    std::string ResolveAssetPath(const std::string& filepath)
    {
        AssetDatabase* assetDatabase = AssetDatabase::GetInstance();
        if (assetDatabase == nullptr)
        {
            return filepath;
        }

        const AssetInfo* asset = assetDatabase->FindByKey(filepath);
        if (asset != nullptr && !asset->path.empty())
        {
            return asset->path;
        }

        return filepath;
    }

    std::string ResolveExistingTexturePath(const std::string& filepath)
    {
        const std::string resolvedAssetPath = ResolveAssetPath(filepath);
        if (FileExists(resolvedAssetPath))
        {
            return resolvedAssetPath;
        }

        std::string relativePath = filepath;
        std::replace(relativePath.begin(), relativePath.end(), '/', '\\');

        AssetDatabase* assetDatabase = AssetDatabase::GetInstance();
        if (assetDatabase != nullptr && !assetDatabase->GetRootPath().empty())
        {
            const std::string resourcePath = JoinPath(assetDatabase->GetRootPath(), relativePath);
            if (FileExists(resourcePath))
            {
                return resourcePath;
            }
        }

        const std::string exeDirectory = GetExeDirectory();
        const std::vector<std::string> candidates =
        {
            JoinPath(exeDirectory, relativePath),
            GetFullPath(JoinPath(exeDirectory, "..\\..\\Resources\\" + relativePath)),
            GetFullPath(JoinPath(exeDirectory, "..\\..\\" + relativePath)),
            GetFullPath(JoinPath(exeDirectory, "..\\..\\Debug\\" + relativePath))
        };

        for (std::vector<std::string>::const_iterator itr = candidates.begin(); itr != candidates.end(); itr++)
        {
            if (FileExists(*itr))
            {
                return *itr;
            }
        }

        return std::string();
    }

    bool CanReadTextureInfo(const std::string& filepath)
    {
        if (filepath.empty())
        {
            return false;
        }

        D3DXIMAGE_INFO imageInfo = {};
        return SUCCEEDED(D3DXGetImageInfoFromFileA(filepath.c_str(), &imageInfo));
    }

    void ReleaseTextureMap(std::unordered_map<std::string, IDirect3DTexture9*>& textureMap,
        std::unordered_set<IDirect3DTexture9*>& releasedTextures)
    {
        for (std::unordered_map<std::string, IDirect3DTexture9*>::iterator itr = textureMap.begin(); itr != textureMap.end(); itr++)
        {
            IDirect3DTexture9* texture = itr->second;
            itr->second = nullptr;
            if (texture != nullptr && releasedTextures.insert(texture).second)
            {
                texture->Release();
            }
        }
        textureMap.clear();
    }

    void ReleaseTextureMap(std::unordered_map<std::wstring, IDirect3DTexture9*>& textureMap,
        std::unordered_set<IDirect3DTexture9*>& releasedTextures)
    {
        for (std::unordered_map<std::wstring, IDirect3DTexture9*>::iterator itr = textureMap.begin(); itr != textureMap.end(); itr++)
        {
            IDirect3DTexture9* texture = itr->second;
            itr->second = nullptr;
            if (texture != nullptr && releasedTextures.insert(texture).second)
            {
                texture->Release();
            }
        }
        textureMap.clear();
    }
}

TextureManager::TextureManager()
{
}

TextureManager::~TextureManager()
{
}

IDirect3DTexture9* TextureManager::GetTexture(const std::string& filepath)
{
    const std::string resolvedPath = ResolveExistingTexturePath(filepath);
    if (resolvedPath.empty())
    {
        return nullptr;
    }

    const std::string cacheKey = NormalizeCacheKey(resolvedPath);
    std::unordered_map<std::string, IDirect3DTexture9*>::iterator cached = m_textureMap.find(cacheKey);
    if (cached != m_textureMap.end())
    {
        return cached->second;
    }

    MainFrame* mainFrame = MainFrame::GetInstance();
    if (mainFrame == nullptr || mainFrame->GetDevice() == nullptr || !CanReadTextureInfo(resolvedPath))
    {
        return nullptr;
    }

    IDirect3DTexture9* texture = nullptr;
    if (SUCCEEDED(D3DXCreateTextureFromFileA(mainFrame->GetDevice(), resolvedPath.c_str(), &texture)))
    {
        m_textureMap[cacheKey] = texture;
        return texture;
    }

    return nullptr;
}

IDirect3DTexture9* TextureManager::GetTexture(const std::string& filepath, bool useMagentaColorKey)
{
    if (!useMagentaColorKey)
    {
        return GetTexture(filepath);
    }

    const std::string resolvedPath = ResolveExistingTexturePath(filepath);
    if (resolvedPath.empty())
    {
        return nullptr;
    }

    const std::string cacheKey = NormalizeCacheKey(resolvedPath);
    std::unordered_map<std::string, IDirect3DTexture9*>::iterator cached = m_colorKeyTextureMap.find(cacheKey);
    if (cached != m_colorKeyTextureMap.end())
    {
        return cached->second;
    }

    MainFrame* mainFrame = MainFrame::GetInstance();
    if (mainFrame == nullptr || mainFrame->GetDevice() == nullptr || !CanReadTextureInfo(resolvedPath))
    {
        return nullptr;
    }

    IDirect3DTexture9* texture = nullptr;
    if (SUCCEEDED(D3DXCreateTextureFromFileExA(
        mainFrame->GetDevice(),
        resolvedPath.c_str(),
        D3DX_DEFAULT,
        D3DX_DEFAULT,
        D3DX_DEFAULT,
        0,
        D3DFMT_UNKNOWN,
        D3DPOOL_MANAGED,
        D3DX_DEFAULT,
        D3DX_DEFAULT,
        TRANSCOLORDX,
        nullptr,
        nullptr,
        &texture)))
    {
        m_colorKeyTextureMap[cacheKey] = texture;
        return texture;
    }

    return nullptr;
}

IDirect3DTexture9* TextureManager::GetTexture(const std::wstring& filepath)
{
    if (!FileExists(filepath))
    {
        return nullptr;
    }

    const std::wstring cacheKey = NormalizeCacheKey(filepath);
    std::unordered_map<std::wstring, IDirect3DTexture9*>::iterator cached = m_textureMapW.find(cacheKey);
    if (cached != m_textureMapW.end())
    {
        return cached->second;
    }

    MainFrame* mainFrame = MainFrame::GetInstance();
    if (mainFrame == nullptr || mainFrame->GetDevice() == nullptr)
    {
        return nullptr;
    }

    const std::wstring resolvedPath = GetFullPath(filepath);
    IDirect3DTexture9* texture = nullptr;
    if (SUCCEEDED(D3DXCreateTextureFromFileW(mainFrame->GetDevice(), resolvedPath.c_str(), &texture)))
    {
        m_textureMapW[cacheKey] = texture;
        return texture;
    }

    return nullptr;
}

void TextureManager::GetTexture(const std::string& filepath, std::function<void(IDirect3DTexture9*)> func)
{
    if (func)
    {
        func(GetTexture(filepath));
    }
}

void TextureManager::GetTexture(const std::wstring& filepath, std::function<void(IDirect3DTexture9*)> func)
{
    if (func)
    {
        func(GetTexture(filepath));
    }
}

void TextureManager::ReleaseAllTextures()
{
    std::unordered_set<IDirect3DTexture9*> releasedTextures;
    ReleaseTextureMap(m_textureMap, releasedTextures);
    ReleaseTextureMap(m_colorKeyTextureMap, releasedTextures);
    ReleaseTextureMap(m_textureMapW, releasedTextures);
}

void TextureManager::ReleaseAllResources()
{
    ReleaseAllTextures();
}
