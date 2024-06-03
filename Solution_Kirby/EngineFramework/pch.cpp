#include "pch.h"

D3DXVECTOR3 WorldToScreen(LPDIRECT3DDEVICE9 pd3dDevice, const D3DXVECTOR3& worldPos)
{
    D3DXMATRIX matView, matProj;
    D3DVIEWPORT9 viewport;

    // 뷰 매트릭스와 프로젝션 매트릭스를 얻어오기
    pd3dDevice->GetTransform(D3DTS_VIEW, &matView);
    pd3dDevice->GetTransform(D3DTS_PROJECTION, &matProj);

    // 뷰포트 얻어오기
    pd3dDevice->GetViewport(&viewport);

    // 스크린 좌표를 저장할 변수
    D3DXVECTOR3 screenPos;

    // 월드 좌표를 스크린 좌표로 변환
    D3DXVec3Project(&screenPos, &worldPos, &viewport, &matProj, &matView, NULL);

    return screenPos;
}

D3DXVECTOR3 ScreenToWorld(LPDIRECT3DDEVICE9 pd3dDevice, const D3DXVECTOR3& screenPos)
{
    D3DXMATRIX matView, matProj, matWorld;
    D3DVIEWPORT9 viewport;

    // 뷰 매트릭스와 프로젝션 매트릭스를 얻어오기
    pd3dDevice->GetTransform(D3DTS_VIEW, &matView);
    pd3dDevice->GetTransform(D3DTS_PROJECTION, &matProj);

    // 뷰포트 얻어오기
    pd3dDevice->GetViewport(&viewport);

    // 스크린 좌표를 월드 좌표로 변환하기 위해서는 역행렬을 사용해야 합니다.
    // 따라서 뷰 매트릭스와 프로젝션 매트릭스를 역행렬로 만듭니다.
    D3DXMATRIX matViewInv, matProjInv;
    D3DXMatrixInverse(&matViewInv, nullptr, &matView);
    D3DXMatrixInverse(&matProjInv, nullptr, &matProj);

    // 스크린 좌표를 NDC 좌표로 변환합니다.
    D3DXVECTOR3 ndcPos;
    ndcPos.x = ((2.0f * screenPos.x) / viewport.Width) - 1.0f;
    ndcPos.y = 1.0f - ((2.0f * screenPos.y) / viewport.Height);
    ndcPos.z = screenPos.z;

    // NDC 좌표를 월드 좌표로 변환
    D3DXVec3TransformCoord(&ndcPos, &ndcPos, &matProjInv);
    D3DXVec3TransformCoord(&ndcPos, &ndcPos, &matViewInv);

    return ndcPos;
}

void DrawTextInRect(HDC hdc, const std::wstring& text, const RECT& rect)
{
    if (hdc != NULL)
    {
        // 사각형 내부에 텍스트 출력
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