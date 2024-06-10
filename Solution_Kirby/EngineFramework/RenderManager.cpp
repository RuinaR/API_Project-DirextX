#include "pch.h"
#include "RenderManager.h"
#include "ImageRender.h"
#include "ColorButton.h"
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

void RenderManager::ResisterBtn(ColorButton* btn)
{
	m_btnVec->push_back(btn);
}

void RenderManager::UnresisterBtn(ColorButton* btn)
{
	for (vector<ColorButton*>::iterator itr = m_btnVec->begin(); itr != m_btnVec->end(); itr++)
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
	m_btnVec = new vector<ColorButton*>();
	m_debugVec = new vector<DebugRender*>();
}

void RenderManager::Update()
{
	LPDIRECT3DDEVICE9 device = MainFrame::GetInstance()->GetDevice();

	device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	if (SUCCEEDED(device->BeginScene()))
	{	
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
		for (vector<ColorButton*>::iterator itr = m_btnVec->begin(); itr != m_btnVec->end(); itr++)
		{
			(*itr)->UpdateRender();
		}
		device->EndScene();
	}
	device->Present(NULL, NULL, NULL, NULL);
}

void RenderManager::Release()
{
	m_transVec->clear();
	delete m_transVec;
	m_noTransVec->clear();
	delete m_noTransVec;
	m_btnVec->clear();
	delete m_btnVec;
	m_debugVec->clear();
	delete m_debugVec;
}
