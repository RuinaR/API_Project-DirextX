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
	int clientWidth = m_viewport.Width;
	int clientHeight = m_viewport.Height;
	this->mPos.x = x - (clientWidth/2);
	this->mPos.y = (clientHeight / 2) - y;
}

void Mouse::SetPos(D3DXVECTOR2 pos)
{
	this->mPos = pos;
}

D3DXVECTOR2 Mouse::GetPos()
{
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
