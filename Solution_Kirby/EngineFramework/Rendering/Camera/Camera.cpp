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

void Camera::InitializeView()
{
	mPthis->m_eye = { 0.0f, 0.0f, -20.0f };
	mPthis->m_at = { 0.0f, 0.0f, 0.0f };
	mPthis->m_up = { 0.0f, 1.0f, 0.0f };

	// 뷰 행렬 생성
	D3DXMATRIX matView;
	D3DXMatrixLookAtLH(&matView, &m_eye, &m_at, &m_up);

	// 디바이스에 뷰 행렬 설정
	MainFrame::GetInstance()->GetDevice()->SetTransform(D3DTS_VIEW, &matView);
}

void Camera::SetPos(float x, float y)
{
	mPthis->m_eye = { x, y, -20.0f };
	mPthis->m_at = { x, y, 0 };
	D3DXMATRIX matView;
	D3DXMatrixLookAtLH(&matView, &m_eye, &m_at, &m_up);
	MainFrame::GetInstance()->GetDevice()->SetTransform(D3DTS_VIEW, &matView);
}

D3DXVECTOR3 Camera::GetPos()
{
	return m_at;
}
