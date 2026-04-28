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
	D3DXVECTOR2 dxPos;
	int clientWidth = m_viewport.Width;
	int clientHeight = m_viewport.Height;
	dxPos.x = this->mPos.x - (clientWidth / 2) - RenderManager::GetInstance()->GetWinPos().x;
	dxPos.y = (clientHeight / 2) - this->mPos.y + RenderManager::GetInstance()->GetWinPos().y;
	return dxPos;
}

D3DXVECTOR2 Mouse::GetWinPos()
{
	D3DXVECTOR2 winPos;
	winPos.x += RenderManager::GetInstance()->GetWinPos().x;
	winPos.y += RenderManager::GetInstance()->GetWinPos().y;

	return this->mPos;
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
