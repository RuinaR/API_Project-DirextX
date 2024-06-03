#pragma once
#include <Windows.h>

class Buffer
{
private:
	HWND m_hWnd;
	HBITMAP m_bitmap;
	HBITMAP m_oldBitmap;
	HBITMAP m_bg;
	BITMAP m_bitInfo;
	BITMAP m_bgInfo;
	HDC m_hdc;
public:
	Buffer();
	~Buffer();

	void Init(HWND hWnd);
	void Release();
	HBITMAP GetBitmap();
	BITMAP GetBitmapInfo();

	HDC GetHDC();
	void SetBG(HBITMAP bit);
	void DrawBG();
	void CopyBitmap(HDC hdc);
};

