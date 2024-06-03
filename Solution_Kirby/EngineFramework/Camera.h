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
public:
	static void Create();
	static Camera* GetInstance();
	static void Destroy();

	void InitializeView();
	void SetPos(float x, float y);
	D3DXVECTOR3 GetPos();
};
