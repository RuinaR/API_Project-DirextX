#pragma once
#include <string>
#include <vector>
#include "Resource/FBXManager.h"

class LogSystem {
public:
    void AddLog(const std::string& log) {
        //logs.push_back(log);
    }

    void ShowLogWindow() 
    {
        /*if (ImGui::Begin("Log Window")) {
            for (const auto& log : logs) {
                ImGui::Text("%s", log.c_str());
            }
            ImGui::End();
        }*/
    }

private:
    std::vector<std::string> logs;
};

class FBXRender : public Component {
private:
    std::string m_fbxFileName;  // 로드할 FBX 파일 이름
    FbxResource* m_resource = nullptr;
    bool m_loaded = false;

    LogSystem m_logSystem;
    void SetWorldTransform(D3DXMATRIX* matWorld);

public:
    FBXRender(std::string name);
    void Initialize() override;
    void Release() override;
    void Start() override;
    void Update() override;
    void Render();
    bool ReloadModel(const std::string& fbxPath);
    const char* GetInspectorName() const override;
    void DrawInspector() override;
    const char* GetSerializableType() const override;
    std::string Serialize() const override;
    bool Deserialize(const std::string& componentJson) override;
};
