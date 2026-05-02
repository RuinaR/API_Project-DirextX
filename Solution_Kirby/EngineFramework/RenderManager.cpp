#include "pch.h"
#include "RenderManager.h"
#include "ImageRender.h"
#include "ImguiButton.h"
#include "UIElement.h"
#include "Camera.h"
#include "ObjectManager.h"

RenderManager* RenderManager::m_Pthis = nullptr;

int RenderManager::FrameCount = 0;

namespace
{
	void RemoveImageRenderFromQueue(vector<ImageRender*>* queue, ImageRender* ir)
	{
		if (queue == nullptr || ir == nullptr)
		{
			return;
		}

		for (vector<ImageRender*>::iterator itr = queue->begin(); itr != queue->end();)
		{
			if ((*itr) == ir)
			{
				itr = queue->erase(itr);
			}
			else
			{
				itr++;
			}
		}
	}

	bool CanRenderComponent(Component* component)
	{
		if (component == nullptr || component->GetGameObject() == nullptr)
		{
			return false;
		}

		GameObject* obj = component->GetGameObject();
		return obj->GetActive() && !obj->GetDestroy();
	}

	void EndImGuiFrameSafely()
	{
		if (ImGui::GetCurrentContext() != nullptr)
		{
			ImGui::EndFrame();
		}
	}

	RECT CalculateAspectFitRect(float targetWidth, float targetHeight, float clientWidth, float clientHeight)
	{
		RECT rect = { 0, 0, static_cast<LONG>(clientWidth), static_cast<LONG>(clientHeight) };
		if (targetWidth <= 0.0f || targetHeight <= 0.0f || clientWidth <= 0.0f || clientHeight <= 0.0f)
		{
			return rect;
		}

		const float targetAspect = targetWidth / targetHeight;
		const float clientAspect = clientWidth / clientHeight;
		float drawWidth = clientWidth;
		float drawHeight = clientHeight;

		if (clientAspect > targetAspect)
		{
			drawHeight = clientHeight;
			drawWidth = drawHeight * targetAspect;
		}
		else
		{
			drawWidth = clientWidth;
			drawHeight = drawWidth / targetAspect;
		}

		const float offsetX = (clientWidth - drawWidth) * 0.5f;
		const float offsetY = (clientHeight - drawHeight) * 0.5f;
		rect.left = static_cast<LONG>(offsetX);
		rect.top = static_cast<LONG>(offsetY);
		rect.right = static_cast<LONG>(offsetX + drawWidth);
		rect.bottom = static_cast<LONG>(offsetY + drawHeight);
		return rect;
	}
}

void RenderManager::Create()
{
	if (!m_Pthis)
	{
		m_Pthis = new RenderManager();
	}
}

RenderManager* RenderManager::GetInstance()
{
	return m_Pthis;
}

void RenderManager::Destroy()
{
	if (m_Pthis)
	{
		delete m_Pthis;
		m_Pthis = nullptr;
	}
}

void RenderManager::Register(ImageRender* ir)
{
	if (ir == nullptr)
	{
		return;
	}

	Unregister(ir);

	if (ir->IsTrans())
	{
		m_transVec->push_back(ir);
	}
	else
	{
		m_noTransVec->push_back(ir);
	}
}

void RenderManager::Unregister(ImageRender* ir)
{
	UnregisterUI(ir);
	RemoveImageRenderFromQueue(m_transVec, ir);
	RemoveImageRenderFromQueue(m_noTransVec, ir);
}

void RenderManager::RegisterUI(ImageRender* ir)
{
	if (ir == nullptr)
	{
		return;
	}

	Unregister(ir);

	for (vector<UIRenderEntry>::iterator itr = m_uiRenderVec->begin(); itr != m_uiRenderVec->end(); itr++)
	{
		if (itr->render == ir)
		{
			itr->orderInLayer = ir->GetOrderInLayer();
			SortUIQueue();
			return;
		}
	}

	UIRenderEntry entry;
	entry.render = ir;
	entry.orderInLayer = ir->GetOrderInLayer();
	entry.registrationOrder = m_nextUIRenderRegistrationOrder++;
	m_uiRenderVec->push_back(entry);
	SortUIQueue();
}

void RenderManager::UnregisterUI(ImageRender* ir)
{
	if (m_uiRenderVec == nullptr || ir == nullptr)
	{
		return;
	}

	for (vector<UIRenderEntry>::iterator itr = m_uiRenderVec->begin(); itr != m_uiRenderVec->end();)
	{
		if (itr->render == ir)
		{
			itr = m_uiRenderVec->erase(itr);
		}
		else
		{
			itr++;
		}
	}
}

void RenderManager::RegisterUI(UIElement* element)
{
	for (vector<UIRenderEntry>::iterator itr = m_uiRenderVec->begin(); itr != m_uiRenderVec->end(); itr++)
	{
		if (itr->element == element)
		{
			itr->orderInLayer = element->GetOrderInLayer();
			SortUIQueue();
			return;
		}
	}

	UIRenderEntry entry;
	entry.element = element;
	entry.orderInLayer = element->GetOrderInLayer();
	entry.registrationOrder = m_nextUIRenderRegistrationOrder++;
	m_uiRenderVec->push_back(entry);
	SortUIQueue();
}

void RenderManager::UnregisterUI(UIElement* element)
{
	for (vector<UIRenderEntry>::iterator itr = m_uiRenderVec->begin(); itr != m_uiRenderVec->end(); itr++)
	{
		if (itr->element == element)
		{
			m_uiRenderVec->erase(itr);
			return;
		}
	}
}

void RenderManager::RefreshUIOrder(ImageRender* ir)
{
	for (vector<UIRenderEntry>::iterator itr = m_uiRenderVec->begin(); itr != m_uiRenderVec->end(); itr++)
	{
		if (itr->render == ir)
		{
			itr->orderInLayer = ir->GetOrderInLayer();
			break;
		}
	}

	SortUIQueue();
}

void RenderManager::RefreshUIOrder(UIElement* element)
{
	for (vector<UIRenderEntry>::iterator itr = m_uiRenderVec->begin(); itr != m_uiRenderVec->end(); itr++)
	{
		if (itr->element == element)
		{
			itr->orderInLayer = element->GetOrderInLayer();
			break;
		}
	}

	SortUIQueue();
}

float RenderManager::CalculateCameraDepth(ImageRender* ir)
{
	if (!ir || !ir->GetGameObject())
	{
		return 0.0f;
	}

	D3DXVECTOR3 objectPosition = ir->GetGameObject()->Position();
	D3DXVECTOR3 cameraPosition = Camera::GetInstance()->GetPos();
	D3DXVECTOR3 cameraForward = Camera::GetInstance()->GetForward();
	D3DXVECTOR3 objectToCamera = objectPosition - cameraPosition;
	return D3DXVec3Dot(&objectToCamera, &cameraForward);
}

void RenderManager::SortWorldRenderQueues()
{
	auto depthDesc = [this](ImageRender* lhs, ImageRender* rhs) {
		const float lhsDepth = CalculateCameraDepth(lhs);
		const float rhsDepth = CalculateCameraDepth(rhs);
		const float epsilon = 0.0001f;
		if (fabs(lhsDepth - rhsDepth) > epsilon)
		{
			return lhsDepth > rhsDepth;
		}
		return lhs->GetZ() > rhs->GetZ();
	};

	std::stable_sort(m_noTransVec->begin(), m_noTransVec->end(), depthDesc);
	std::stable_sort(m_transVec->begin(), m_transVec->end(), depthDesc);
}

void RenderManager::SortUIQueue()
{
	std::stable_sort(m_uiRenderVec->begin(), m_uiRenderVec->end(),
		[](const UIRenderEntry& lhs, const UIRenderEntry& rhs) {
			if (lhs.orderInLayer == rhs.orderInLayer)
			{
				return lhs.registrationOrder < rhs.registrationOrder;
			}
			return lhs.orderInLayer < rhs.orderInLayer;
		});
}

bool RenderManager::IsTopUIRenderAt(ImageRender* ir, const D3DXVECTOR2* point)
{
	if (!point)
	{
		return false;
	}

	for (vector<UIRenderEntry>::reverse_iterator itr = m_uiRenderVec->rbegin(); itr != m_uiRenderVec->rend(); itr++)
	{
		ImageRender* render = itr->render;
		if (!render || !render->IsRenderEnabled() || !CanRenderComponent(render))
		{
			continue;
		}

		GameObject* obj = render->GetGameObject();
		D3DXVECTOR3 position = render->GetRenderPosition();
		D3DXVECTOR2 size = obj->Size2D();
		RECT rect =
		{
			static_cast<LONG>(position.x - (size.x * 0.5f)),
			static_cast<LONG>(position.y - (size.y * 0.5f)),
			static_cast<LONG>(position.x + (size.x * 0.5f)),
			static_cast<LONG>(position.y + (size.y * 0.5f))
		};
		POINT winPoint =
		{
			static_cast<LONG>(point->x),
			static_cast<LONG>(point->y)
		};

		if (PtInRect(&rect, winPoint))
		{
			return render == ir;
		}
	}

	return false;
}

void RenderManager::Register(FBXRender* fbxr)
{
	for (vector<FBXRender*>::iterator itr = m_fbxVec->begin(); itr != m_fbxVec->end(); itr++)
	{
		if ((*itr) == fbxr)
		{
			return;
		}
	}

	m_fbxVec->push_back(fbxr);
}

void RenderManager::Unregister(FBXRender* fbxr)
{
	for (vector<FBXRender*>::iterator itr = m_fbxVec->begin(); itr != m_fbxVec->end(); itr++)
	{
		if ((*itr) == fbxr)
		{
			m_fbxVec->erase(itr);
			return;
		}
	}
}

void RenderManager::RegisterBtn(ImguiButton* btn)
{
	m_btnVec->push_back(btn);
}

void RenderManager::UnregisterBtn(ImguiButton* btn)
{
	for (vector<ImguiButton*>::iterator itr = m_btnVec->begin(); itr != m_btnVec->end(); itr++)
	{
		if ((*itr) == btn)
		{
			m_btnVec->erase(itr);
			return;
		}
	}
}

void RenderManager::RegisterDebug(DebugRender* db)
{
	m_debugVec->push_back(db);
}

void RenderManager::UnregisterDebug(DebugRender* db)
{
	for (vector<DebugRender*>::iterator itr = m_debugVec->begin(); itr != m_debugVec->end(); itr++)
	{
		if ((*itr) == db)
		{
			m_debugVec->erase(itr);
			return;
		}
	}
}

void RenderManager::SetColliderDebugVisible(bool visible)
{
	m_showColliderDebug = visible;
}

bool RenderManager::IsColliderDebugVisible() const
{
	return m_showColliderDebug;
}

void RenderManager::ClearImmediateDebugLines()
{
	if (m_immediateDebugLines != nullptr)
	{
		m_immediateDebugLines->clear();
	}
}

void RenderManager::AddImmediateDebugLine(const D3DXVECTOR3& start, const D3DXVECTOR3& end, D3DCOLOR color)
{
	if (m_immediateDebugLines == nullptr)
	{
		return;
	}

	DebugLine line;
	line.start = start;
	line.end = end;
	line.color = color;
	m_immediateDebugLines->push_back(line);
}

void RenderManager::AddImmediateDebugCross(const D3DXVECTOR3& center, float halfSize, D3DCOLOR color)
{
	AddImmediateDebugLine(
		D3DXVECTOR3(center.x - halfSize, center.y, center.z),
		D3DXVECTOR3(center.x + halfSize, center.y, center.z),
		color);
	AddImmediateDebugLine(
		D3DXVECTOR3(center.x, center.y - halfSize, center.z),
		D3DXVECTOR3(center.x, center.y + halfSize, center.z),
		color);
}

void RenderManager::Initialize()
{
	m_transVec = new vector<ImageRender*>();
	m_noTransVec = new vector<ImageRender*>();
	m_btnVec = new vector<ImguiButton*>();
	m_debugVec = new vector<DebugRender*>();
	m_fbxVec = new vector<FBXRender*>();
	m_uiRenderVec = new vector<UIRenderEntry>();
	m_immediateDebugLines = new vector<DebugLine>();
	m_nextUIRenderRegistrationOrder = 0;

	CreateRenderTargetTexture();
}

void RenderManager::ReleaseRenderTargetTexture()
{
	if (renderTargetTexture != nullptr)
	{
		renderTargetTexture->Release();
		renderTargetTexture = nullptr;
	}
}

bool RenderManager::CreateRenderTargetTexture()
{
	LPDIRECT3DDEVICE9 device = MainFrame::GetInstance() != nullptr ? MainFrame::GetInstance()->GetDevice() : nullptr;
	if (device == nullptr)
	{
		return false;
	}

	HRESULT hr = device->CreateTexture(
		DRAWWINDOWW,
		DRAWWINDOWH,
		1,
		D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT,
		&renderTargetTexture,
		nullptr);

	return SUCCEEDED(hr) && renderTargetTexture != nullptr;
}

void RenderManager::RenderImmediateDebugLines()
{
	if (m_immediateDebugLines == nullptr || m_immediateDebugLines->empty())
	{
		return;
	}

	LPDIRECT3DDEVICE9 device = MainFrame::GetInstance() != nullptr ? MainFrame::GetInstance()->GetDevice() : nullptr;
	if (device == nullptr)
	{
		return;
	}

	device->SetTexture(0, nullptr);
	device->SetFVF(D3DFVF_DEBUGVERTEX);
	device->SetRenderState(D3DRS_ZENABLE, FALSE);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

	for (vector<DebugLine>::iterator itr = m_immediateDebugLines->begin(); itr != m_immediateDebugLines->end(); ++itr)
	{
		DEBUGVERTEX vertices[] =
		{
			{ itr->start.x, itr->start.y, itr->start.z, itr->color },
			{ itr->end.x, itr->end.y, itr->end.z, itr->color },
		};
		device->DrawPrimitiveUP(D3DPT_LINELIST, 1, vertices, sizeof(DEBUGVERTEX));
	}

	device->SetRenderState(D3DRS_ZENABLE, TRUE);
}

void RenderManager::RenderSelectedObjectMarker(const ImVec2& imageScreenPos)
{
	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (objectManager == nullptr)
	{
		return;
	}

	GameObject* selectedObject = objectManager->GetSelectedObject();
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
	viewport.X = 0;
	viewport.Y = 0;
	viewport.Width = static_cast<DWORD>(DRAWWINDOWW);
	viewport.Height = static_cast<DWORD>(DRAWWINDOWH);
	viewport.MinZ = 0.0f;
	viewport.MaxZ = 1.0f;

	D3DXVECTOR3 projectedCenter;
	D3DXVec3Project(&projectedCenter, &markerWorldPosition, &viewport, &projectionMatrix, &viewMatrix, &worldMatrix);

	const float markerX = imageScreenPos.x + projectedCenter.x;
	const float markerY = imageScreenPos.y + projectedCenter.y;
	const float radius = 14.0f;
	drawList->AddCircle(ImVec2(markerX, markerY), radius, IM_COL32(255, 0, 0, 255), 32, 2.0f);
}

void RenderManager::EditUpdate()
{
	LPDIRECT3DSURFACE9 renderTargetSurface = nullptr;
	LPDIRECT3DSURFACE9 originalRenderTarget = nullptr;
	LPDIRECT3DSURFACE9 originalDepthStencil = nullptr;

	LPDIRECT3DDEVICE9 device = MainFrame::GetInstance()->GetDevice();
	if (renderTargetTexture == nullptr)
	{
		CreateRenderTargetTexture();
	}
	if (device == nullptr || renderTargetTexture == nullptr)
	{
		EndImGuiFrameSafely();
		return;
	}
	device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	
	renderTargetTexture->GetSurfaceLevel(0, &renderTargetSurface);
	if (renderTargetSurface == nullptr)
	{
		EndImGuiFrameSafely();
		return;
	}

	device->GetRenderTarget(0, &originalRenderTarget);
	device->GetDepthStencilSurface(&originalDepthStencil);
	device->SetRenderTarget(0, renderTargetSurface);

	device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	if (SUCCEEDED(device->BeginScene()))
	{	
		SortWorldRenderQueues();
		device->SetRenderState(D3DRS_ZENABLE, TRUE);
		device->SetRenderState(D3DRS_LIGHTING, FALSE);
		device->SetFVF(D3DFVF_CUSTOMVERTEX);
		device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE); 
		for (vector<ImageRender*>::iterator itr = m_noTransVec->begin(); itr != m_noTransVec->end(); itr++)
		{
			if (!CanRenderComponent(*itr))
				continue;
			(*itr)->Render();
		}	
		//FBX Render
		device->SetFVF(D3DFVF_CUSTOMVERTEX);
		device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
		device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
		device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

		for (vector<FBXRender*>::iterator itr = m_fbxVec->begin(); itr != m_fbxVec->end(); itr++)
		{
			if (!CanRenderComponent(*itr))
				continue;
			(*itr)->Render();
		}
		device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
		device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		for (vector<ImageRender*>::iterator itr = m_transVec->begin(); itr != m_transVec->end(); itr++)
		{
			if (!CanRenderComponent(*itr))
				continue;
			(*itr)->Render();
		}
		device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		//DebugRender
		if (m_showColliderDebug)
		{
			device->SetRenderState(D3DRS_ZENABLE, FALSE);
			for (vector<DebugRender*>::iterator itr = m_debugVec->begin(); itr != m_debugVec->end(); itr++)
			{
				(*itr)->DebugRenderUpdate();
			}
			device->SetRenderState(D3DRS_ZENABLE, TRUE);
		}
		RenderImmediateDebugLines();
		RenderUIQueue();
		device->SetTexture(0, nullptr);
		device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
		device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
		device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
		device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

		//imgui
		device->SetRenderTarget(0, originalRenderTarget);
		device->SetDepthStencilSurface(originalDepthStencil);
		originalRenderTarget->Release();
		originalDepthStencil->Release();

		ImGuiViewport* mainViewport = ImGui::GetMainViewport();
		ImVec2 imageScreenPos = mainViewport != nullptr ? mainViewport->Pos : ImVec2(0.0f, 0.0f);
		ImVec2 windowSize(DRAWWINDOWW, DRAWWINDOWH);
		ImVec2 imageMax(imageScreenPos.x + windowSize.x, imageScreenPos.y + windowSize.y);
		ImGui::GetBackgroundDrawList(mainViewport)->AddImage((void*)renderTargetTexture, imageScreenPos, imageMax);
		RenderSelectedObjectMarker(imageScreenPos);

		m_gameViewScreenPos = D3DXVECTOR2(imageScreenPos.x, imageScreenPos.y);
		POINT imageClientPos =
		{
			static_cast<LONG>(imageScreenPos.x),
			static_cast<LONG>(imageScreenPos.y)
		};
		ScreenToClient(WindowFrame::GetInstance()->GetHWND(), &imageClientPos);
		m_gameViewPos = D3DXVECTOR2(static_cast<float>(imageClientPos.x), static_cast<float>(imageClientPos.y));
		m_gameViewSize = D3DXVECTOR2(windowSize.x, windowSize.y);
		m_useScreenSpaceUIMouse = true;
		m_winPos = ImVec2(m_gameViewPos.x, m_gameViewPos.y);

		if (!m_btnVec->empty())
		{
			ImGui::Begin("Button");
			for (vector<ImguiButton*>::iterator itr = m_btnVec->begin(); itr != m_btnVec->end(); itr++)
			{
				ImGui::SameLine();
				(*itr)->UpdateRender();
			}
			ImGui::End();
		}

		//ObjMgr
		ObjectManager::GetInstance()->ImguiUpdate();

		//FrameCheck
		ImGuiIO& io = ImGui::GetIO();
		ImGui::Begin("Frame");
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		ImGui::End();

		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
		//ImGui::EndFrame();
		device->SetRenderState(D3DRS_ZENABLE, FALSE);
		device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);


		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		// Update and Render additional Platform Windows
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			// TODO for OpenGL: restore current GL context.
		}
		device->EndScene();
	}
	else
	{
		if (originalRenderTarget != nullptr)
		{
			originalRenderTarget->Release();
		}
		if (originalDepthStencil != nullptr)
		{
			originalDepthStencil->Release();
		}
		if (renderTargetSurface != nullptr)
		{
			renderTargetSurface->Release();
		}
		EndImGuiFrameSafely();
	}
	device->Present(NULL, NULL, NULL, NULL);
}

void RenderManager::GameUpdate()
{
	LPDIRECT3DDEVICE9 device = MainFrame::GetInstance()->GetDevice();
	RECT clientRect = {};
	if (WindowFrame::GetInstance() != nullptr)
	{
		GetClientRect(WindowFrame::GetInstance()->GetHWND(), &clientRect);
	}
	const float clientWidth = static_cast<float>(clientRect.right - clientRect.left);
	const float clientHeight = static_cast<float>(clientRect.bottom - clientRect.top);
	const float resolvedClientWidth = clientWidth > 0.0f ? clientWidth : static_cast<float>(DRAWWINDOWW);
	const float resolvedClientHeight = clientHeight > 0.0f ? clientHeight : static_cast<float>(DRAWWINDOWH);
	const RECT presentRect = CalculateAspectFitRect(
		static_cast<float>(DRAWWINDOWW),
		static_cast<float>(DRAWWINDOWH),
		resolvedClientWidth,
		resolvedClientHeight);
	m_gameViewPos = D3DXVECTOR2(static_cast<float>(presentRect.left), static_cast<float>(presentRect.top));
	m_gameViewScreenPos = m_gameViewPos;
	m_gameViewSize = D3DXVECTOR2(
		static_cast<float>(presentRect.right - presentRect.left),
		static_cast<float>(presentRect.bottom - presentRect.top));
	m_useScreenSpaceUIMouse = false;
	m_winPos = ImVec2(0.0f, 0.0f);

	if (device == nullptr)
	{
		EndImGuiFrameSafely();
		return;
	}

	device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	if (SUCCEEDED(device->BeginScene()))
	{
		D3DVIEWPORT9 previousViewport = {};
		device->GetViewport(&previousViewport);

		D3DVIEWPORT9 gameViewport = {};
		gameViewport.X = static_cast<DWORD>(max(0L, presentRect.left));
		gameViewport.Y = static_cast<DWORD>(max(0L, presentRect.top));
		gameViewport.Width = static_cast<DWORD>(max(1L, presentRect.right - presentRect.left));
		gameViewport.Height = static_cast<DWORD>(max(1L, presentRect.bottom - presentRect.top));
		gameViewport.MinZ = 0.0f;
		gameViewport.MaxZ = 1.0f;
		device->SetViewport(&gameViewport);

		SortWorldRenderQueues();
		device->SetRenderState(D3DRS_ZENABLE, TRUE);
		device->SetRenderState(D3DRS_LIGHTING, FALSE);
		device->SetFVF(D3DFVF_CUSTOMVERTEX);
		device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		for (vector<ImageRender*>::iterator itr = m_noTransVec->begin(); itr != m_noTransVec->end(); itr++)
		{
			if (!CanRenderComponent(*itr))
				continue;
			(*itr)->Render();
		}
		//FBX Render
		device->SetFVF(D3DFVF_CUSTOMVERTEX);
		device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
		device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		for (vector<FBXRender*>::iterator itr = m_fbxVec->begin(); itr != m_fbxVec->end(); itr++)
		{
			if (!CanRenderComponent(*itr))
				continue;
			(*itr)->Render();
		}
		device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
		device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		for (vector<ImageRender*>::iterator itr = m_transVec->begin(); itr != m_transVec->end(); itr++)
		{
			if (!CanRenderComponent(*itr))
				continue;
			(*itr)->Render();
		}
		device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		RenderUIQueue();
		device->SetTexture(0, nullptr);
		device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
		device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
		device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
		device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

		device->SetViewport(&previousViewport);
		device->EndScene();
		EndImGuiFrameSafely();
	}
	else
	{
		EndImGuiFrameSafely();
	}

	device->Present(NULL, NULL, NULL, NULL);
}

void RenderManager::Release()
{
	m_transVec->clear();
	m_noTransVec->clear();
	m_btnVec->clear();
	m_debugVec->clear();
	m_uiRenderVec->clear();
	if (m_immediateDebugLines != nullptr)
	{
		m_immediateDebugLines->clear();
	}

	delete m_transVec;
	delete m_noTransVec;
	delete m_btnVec;
	delete m_debugVec;
	delete m_fbxVec;
	delete m_uiRenderVec;
	delete m_immediateDebugLines;

	if (renderTargetTexture != nullptr)
	{
		renderTargetTexture->Release();
		renderTargetTexture = nullptr;
	}
}

ImVec2 RenderManager::GetWinPos()
{
	return m_winPos;
}

D3DXVECTOR2 RenderManager::ScreenToUICoordinate(const D3DXVECTOR2* screenPosition)
{
	if (!screenPosition)
	{
		return D3DXVECTOR2(0.0f, 0.0f);
	}

	D3DXVECTOR2 uiPosition =
	{
		screenPosition->x - m_gameViewPos.x,
		screenPosition->y - m_gameViewPos.y
	};

	if (m_gameViewSize.x > 0.0f && m_gameViewSize.y > 0.0f)
	{
		uiPosition.x *= static_cast<float>(DRAWWINDOWW) / m_gameViewSize.x;
		uiPosition.y *= static_cast<float>(DRAWWINDOWH) / m_gameViewSize.y;
	}

	return uiPosition;
}

D3DXVECTOR2 RenderManager::GetMouseUICoordinate()
{
	D3DXVECTOR2 mousePosition;

	if (m_useScreenSpaceUIMouse)
	{
		ImVec2 imguiMousePosition = ImGui::GetMousePos();
		mousePosition = D3DXVECTOR2(imguiMousePosition.x - m_gameViewScreenPos.x, imguiMousePosition.y - m_gameViewScreenPos.y);
	}
	else
	{
		D3DXVECTOR2 clientMousePosition = Mouse::GetInstance()->GetWinPos();
		mousePosition = D3DXVECTOR2(clientMousePosition.x - m_gameViewPos.x, clientMousePosition.y - m_gameViewPos.y);
	}

	if (m_gameViewSize.x > 0.0f && m_gameViewSize.y > 0.0f)
	{
		mousePosition.x *= static_cast<float>(DRAWWINDOWW) / m_gameViewSize.x;
		mousePosition.y *= static_cast<float>(DRAWWINDOWH) / m_gameViewSize.y;
	}

	return mousePosition;
}

bool RenderManager::IsUIMouseLeftDown()
{
	if (m_useScreenSpaceUIMouse)
	{
		return ImGui::IsMouseDown(ImGuiMouseButton_Left);
	}

	return Mouse::GetInstance()->IsLeftDown();
}

D3DXVECTOR2 RenderManager::GetGameViewPos()
{
	return m_gameViewPos;
}

D3DXVECTOR2 RenderManager::GetGameViewScreenPos()
{
	return m_gameViewScreenPos;
}

D3DXVECTOR2 RenderManager::GetGameViewSize()
{
	return m_gameViewSize;
}

bool RenderManager::IsUsingScreenSpaceUIMouse()
{
	return m_useScreenSpaceUIMouse;
}

void RenderManager::RenderUIQueue()
{
	if (m_uiRenderVec->empty())
	{
		return;
	}

	LPDIRECT3DDEVICE9 device = MainFrame::GetInstance()->GetDevice();
	D3DXMATRIX oldWorld;
	D3DXMATRIX oldView;
	D3DXMATRIX oldProjection;
	D3DXMATRIX identity;
	D3DXMATRIX uiProjection;

	device->GetTransform(D3DTS_WORLD, &oldWorld);
	device->GetTransform(D3DTS_VIEW, &oldView);
	device->GetTransform(D3DTS_PROJECTION, &oldProjection);
	D3DXMatrixIdentity(&identity);
	D3DXMatrixOrthoOffCenterLH(&uiProjection, 0.0f, static_cast<float>(DRAWWINDOWW), static_cast<float>(DRAWWINDOWH), 0.0f, -1.0f, 1.0f);

	device->SetTransform(D3DTS_WORLD, &identity);
	device->SetTransform(D3DTS_VIEW, &identity);
	device->SetTransform(D3DTS_PROJECTION, &uiProjection);
	device->SetFVF(D3DFVF_CUSTOMVERTEX);
	device->SetRenderState(D3DRS_ZENABLE, FALSE);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

	for (vector<UIRenderEntry>::iterator itr = m_uiRenderVec->begin(); itr != m_uiRenderVec->end(); itr++)
	{
		if (itr->render)
		{
			if (!CanRenderComponent(itr->render))
				continue;
			itr->render->Render();
		}
		else if (itr->element)
		{
			if (!CanRenderComponent(itr->element))
				continue;
			itr->element->RenderUI();
		}
	}

	device->SetTransform(D3DTS_WORLD, &oldWorld);
	device->SetTransform(D3DTS_VIEW, &oldView);
	device->SetTransform(D3DTS_PROJECTION, &oldProjection);
	device->SetRenderState(D3DRS_ZENABLE, TRUE);
}
