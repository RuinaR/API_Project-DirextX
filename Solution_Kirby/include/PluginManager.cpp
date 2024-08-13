
//#include "pch.h"
#include "PluginManager.h"
#include "Plugin.h"
#include <io.h>


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

	WIN32_FIND_DATA FindData;
	HANDLE hFind;

	std::wstring wtmp;
	wtmp.assign(mask.begin(), mask.end());
	hFind = FindFirstFile(wtmp.c_str(), &FindData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do {
			std::string stmp;
			wtmp = std::wstring(FindData.cFileName);
			stmp.assign(wtmp.begin(), wtmp.end());
			files.push_back(stmp);
		} while (FindNextFile(hFind, &FindData));
	}

	FindClose(hFind);
	return files;
}

bool PluginManager::LoadPlugin(const std::string filename)
{
    HMODULE hDll = ::LoadLibraryA(filename.c_str());
    if (hDll == NULL)
    {
        LPVOID lpMsgBuf;
        ::FormatMessage( 
            FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_SYSTEM | 
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            GetLastError(),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
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