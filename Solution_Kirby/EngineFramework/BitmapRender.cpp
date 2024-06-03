#include "pch.h"
#include "BitmapRender.h"

void BitmapRender::DrawBitmap(HDC hdc, int x, int y, int w, int h, HBITMAP hbit, UINT transparentColor)
{
    D3DXVECTOR3 vec;
    D3DXMATRIX mat;
    D3DXMatrixTranslation(&mat, 1, 1, 1);

    HDC MemoryDC;
    int BitmapX, BitmapY;
    BITMAP Bitmap_;
    HBITMAP OldBitmap;
    MemoryDC = CreateCompatibleDC(hdc);
    OldBitmap = (HBITMAP)SelectObject(MemoryDC, hbit);
    GetObject(hbit, sizeof(BITMAP), &Bitmap_);
    BitmapX = Bitmap_.bmWidth;
    BitmapY = Bitmap_.bmHeight;
    TransparentBlt(hdc, x, y, w, h, MemoryDC, 0, 0, BitmapX, BitmapY, transparentColor);
    SelectObject(MemoryDC, OldBitmap);
    DeleteDC(MemoryDC);
}


BitmapRender::BitmapRender(HBITMAP hbit) : Component(), m_bit(hbit)
{
}

void BitmapRender::Initialize()
{
}

void BitmapRender::Release()
{
}

void BitmapRender::ChangeBitmap(HBITMAP hbit)
{
	m_bit = hbit;
}

void BitmapRender::Start()
{
}

void BitmapRender::Update()
{
	if (m_bit == nullptr)
		return;

    RECT tmp, bitrect, clientrect;
    GetClientRect(WindowFrame::GetInstance()->GetHWND(), &clientrect);
    bitrect = 
        { (long)(m_gameObj->Position().x - Camera::GetInstance()->GetPos().x),
        (long)(m_gameObj->Position().y - Camera::GetInstance()->GetPos().y),
        (long)(m_gameObj->Position().x - Camera::GetInstance()->GetPos().x) + (long)(m_gameObj->Size().x),
        (long)(m_gameObj->Position().y - Camera::GetInstance()->GetPos().y) + (long)(m_gameObj->Size().y) };
    if (IntersectRect(&tmp, &bitrect, &clientrect))
    {
        DrawBitmap(WindowFrame::GetInstance()->GetBuffer()->GetHDC(),
            m_gameObj->Position().x - Camera::GetInstance()->GetPos().x,
            m_gameObj->Position().y - Camera::GetInstance()->GetPos().y,
            m_gameObj->Size().x,
            m_gameObj->Size().y,
            m_bit, TRANSCOLOR);
    }
}
