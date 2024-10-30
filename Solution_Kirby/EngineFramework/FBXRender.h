#pragma once
#include <string>
#include <vector>

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
    FbxTool m_tool;
    std::string m_fbxFileName;  // 로드할 FBX 파일 이름
    std::vector<Model> m_models;

    LogSystem m_logSystem;
    void SetWorldTransform(D3DXMATRIX& matWorld);

public:
    FBXRender(std::string name);
    void Initialize() override;
    void Release() override;
    void Start() override;
    void Update() override;
    void Render();
};