#pragma once
#include "structs.h"

enum class CameraProjectionMode
{
	Orthographic = 0,
	Perspective,
};

class Camera
{
private:
	static Camera* mPthis;
	Camera() {}
	~Camera() {}
	// 카메라 위치
	D3DXVECTOR3 m_eye;
	D3DXVECTOR3 m_at;
	D3DXVECTOR3 m_up;
	D3DXMATRIX m_viewMatrix;
	D3DXVECTOR3 m_rotation = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	float m_distance = 20.0f;
	CameraProjectionMode m_projectionMode = CameraProjectionMode::Orthographic;
	float m_fov = D3DX_PI / 4.0f;
	float m_orthographicSize = 1.0f;
	float m_nearClip = 1.0f;
	float m_farClip = 1000.0f;

	D3DXVECTOR3 RotateDirection(const D3DXVECTOR3* direction);
	void UpdateViewMatrix();
	void UpdateProjectionMatrix();
public:
	static void Create();
	static Camera* GetInstance();
	static void Destroy();

	void InitializeView();
	void SetPos(float x, float y);
	void SetPos(float x, float y, float z);
	D3DXVECTOR3 GetPos();
	const D3DXMATRIX& GetViewMatrix() const;
	void SetRotation(float x, float y, float z);
	void SetRotation(const D3DXVECTOR3* rotation);
	D3DXVECTOR3 GetRotation();
	void AddRotation(float x, float y, float z);
	void AddRotation(const D3DXVECTOR3* rotation);
	D3DXVECTOR3 GetForward();
	D3DXVECTOR3 GetRight();
	D3DXVECTOR3 GetUp();
	void SetProjectionMode(CameraProjectionMode mode);
	CameraProjectionMode GetProjectionMode() const;
	void SetFov(float fov);
	float GetFov() const;
	void SetOrthographicSize(float orthographicSize);
	float GetOrthographicSize() const;
	void SetNearClip(float nearClip);
	float GetNearClip() const;
	void SetFarClip(float farClip);
	float GetFarClip() const;
	void ApplyProjection();
};
