#include "pch.h"
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
	D3DXVECTOR2 gameViewPos = RenderManager::GetInstance()->GetGameViewPos();
	D3DXVECTOR2 gameViewSize = RenderManager::GetInstance()->GetGameViewSize();
	D3DXVECTOR2 localPos =
	{
		mPos.x - gameViewPos.x,
		mPos.y - gameViewPos.y
	};

	if (gameViewSize.x > 0.0f && gameViewSize.y > 0.0f)
	{
		localPos.x *= static_cast<float>(DRAWWINDOWW) / gameViewSize.x;
		localPos.y *= static_cast<float>(DRAWWINDOWH) / gameViewSize.y;
	}

	return localPos;
}

D3DXVECTOR3 Mouse::GetWorldPos(const D3DXVECTOR2& cameraPos, float z)
{
	D3DXVECTOR2 dxPos = GetDXPos();
	return D3DXVECTOR3(dxPos.x + cameraPos.x, dxPos.y + cameraPos.y, z);
}

void Mouse::SetLeftBtn(bool isDown)
{
	this->mIsLeftDown = isDown;
}

bool Mouse::IsLeftDown()
{
	return this->mIsLeftDown;
}

void Mouse::SetRightBtn(bool isDown)
{
	this->mIsRightDown = isDown;
}

bool Mouse::IsRightDown()
{
	return this->mIsRightDown;
}
