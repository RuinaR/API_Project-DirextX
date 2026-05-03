#include "pch.h"
#include "EditorObjectManagerBridge.h"

#include "EditorHierarchyWindow.h"
#include "EditorInspectorWindow.h"
#include "EditorSelectionService.h"
#include "Resource/ResourceBrowser.h"

namespace EditorObjectManagerBridge
{
	void DrawManagerWindows()
	{
		EditorSelectionService::EnsureObjectManagerHooksRegistered();
		EditorHierarchyWindow::Draw();
		if (WindowFrame::GetInstance() != nullptr &&
			WindowFrame::GetInstance()->GetRenderType() == RenderType::Edit)
		{
			ResourceBrowser::Draw();
		}
		EditorInspectorWindow::Draw();
	}
}
