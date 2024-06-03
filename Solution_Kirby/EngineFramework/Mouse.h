#pragma once

class Mouse
{
private:
	static Mouse* mPthis;
	Mouse() {}
	~Mouse() {}

	COORD mPos = { 0 };
	bool mIsLeftDown = false;
	bool mIsRightDown = false;
public:
	static void Create();
	static Mouse* GetInstance();
	static void Destroy();

	void Initialize();

	void SetPos(int x, int y);
	void SetPos(COORD pos);
	COORD GetPos();
	void SetLeftBtn(bool isDown);
	bool IsLeftDown();
	void SetRightBtn(bool isDown);
	bool IsRightDown();
};