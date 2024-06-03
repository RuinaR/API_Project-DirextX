#include "pch.h"
#include "Camera.h"
Camera* Camera::mPthis = nullptr;
void Camera::Create()
{
	if (!mPthis)
	{
		mPthis = new Camera();
	}
}

Camera* Camera::GetInstance()
{
	return mPthis;
}

void Camera::Destroy()
{
	if (mPthis)
	{
		delete mPthis;
		mPthis = nullptr;
	}
}

void Camera::SetPos(float x, float y)
{
	m_Position.x = x;
	m_Position.y = y;
}

Vector2D Camera::GetPos()
{
	return m_Position;
}
