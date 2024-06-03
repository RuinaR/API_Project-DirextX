#include "pch.h"
#include "Buffer.h"

Buffer::Buffer()
	:m_bitmap(NULL), m_hWnd(NULL), m_hdc(NULL), m_oldBitmap(NULL), m_bitInfo(), m_bg(NULL), m_bgInfo()
{

}

Buffer::~Buffer()
{

}

void Buffer::Init(HWND hWnd)
{
	m_hWnd = hWnd;
	RECT rect = { 0 };
	GetClientRect(m_hWnd, &rect);

	HDC hdcMain = GetDC(m_hWnd);
	m_hdc = CreateCompatibleDC(hdcMain);
	m_bitmap = CreateCompatibleBitmap(hdcMain, BUFFERBITW, BUFFERBITH);

	m_oldBitmap = (HBITMAP)SelectObject(m_hdc, m_bitmap);
	FillRect(m_hdc, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
	GetObject(m_bitmap, sizeof(BITMAP), &m_bitInfo);

	ReleaseDC(m_hWnd, hdcMain);
}

void Buffer::Release()
{
	SelectObject(m_hdc, m_oldBitmap);
	DeleteDC(m_hdc);
	DeleteObject(m_bitmap);
}

HBITMAP Buffer::GetBitmap()
{
	return m_bitmap;
}

BITMAP Buffer::GetBitmapInfo()
{
	return m_bitInfo;
}

HDC Buffer::GetHDC()
{
	return m_hdc;
}

void Buffer::SetBG(HBITMAP bit)
{
	m_bg = bit;
	if (m_bg != NULL)
	{
		GetObject(m_bg, sizeof(BITMAP), &m_bgInfo);
	}
}

void Buffer::DrawBG()
{
	if (m_bg == NULL)
	{
		RECT rect;
		rect = { 0,0,m_bitInfo.bmWidth,m_bitInfo.bmHeight };
		FillRect(m_hdc, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
		return;
	}
	HDC MemoryDC;
	HBITMAP OldBitmap;
	MemoryDC = CreateCompatibleDC(m_hdc);
	OldBitmap = (HBITMAP)SelectObject(MemoryDC, m_bg);
	TransparentBlt(m_hdc, 0, 0, m_bitInfo.bmWidth, m_bitInfo.bmHeight, 
		MemoryDC, 0, 0, m_bgInfo.bmWidth, m_bgInfo.bmHeight, TRANSCOLOR);
	SelectObject(MemoryDC, OldBitmap);
	DeleteDC(MemoryDC);
}

void Buffer::CopyBitmap(HDC hdc)
{
	BitBlt(hdc, 0, 0, m_bitInfo.bmWidth, m_bitInfo.bmHeight, m_hdc, 0, 0, SRCCOPY);
}
