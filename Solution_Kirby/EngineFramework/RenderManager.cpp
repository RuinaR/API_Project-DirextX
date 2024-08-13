#include "pch.h"
#include "RenderManager.h"
#include "ImageRender.h"
#include "Button.h"
RenderManager* RenderManager::m_Pthis = nullptr;

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

void RenderManager::Resister(ImageRender* ir)
{
	if (ir->IsTrans())
	{
		auto it = std::lower_bound(m_transVec->begin(), m_transVec->end(), ir,
			[](ImageRender* lhs, ImageRender* rhs) {
				return lhs->GetZ() > rhs->GetZ();
			});
		m_transVec->insert(it, ir);
	}
	else
	{
		auto it = std::lower_bound(m_noTransVec->begin(), m_noTransVec->end(), ir,
			[](ImageRender* lhs, ImageRender* rhs) {
				return lhs->GetZ() > rhs->GetZ();
			});
		m_noTransVec->insert(it, ir);
	}
}

void RenderManager::Unresister(ImageRender* ir)
{
	if (ir->IsTrans())
	{
		for (vector<ImageRender*>::iterator itr = m_transVec->begin(); itr != m_transVec->end(); itr++)
		{
			if ((*itr) == ir)
			{
				m_transVec->erase(itr);
				return;
			}
		}
	}
	else
	{
		for (vector<ImageRender*>::iterator itr = m_noTransVec->begin(); itr != m_noTransVec->end(); itr++)
		{
			if ((*itr) == ir)
			{
				m_noTransVec->erase(itr);
				return;
			}
		}
	}
}

void RenderManager::ResisterBtn(Button* btn)
{
	m_btnVec->push_back(btn);
}

void RenderManager::UnresisterBtn(Button* btn)
{
	for (vector<Button*>::iterator itr = m_btnVec->begin(); itr != m_btnVec->end(); itr++)
	{
		if ((*itr) == btn)
		{
			m_btnVec->erase(itr);
			return;
		}
	}
}

void RenderManager::ResisterDebug(DebugRender* db)
{
	m_debugVec->push_back(db);
}

void RenderManager::UnresisterDebug(DebugRender* db)
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

void RenderManager::Initialize()
{
	m_transVec = new vector<ImageRender*>();
	m_noTransVec = new vector<ImageRender*>();
	m_btnVec = new vector<Button*>();
	m_debugVec = new vector<DebugRender*>();

	LPDIRECT3DDEVICE9 device = MainFrame::GetInstance()->GetDevice();
	device->CreateTexture(
		DRAWWINDOWW,
		DRAWWINDOWH,
		1,
		D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT,
		&renderTargetTexture,
		nullptr);
}

void RenderManager::EditUpdate()
{
	LPDIRECT3DSURFACE9 renderTargetSurface = nullptr;
	LPDIRECT3DSURFACE9 originalRenderTarget = nullptr;
	LPDIRECT3DSURFACE9 originalDepthStencil = nullptr;

	LPDIRECT3DDEVICE9 device = MainFrame::GetInstance()->GetDevice();
	device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	
	renderTargetTexture->GetSurfaceLevel(0, &renderTargetSurface);

	device->GetRenderTarget(0, &originalRenderTarget);
	device->GetDepthStencilSurface(&originalDepthStencil);
	device->SetRenderTarget(0, renderTargetSurface);

	device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	if (SUCCEEDED(device->BeginScene()))
	{	
		device->SetRenderState(D3DRS_ZENABLE, TRUE);
		device->SetRenderState(D3DRS_LIGHTING, FALSE);
		device->SetFVF(D3DFVF_CUSTOMVERTEX);
		device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE); 
		for (vector<ImageRender*>::iterator itr = m_noTransVec->begin(); itr != m_noTransVec->end(); itr++)
		{
			(*itr)->Render();
		}	
		device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		for (vector<ImageRender*>::iterator itr = m_transVec->begin(); itr != m_transVec->end(); itr++)
		{
			(*itr)->Render();
		}
		//DebugRender
		for (vector<DebugRender*>::iterator itr = m_debugVec->begin(); itr != m_debugVec->end(); itr++)
		{
			(*itr)->DebugRenderUpdate();
		}
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

		ImGui::SetNextWindowSize(ImVec2(DRAWWINDOWW, DRAWWINDOWH), ImGuiCond_Once);
		ImGui::Begin(WINDOWTEXT, nullptr, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize);

		//button/imgui
		for (vector<Button*>::iterator itr = m_btnVec->begin(); itr != m_btnVec->end(); itr++)
		{
			ImGui::SameLine();
			(*itr)->UpdateRender();
		}
		
		ImVec2 windowSize = ImGui::GetContentRegionAvail();
		//Game
		ImGui::Image((void*)renderTargetTexture, windowSize);
		m_winPos = ImGui::GetWindowPos();
		ImGui::End();

		//ObjMgr
		ObjectManager::GetInstance()->ImguiUpdate();

		//FrameCheck
		ImGuiIO& io = ImGui::GetIO();
		ImGui::Begin("Frame");
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		ImGui::End();

		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
		ImGui::EndFrame();
		device->SetRenderState(D3DRS_ZENABLE, FALSE);
		device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

		device->EndScene();
	}
	
	device->Present(NULL, NULL, NULL, NULL);
}

void RenderManager::GameUpdate()
{
	LPDIRECT3DDEVICE9 device = MainFrame::GetInstance()->GetDevice();
	device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	if (SUCCEEDED(device->BeginScene()))
	{
		device->SetRenderState(D3DRS_ZENABLE, TRUE);
		device->SetRenderState(D3DRS_LIGHTING, FALSE);
		device->SetFVF(D3DFVF_CUSTOMVERTEX);
		device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		for (vector<ImageRender*>::iterator itr = m_noTransVec->begin(); itr != m_noTransVec->end(); itr++)
		{
			(*itr)->Render();
		}
		device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		for (vector<ImageRender*>::iterator itr = m_transVec->begin(); itr != m_transVec->end(); itr++)
		{
			(*itr)->Render();
		}
		
		device->SetTexture(0, nullptr);
		device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
		device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
		device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
		device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

		//imgui
		//ImGui::SetNextWindowSize(ImVec2(DRAWWINDOWW, DRAWWINDOWH), ImGuiCond_Once);
		//ImGui::Begin(WINDOWTEXT, nullptr, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize);

		//button/imgui
		ImGui::Begin("Button");
		for (vector<Button*>::iterator itr = m_btnVec->begin(); itr != m_btnVec->end(); itr++)
		{
			ImGui::SameLine();
			(*itr)->UpdateRender();
		}

		//ImVec2 windowSize = ImGui::GetContentRegionAvail();
		//Game
		//ImGui::Image((void*)renderTargetTexture, windowSize);
		//m_winPos = ImGui::GetWindowPos();
		ImGui::End();

		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
		ImGui::EndFrame();
		device->SetRenderState(D3DRS_ZENABLE, FALSE);
		device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

		device->EndScene();
	}

	device->Present(NULL, NULL, NULL, NULL);
}

void RenderManager::Release()
{
	m_transVec->clear();
	m_noTransVec->clear();
	m_btnVec->clear();
	m_debugVec->clear();

	delete m_transVec;
	delete m_noTransVec;
	delete m_btnVec;
	delete m_debugVec;

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
