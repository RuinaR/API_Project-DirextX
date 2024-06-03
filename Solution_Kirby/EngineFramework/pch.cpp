#include "pch.h"

D3DXVECTOR3 WorldToScreen(LPDIRECT3DDEVICE9 pd3dDevice, const D3DXVECTOR3& worldPos)
{
    D3DXMATRIX matView, matProj;
    D3DVIEWPORT9 viewport;

    // �� ��Ʈ������ �������� ��Ʈ������ ������
    pd3dDevice->GetTransform(D3DTS_VIEW, &matView);
    pd3dDevice->GetTransform(D3DTS_PROJECTION, &matProj);

    // ����Ʈ ������
    pd3dDevice->GetViewport(&viewport);

    // ��ũ�� ��ǥ�� ������ ����
    D3DXVECTOR3 screenPos;

    // ���� ��ǥ�� ��ũ�� ��ǥ�� ��ȯ
    D3DXVec3Project(&screenPos, &worldPos, &viewport, &matProj, &matView, NULL);

    return screenPos;
}

D3DXVECTOR3 ScreenToWorld(LPDIRECT3DDEVICE9 pd3dDevice, const D3DXVECTOR3& screenPos)
{
    D3DXMATRIX matView, matProj, matWorld;
    D3DVIEWPORT9 viewport;

    // �� ��Ʈ������ �������� ��Ʈ������ ������
    pd3dDevice->GetTransform(D3DTS_VIEW, &matView);
    pd3dDevice->GetTransform(D3DTS_PROJECTION, &matProj);

    // ����Ʈ ������
    pd3dDevice->GetViewport(&viewport);

    // ��ũ�� ��ǥ�� ���� ��ǥ�� ��ȯ�ϱ� ���ؼ��� ������� ����ؾ� �մϴ�.
    // ���� �� ��Ʈ������ �������� ��Ʈ������ ����ķ� ����ϴ�.
    D3DXMATRIX matViewInv, matProjInv;
    D3DXMatrixInverse(&matViewInv, nullptr, &matView);
    D3DXMatrixInverse(&matProjInv, nullptr, &matProj);

    // ��ũ�� ��ǥ�� NDC ��ǥ�� ��ȯ�մϴ�.
    D3DXVECTOR3 ndcPos;
    ndcPos.x = ((2.0f * screenPos.x) / viewport.Width) - 1.0f;
    ndcPos.y = 1.0f - ((2.0f * screenPos.y) / viewport.Height);
    ndcPos.z = screenPos.z;

    // NDC ��ǥ�� ���� ��ǥ�� ��ȯ
    D3DXVec3TransformCoord(&ndcPos, &ndcPos, &matProjInv);
    D3DXVec3TransformCoord(&ndcPos, &ndcPos, &matViewInv);

    return ndcPos;
}

void DrawTextInRect(HDC hdc, const std::wstring& text, const RECT& rect)
{
    if (hdc != NULL)
    {
        // �簢�� ���ο� �ؽ�Ʈ ���
        DrawText(hdc, text.c_str(), -1, (LPRECT)&rect, DT_CENTER | DT_WORDBREAK | DT_VCENTER);
    }
}

wstring ConvertToWideString(const std::string& narrowStr)
{
    wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(narrowStr);
}

void FillRectWithColor(HDC hdc, const RECT& rect, COLORREF color)
{
    if (hdc != NULL)
    {
        HBRUSH hBrush = CreateSolidBrush(color);
        if (hBrush != NULL)
        {
            FillRect(hdc, &rect, hBrush);
            DeleteObject(hBrush);
        }
    }
}