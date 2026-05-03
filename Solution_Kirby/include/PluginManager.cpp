
//#include "pch.h"
#include "PluginManager.h"
#include "Plugin.h"
#include <io.h>

namespace
{
	std::wstring Utf8ToWide(const std::string& text)
	{
		if (text.empty())
		{
			return std::wstring();
		}

		const int length = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
		if (length <= 0)
		{
			return std::wstring(text.begin(), text.end());
		}

		std::wstring wideText(static_cast<size_t>(length - 1), L'\0');
		if (!wideText.empty())
		{
			MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, &wideText[0], length);
		}
		return wideText;
	}

	std::string WideToUtf8(const wchar_t* text)
	{
		if (text == nullptr || *text == L'\0')
		{
			return std::string();
		}

		const int length = WideCharToMultiByte(CP_UTF8, 0, text, -1, nullptr, 0, nullptr, nullptr);
		if (length <= 0)
		{
			return std::string();
		}

		std::string utf8Text(static_cast<size_t>(length - 1), '\0');
		if (!utf8Text.empty())
		{
			WideCharToMultiByte(CP_UTF8, 0, text, -1, &utf8Text[0], length, nullptr, nullptr);
		}
		return utf8Text;
	}
}


PluginManager *PluginManager::g_pInstance = nullptr;

PluginManager::PluginManager(){}

PluginManager* PluginManager::GetInstance()
{
    if (g_pInstance == nullptr)
        g_pInstance = new PluginManager();

    return g_pInstance;
}

void PluginManager::ReleaseInstance()
{
	if (g_pInstance != nullptr)
	{
		delete g_pInstance;
		g_pInstance = nullptr;
	}
}

bool PluginManager::DirectLoadPlugin(const std::string p_fullpath)
{
    return LoadPlugin(p_fullpath);
}

void PluginManager::LoadPlugins(const std::string dir)
{
    std::vector<std::string> filenames;
	filenames = GetFileNames(dir);

    std::vector<std::string>::const_iterator it;
    for (it = filenames.begin(); it != filenames.end(); ++it)
    {
        const std::string &filename = *it;
		std::string fullName = dir + std::string("\\") + filename;
        LoadPlugin(fullName);
    }
}

std::vector<std::string> PluginManager::GetFileNames(const std::string dir) const
{
	std::string mask = dir + std::string("*.plug");
	std::vector<std::string> files;

	WIN32_FIND_DATAW FindData;
	HANDLE hFind;

	std::wstring wtmp = Utf8ToWide(mask);
	hFind = FindFirstFileW(wtmp.c_str(), &FindData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do {
			files.push_back(WideToUtf8(FindData.cFileName));
		} while (FindNextFileW(hFind, &FindData));
	}

	FindClose(hFind);
	return files;
}

bool PluginManager::LoadPlugin(const std::string filename)
{
    const std::wstring wideFileName = Utf8ToWide(filename);
    HMODULE hDll = ::LoadLibraryW(wideFileName.c_str());
    if (hDll == NULL)
    {
        LPVOID lpMsgBuf;
        ::FormatMessage( 
            FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_SYSTEM | 
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            GetLastError(),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // 기본 언어
            (LPTSTR) &lpMsgBuf,
            0,
            NULL 
        );

        ::OutputDebugStringA((char *)lpMsgBuf);

        LocalFree(lpMsgBuf);
        return false;
    }

    CREATEPLUGIN pFunc = (CREATEPLUGIN)::GetProcAddress(hDll, "CreatePlugin");
    if (pFunc == nullptr)
        return false;

    IPlugin *pPlugin = pFunc(*this);
    if (pPlugin == nullptr)
        return false;

    PluginInfo info;
    info.pPlugin = pPlugin;
    info.hDll    = hDll;

    m_plugins.push_back(info);

    return true;
}


void PluginManager::UnloadAllPlugins()
{
    std::vector<PluginInfo>::iterator it;
    for (it=m_plugins.begin(); it!=m_plugins.end(); ++it)
    {
        PluginInfo &info = *it;
            
        delete info.pPlugin;
        ::FreeLibrary (info.hDll);
    }

    m_plugins.clear();
}

size_t PluginManager::GetNumPlugins() const
{
    return m_plugins.size();
}

IPlugin *PluginManager::GetPlugin(size_t idx)
{
    if (idx < 0 || idx >= GetNumPlugins())
        return nullptr;

    return m_plugins[idx].pPlugin;
}
