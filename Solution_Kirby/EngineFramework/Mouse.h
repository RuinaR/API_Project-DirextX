#pragma once

class Mouse
{
private:
	static Mouse* mPthis;
	Mouse() {}
	~Mouse() {}

	D3DXVECTOR2 mPos = { 0,0 };
	bool mIsLeftDown = false;
	bool mIsRightDown = false;
	D3DVIEWPORT9 m_viewport;
public:
	static void Create();
	static Mouse* GetInstance();
	static void Destroy();

	void Initialize();

	void SetPos(int x, int y);
	void SetPos(D3DXVECTOR2 pos);
	D3DXVECTOR2 GetPos();
	void SetLeftBtn(bool isDown);
	bool IsLeftDown();
	void SetRightBtn(bool isDown);
	bool IsRightDown();
};