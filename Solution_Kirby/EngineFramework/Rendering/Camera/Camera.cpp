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
	mPthis->m_at = { 0.0f, 0.0f, 0.0f };
	mPthis->m_rotation = { 0.0f, 0.0f, 0.0f };
	mPthis->m_distance = 20.0f;
	mPthis->UpdateViewMatrix();
}

void Camera::SetPos(float x, float y)
{
	SetPos(x, y, 0.0f);
}

void Camera::SetPos(float x, float y, float z)
{
	mPthis->m_at = { x, y, z };
	mPthis->UpdateViewMatrix();
}

D3DXVECTOR3 Camera::GetPos()
{
	return m_at;
}

void Camera::SetRotation(float x, float y, float z)
{
	m_rotation = D3DXVECTOR3(x, y, z);
	UpdateViewMatrix();
}

void Camera::SetRotation(const D3DXVECTOR3* rotation)
{
	if (!rotation)
	{
		return;
	}
	SetRotation(rotation->x, rotation->y, rotation->z);
}

D3DXVECTOR3 Camera::GetRotation()
{
	return m_rotation;
}

void Camera::AddRotation(float x, float y, float z)
{
	m_rotation += D3DXVECTOR3(x, y, z);
	UpdateViewMatrix();
}

void Camera::AddRotation(const D3DXVECTOR3* rotation)
{
	if (!rotation)
	{
		return;
	}
	AddRotation(rotation->x, rotation->y, rotation->z);
}

D3DXVECTOR3 Camera::GetForward()
{
	D3DXVECTOR3 direction(0.0f, 0.0f, 1.0f);
	return RotateDirection(&direction);
}

D3DXVECTOR3 Camera::GetRight()
{
	D3DXVECTOR3 direction(1.0f, 0.0f, 0.0f);
	return RotateDirection(&direction);
}

D3DXVECTOR3 Camera::GetUp()
{
	D3DXVECTOR3 direction(0.0f, 1.0f, 0.0f);
	return RotateDirection(&direction);
}

D3DXVECTOR3 Camera::RotateDirection(const D3DXVECTOR3* direction)
{
	if (!direction)
	{
		return D3DXVECTOR3(0.0f, 0.0f, 1.0f);
	}

	D3DXMATRIX rotationMatrix;
	D3DXVECTOR3 result;
	D3DXMatrixRotationYawPitchRoll(&rotationMatrix, m_rotation.y, m_rotation.x, m_rotation.z);
	D3DXVec3TransformNormal(&result, direction, &rotationMatrix);
	D3DXVec3Normalize(&result, &result);
	return result;
}

void Camera::UpdateViewMatrix()
{
	D3DXVECTOR3 forward = GetForward();
	m_up = GetUp();
	m_eye = m_at - (forward * m_distance);

	D3DXMATRIX matView;
	D3DXMatrixLookAtLH(&matView, &m_eye, &m_at, &m_up);
	MainFrame::GetInstance()->GetDevice()->SetTransform(D3DTS_VIEW, &matView);
}
