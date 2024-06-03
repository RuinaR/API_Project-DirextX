#include "pch.h"
#include "DebugWindow.h"
DebugWindow* DebugWindow::m_Pthis = nullptr;

void DebugWindow::Initialize()
{

}

void DebugWindow::Release()
{

}

void DebugWindow::Start()
{
}

void DebugWindow::Update()
{
	m_gameObj->SetPosition({
		m_dwPos.x + Camera::GetInstance()->GetPos().x,
		m_dwPos.y + Camera::GetInstance()->GetPos().y });

	int bkmode = SetBkMode(WindowFrame::GetInstance()->GetBuffer()->GetHDC(), TRANSPARENT);
	RECT rect = {
	m_gameObj->Position().x - Camera::GetInstance()->GetPos().x,
	m_gameObj->Position().y - Camera::GetInstance()->GetPos().y,
	m_gameObj->Position().x - Camera::GetInstance()->GetPos().x + m_dwSize.x,
	m_gameObj->Position().y - Camera::GetInstance()->GetPos().y + m_dwSize.y };
    FillRectWithColor(WindowFrame::GetInstance()->GetBuffer()->GetHDC(), rect, RGB(255, 255, 0));
	DrawTextInRect(WindowFrame::GetInstance()->GetBuffer()->GetHDC(), 
		ConvertToWideString(
			m_text1 + "\n\n" +
			m_text2 + "\n\n" +
			m_text3 + "\n\n"			
		), rect);
	SetBkMode(WindowFrame::GetInstance()->GetBuffer()->GetHDC(), bkmode);
}

DebugWindow* DebugWindow::GetInstance()
{
    return m_Pthis;
}


void DebugWindow::Create()
{
	if (m_Pthis == nullptr)
	{
		m_Pthis = new DebugWindow();
		GameObject* DW = new GameObject();
		DW->AddComponent(m_Pthis);
		m_Pthis->SetDWPos({ 0, 0 });
		m_Pthis->SetDWSize({ 200,400 });
		DW->SetOrderInLayer(10);
		DW->SetTag("DebugWindow");
		DW->InitializeSet();
	}
}


void DebugWindow::Destroy()
{
	if (m_Pthis != nullptr)
	{
		if (m_Pthis->m_gameObj != nullptr)
		{
			m_Pthis->m_gameObj->SetDestroy(true);
		}
		else
		{
			delete m_Pthis;
		}
		m_Pthis = nullptr;
	}
}

void DebugWindow::SetText1(string text)
{
	m_text1 = text;
}

void DebugWindow::SetText2(string text)
{
	m_text2 = text;
}

void DebugWindow::SetText3(string text)
{
	m_text3 = text;
}

void DebugWindow::SetDWSize(Vector2D size)
{
	m_dwSize = size;
	if (m_gameObj)
		m_gameObj->Size() = m_dwSize;
}

void DebugWindow::SetDWPos(Vector2D pos)
{
	m_dwPos = pos;
}

