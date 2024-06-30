#pragma once
#include "RenderManager.h"
class Button : public Component
{
protected:
    D3DCOLOR m_textColor = D3DCOLOR_XRGB(0, 0, 0);

    COLORREF m_curColor = NULL;
    D3DXVECTOR3 m_UIPos = D3DXVECTOR3();
    D3DXVECTOR2 m_UISize = D3DXVECTOR2();
    string m_text = "";
    int m_textSize = 20;

    bool m_isClicked = false;
    void ButtonEvent();

    std::function<void()> m_event = nullptr;

public:
    void Initialize() override;
    void Release() override;
    void Start() override;
    void Update() override;
    void UpdateRender();

    void SetUIPos(D3DXVECTOR3 v);
    void SetUISize(D3DXVECTOR2 v);
    void SetTextColor(D3DCOLOR col);
    void SetText(string str);

    void SetEvent(std::function<void()> func);
};

