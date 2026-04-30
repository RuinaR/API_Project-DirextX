#pragma once
#include "structs.h"

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
	D3DXVECTOR3 m_rotation = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	float m_distance = 20.0f;

	D3DXVECTOR3 RotateDirection(const D3DXVECTOR3* direction);
	void UpdateViewMatrix();
public:
	static void Create();
	static Camera* GetInstance();
	static void Destroy();

	void InitializeView();
	void SetPos(float x, float y);
	void SetPos(float x, float y, float z);
	D3DXVECTOR3 GetPos();
	void SetRotation(float x, float y, float z);
	void SetRotation(const D3DXVECTOR3* rotation);
	D3DXVECTOR3 GetRotation();
	void AddRotation(float x, float y, float z);
	void AddRotation(const D3DXVECTOR3* rotation);
	D3DXVECTOR3 GetForward();
	D3DXVECTOR3 GetRight();
	D3DXVECTOR3 GetUp();
};
