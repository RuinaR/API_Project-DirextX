#include "pch.h"
#include "EditorRenderFacade.h"

#include "ObjectManager.h"
#include "RenderManager.h"
#include "ImguiButton.h"
#include "ImageRender.h"
#include "UIElement.h"
#include "EditorObjectManagerBridge.h"
#include "EditorSelectionService.h"

namespace
{
	void DrawSelectedObjectMarker(const ImVec2& imageScreenPos)
	{
		GameObject* selectedObject = EditorSelectionService::GetSelectedObject();
		if (selectedObject == nullptr || selectedObject->GetDestroy())
		{
			return;
		}

		ImageRender* imageRender = nullptr;
		UIElement* uiElement = nullptr;
		vector<Component*>* components = selectedObject->GetComponentVec();
		if (components != nullptr)
		{
			for (vector<Component*>::iterator itr = components->begin(); itr != components->end(); ++itr)
			{
				Component* component = *itr;
				if (component == nullptr)
				{
					continue;
				}

				if (imageRender == nullptr)
				{
					imageRender = dynamic_cast<ImageRender*>(component);
				}
				if (uiElement == nullptr)
				{
					uiElement = dynamic_cast<UIElement*>(component);
				}

				if (imageRender != nullptr && uiElement != nullptr)
				{
					break;
				}
			}
		}

		ImDrawList* drawList = ImGui::GetBackgroundDrawList(ImGui::GetMainViewport());
		if (drawList == nullptr)
		{
			return;
		}

		if (uiElement != nullptr)
		{
			const D3DXVECTOR2 uiPosition = uiElement->GetPosition();
			const D3DXVECTOR2 uiSize = uiElement->GetSize();
			const float markerX = imageScreenPos.x + uiPosition.x + (uiSize.x * 0.5f);
			const float markerY = imageScreenPos.y + uiPosition.y + (uiSize.y * 0.5f);
			const float radius = min(max(max(uiSize.x, uiSize.y) * 0.12f, 10.0f), 18.0f);
			drawList->AddCircle(ImVec2(markerX, markerY), radius, IM_COL32(255, 0, 0, 255), 32, 2.0f);
			return;
		}

		if (imageRender != nullptr && imageRender->IsUIRender())
		{
			const D3DXVECTOR3 renderPosition = imageRender->GetRenderPosition();
			const D3DXVECTOR2 size = selectedObject->Size2D();
			const float markerX = imageScreenPos.x + renderPosition.x;
			const float markerY = imageScreenPos.y + renderPosition.y;
			const float radius = min(max(max(size.x, size.y) * 0.12f, 10.0f), 18.0f);
			drawList->AddCircle(ImVec2(markerX, markerY), radius, IM_COL32(255, 0, 0, 255), 32, 2.0f);
			return;
		}

		LPDIRECT3DDEVICE9 device = MainFrame::GetInstance() != nullptr ? MainFrame::GetInstance()->GetDevice() : nullptr;
		if (device == nullptr)
		{
			return;
		}

		D3DXVECTOR3 markerWorldPosition = selectedObject->Position();
		if (imageRender != nullptr)
		{
			markerWorldPosition = imageRender->GetRenderPosition();
		}

		D3DXMATRIX worldMatrix;
		D3DXMATRIX viewMatrix;
		D3DXMATRIX projectionMatrix;
		D3DXMatrixIdentity(&worldMatrix);
		if (FAILED(device->GetTransform(D3DTS_VIEW, &viewMatrix)) || FAILED(device->GetTransform(D3DTS_PROJECTION, &projectionMatrix)))
		{
			return;
		}

		D3DVIEWPORT9 viewport = {};
		if (FAILED(device->GetViewport(&viewport)))
		{
			return;
		}

		D3DXVECTOR3 projectedCenter;
		D3DXVec3Project(&projectedCenter, &markerWorldPosition, &viewport, &projectionMatrix, &viewMatrix, &worldMatrix);

		const float markerX = imageScreenPos.x + projectedCenter.x;
		const float markerY = imageScreenPos.y + projectedCenter.y;
		const float radius = 14.0f;
		drawList->AddCircle(ImVec2(markerX, markerY), radius, IM_COL32(255, 0, 0, 255), 32, 2.0f);
	}

	void DrawFrameInfoWindow()
	{
		ImGuiIO& io = ImGui::GetIO();
		ImGui::Begin("Frame");
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		ImGui::End();
	}
}

void EditorRenderFacade::DrawOverlay(RenderManager& renderManager)
{
	const D3DXVECTOR2 gameViewScreenPos = renderManager.GetGameViewScreenPos();
	DrawSelectedObjectMarker(ImVec2(gameViewScreenPos.x, gameViewScreenPos.y));

	const vector<ImguiButton*>* buttons = renderManager.GetRegisteredButtons();
	if (buttons != nullptr && !buttons->empty())
	{
		ImGui::Begin("Button");
		for (vector<ImguiButton*>::const_iterator itr = buttons->begin(); itr != buttons->end(); ++itr)
		{
			ImGui::SameLine();
			(*itr)->UpdateRender();
		}
		ImGui::End();
	}

	EditorObjectManagerBridge::DrawManagerWindows();

	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (objectManager != nullptr)
	{
		const bool isMouseCapturedByEditor = ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantCaptureMouse;
		objectManager->SetMouseInteractionBlocked(isMouseCapturedByEditor);
	}

	DrawFrameInfoWindow();
}
