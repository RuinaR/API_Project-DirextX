
#ifndef PLUGINMANAGER_H_
#define PLUGINMANAGER_H_

#define WIN32_LEAN_AND_MEAN             // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.
// Windows ��� ����
#include <windows.h>

//#include "../ExamPlugin/pch.h"
#include <vector>
#include <string>

class IPlugin;
class PluginManager
{
public:
    static PluginManager *GetInstance();
	static void ReleaseInstance();

public:
    void LoadPlugins(const std::string dir);
    bool DirectLoadPlugin(const std::string p_fullpath);
    void UnloadAllPlugins();
	size_t GetNumPlugins() const;
    IPlugin *GetPlugin(size_t idx);

private:
    PluginManager();
	std::vector<std::string> GetFileNames(const std::string dir) const;
    bool LoadPlugin(const std::string filename);

    struct PluginInfo
    {
        IPlugin *pPlugin;
        HMODULE hDll;
    };

    std::vector<PluginInfo> m_plugins;
    static PluginManager *g_pInstance;
};

#endif