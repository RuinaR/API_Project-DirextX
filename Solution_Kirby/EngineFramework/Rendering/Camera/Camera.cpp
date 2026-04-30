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
	mPthis->m_projectionMode = CameraProjectionMode::Orthographic;
	mPthis->m_fov = D3DX_PI / 4.0f;
	mPthis->m_orthographicSize = static_cast<float>(DRAWWINDOWH);
	mPthis->m_nearClip = 1.0f;
	mPthis->m_farClip = 1000.0f;
	mPthis->UpdateViewMatrix();
	mPthis->UpdateProjectionMatrix();
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

const D3DXMATRIX& Camera::GetViewMatrix() const
{
	return m_viewMatrix;
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

void Camera::SetProjectionMode(CameraProjectionMode mode)
{
	m_projectionMode = mode;
	UpdateProjectionMatrix();
}

CameraProjectionMode Camera::GetProjectionMode() const
{
	return m_projectionMode;
}

void Camera::SetFov(float fov)
{
	m_fov = max(0.01f, fov);
	UpdateProjectionMatrix();
}

float Camera::GetFov() const
{
	return m_fov;
}

void Camera::SetOrthographicSize(float orthographicSize)
{
	m_orthographicSize = max(1.0f, orthographicSize);
	UpdateProjectionMatrix();
}

float Camera::GetOrthographicSize() const
{
	return m_orthographicSize;
}

void Camera::SetNearClip(float nearClip)
{
	m_nearClip = max(0.001f, nearClip);
	if (m_farClip <= m_nearClip)
	{
		m_farClip = m_nearClip + 1.0f;
	}
	UpdateProjectionMatrix();
}

float Camera::GetNearClip() const
{
	return m_nearClip;
}

void Camera::SetFarClip(float farClip)
{
	m_farClip = max(m_nearClip + 1.0f, farClip);
	UpdateProjectionMatrix();
}

float Camera::GetFarClip() const
{
	return m_farClip;
}

void Camera::ApplyProjection()
{
	UpdateProjectionMatrix();
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

	D3DXMatrixLookAtLH(&m_viewMatrix, &m_eye, &m_at, &m_up);
	MainFrame::GetInstance()->GetDevice()->SetTransform(D3DTS_VIEW, &m_viewMatrix);
}

void Camera::UpdateProjectionMatrix()
{
	if (MainFrame::GetInstance() == nullptr)
	{
		return;
	}

	MainFrame::GetInstance()->ApplyCameraProjection();
}
