#pragma once
#include "RenderManager.h"
class ColorButton : public Component
{
protected:
    COLORREF m_defaultColor = RGB(255, 255, 255);
    COLORREF m_hoverColor = RGB(255, 255, 0);
    COLORREF m_downColor = RGB(0, 255, 0);
    COLORREF m_borderColor = RGB(0, 0, 0);
    D3DCOLOR m_textColor = D3DCOLOR_XRGB(0, 0, 0);

    COLORREF m_curColor = NULL;
    D3DXVECTOR3 m_UIPos = D3DXVECTOR3();
    D3DXVECTOR2 m_UISize = D3DXVECTOR2();
    std::wstring m_text = L"";
    int m_textSize = 20;

    void ButtonEvent();

    std::function<void()> m_event = nullptr;

public:
    void Initialize() override;
    void Release() override;
    void Start() override;
    void Update() override;
    void UpdateRender();
    void OnLBtnDown() final;
    void OnLBtnUp() final;
    void OnRBtnDown() final;
    void OnRBtnUp() final;
    void OnMouseHover() final;

    void SetUIPos(D3DXVECTOR3 v);
    void SetUISize(D3DXVECTOR2 v);
    void SetDefaultColor(COLORREF col);
    void SetHoverColor(COLORREF col);
    void SetDownColor(COLORREF col);
    void SetBorderColor(COLORREF col);
    void SetTextColor(D3DCOLOR col);
    void SetTextSize(int val);
    void SetText(std::wstring str);

    void SetEvent(std::function<void()> func);
};

