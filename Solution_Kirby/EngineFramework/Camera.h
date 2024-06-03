#pragma once
#include "structs.h"

class Camera
{
private:
	static Camera* mPthis;
	Camera() {}
	~Camera() {}
	Vector2D m_Position = {0.0f, 0.0f};
public:
	static void Create();
	static Camera* GetInstance();
	static void Destroy();

	void SetPos(float x, float y);
	Vector2D GetPos();
};
