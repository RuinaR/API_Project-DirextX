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
	this->mPos.X = 0;
	this->mPos.Y = 0;
	this->mIsLeftDown = false;
	this->mIsRightDown = false;
}

void Mouse::SetPos(int x, int y)
{
	this->mPos.X = x;
	this->mPos.Y = y;
}

void Mouse::SetPos(COORD pos)
{
	this->mPos = pos;
}

COORD Mouse::GetPos()
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
