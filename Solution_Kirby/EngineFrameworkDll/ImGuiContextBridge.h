#pragma once

#include "MainFrame.h"
#include "../Imgui/imgui/imgui.h"

inline void SyncImGuiContextForCurrentModule()
{
    MainFrame* mainFrame = MainFrame::GetInstance();
    if (mainFrame == nullptr)
    {
        return;
    }

    ImGuiContext* sharedContext = mainFrame->GetImGuiContext();
    if (sharedContext != nullptr)
    {
        ImGui::SetCurrentContext(sharedContext);
    }
}
