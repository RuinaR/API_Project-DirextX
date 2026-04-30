#include "pch.h"
#include "Mouse.h"
#include "Camera.h"

namespace
{
	D3DVIEWPORT9 CreateRayViewport()
	{
		D3DVIEWPORT9 viewport = {};
		viewport.X = 0;
		viewport.Y = 0;
		viewport.Width = static_cast<DWORD>(max(1, MainFrame::GetInstance()->GetViewportWidth()));
		viewport.Height = static_cast<DWORD>(max(1, MainFrame::GetInstance()->GetViewportHeight()));
		viewport.MinZ = 0.0f;
		viewport.MaxZ = 1.0f;
		return viewport;
	}

	Ray CreateFallbackRay()
	{
		Camera* camera = Camera::GetInstance();
		if (camera == nullptr)
		{
			return Ray();
		}

		return Ray(camera->GetPos(), camera->GetForward());
	}
}

Mouse* Mouse::mPthis = nullptr;

void Mouse::Create()
{
	if (!mPthis)
	{
		mPthis = new Mouse();
	}
}

Mouse* Mouse::GetInstance()
{
	return mPthis;
}

void Mouse::Destroy()
{
	if (mPthis)
	{
		delete mPthis;
		mPthis = nullptr;
	}
}

void Mouse::Initialize()
{
	this->mPos.x = 0;
	this->mPos.y = 0;
	this->mIsLeftDown = false;
	this->mIsRightDown = false;
	MainFrame::GetInstance()->GetDevice()->GetViewport(&m_viewport);
}

void Mouse::SetPos(int x, int y)
{
	this->mPos.x = x;
	this->mPos.y = y;
}

void Mouse::SetPos(D3DXVECTOR2 pos)
{
	this->mPos = pos;
}

D3DXVECTOR2 Mouse::GetDXPos()
{
	D3DXVECTOR2 gameViewPos = GetGameViewPos();
	return D3DXVECTOR2(
		gameViewPos.x - (DRAWWINDOWW * 0.5f),
		(DRAWWINDOWH * 0.5f) - gameViewPos.y);
}

D3DXVECTOR2 Mouse::GetWinPos()
{
	return this->mPos;
}

D3DXVECTOR2 Mouse::GetGameViewPos()
{
	D3DXVECTOR2 gameViewPos = RenderManager::GetInstance()->IsUsingScreenSpaceUIMouse()
		? RenderManager::GetInstance()->GetGameViewScreenPos()
		: RenderManager::GetInstance()->GetGameViewPos();
	D3DXVECTOR2 gameViewSize = RenderManager::GetInstance()->GetGameViewSize();
	D3DXVECTOR2 mousePos = mPos;
	if (RenderManager::GetInstance()->IsUsingScreenSpaceUIMouse())
	{
		ImVec2 imguiMousePos = ImGui::GetMousePos();
		mousePos = D3DXVECTOR2(imguiMousePos.x, imguiMousePos.y);
	}

	D3DXVECTOR2 localPos =
	{
		mousePos.x - gameViewPos.x,
		mousePos.y - gameViewPos.y
	};

	if (gameViewSize.x > 0.0f && gameViewSize.y > 0.0f)
	{
		localPos.x *= static_cast<float>(DRAWWINDOWW) / gameViewSize.x;
		localPos.y *= static_cast<float>(DRAWWINDOWH) / gameViewSize.y;
	}

	return localPos;
}

Ray Mouse::ScreenPointToRay()
{
	return ScreenPointToRay(GetGameViewPos());
}

Ray Mouse::ScreenPointToRay(const D3DXVECTOR2& screenPos)
{
	Camera* camera = Camera::GetInstance();
	MainFrame* mainFrame = MainFrame::GetInstance();
	if (camera == nullptr || mainFrame == nullptr)
	{
		return Ray();
	}

	const D3DVIEWPORT9 viewport = CreateRayViewport();
	const D3DXMATRIX& viewMatrix = camera->GetViewMatrix();
	const D3DXMATRIX& projectionMatrix = mainFrame->GetProjectionMatrix();
	D3DXMATRIX identityWorld;
	D3DXMatrixIdentity(&identityWorld);

	D3DXVECTOR3 nearScreenPoint(screenPos.x, screenPos.y, 0.0f);
	D3DXVECTOR3 farScreenPoint(screenPos.x, screenPos.y, 1.0f);
	D3DXVECTOR3 nearWorldPoint;
	D3DXVECTOR3 farWorldPoint;

	if (D3DXVec3Unproject(&nearWorldPoint, &nearScreenPoint, &viewport, &projectionMatrix, &viewMatrix, &identityWorld) == nullptr
		|| D3DXVec3Unproject(&farWorldPoint, &farScreenPoint, &viewport, &projectionMatrix, &viewMatrix, &identityWorld) == nullptr)
	{
		return CreateFallbackRay();
	}

	if (camera->GetProjectionMode() == CameraProjectionMode::Orthographic)
	{
		return Ray(nearWorldPoint, camera->GetForward());
	}

	D3DXVECTOR3 direction = farWorldPoint - nearWorldPoint;
	return Ray(nearWorldPoint, direction);
}

D3DXVECTOR3 Mouse::GetWorldPos(const D3DXVECTOR2* cameraPos, float z)
{
	if (!cameraPos)
	{
		return D3DXVECTOR3(0.0f, 0.0f, z);
	}

	D3DXVECTOR2 dxPos = GetDXPos();
	return D3DXVECTOR3(dxPos.x + cameraPos->x, dxPos.y + cameraPos->y, z);
}

void Mouse::SetLeftBtn(bool isDown)
{
	this->mIsLeftDown = isDown;
}

bool Mouse::IsLeftDown()
{
	if (RenderManager::GetInstance()->IsUsingScreenSpaceUIMouse())
	{
		return ImGui::IsMouseDown(ImGuiMouseButton_Left);
	}

	return this->mIsLeftDown;
}

void Mouse::SetRightBtn(bool isDown)
{
	this->mIsRightDown = isDown;
}

bool Mouse::IsRightDown()
{
	if (RenderManager::GetInstance()->IsUsingScreenSpaceUIMouse())
	{
		return ImGui::IsMouseDown(ImGuiMouseButton_Right);
	}

	return this->mIsRightDown;
}
