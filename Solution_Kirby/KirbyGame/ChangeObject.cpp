#include "pch.h"
#include "ChangeObject.h"
#include "BoxCollider.h"


ChangeObject::ChangeObject(PlayerMode mode, Player* player)
	:m_mode(mode), m_player(player), Component()
{
}

ChangeObject::~ChangeObject()
{
}

void ChangeObject::Initialize()
{
}

void ChangeObject::Release()
{
}

void ChangeObject::Start()
{
}

void ChangeObject::Update()
{
	if (m_gameObj->GetDestroy())
		return;
	if (m_player->GetState() == PlayerAState::eating)
	{
		if (m_gameObj->Position().y < m_player->GetGameObject()->Position().y + 50 &&
			m_gameObj->Position().y > m_player->GetGameObject()->Position().y - 10)
		{
			if (m_player->GetArrow() == Arrow::left &&
				m_gameObj->Position().x < m_player->GetGameObject()->Position().x &&
				m_gameObj->Position().x > m_player->GetGameObject()->Position().x - m_area)
			{
				m_gameObj->AddPosition({ (float)(m_speed * MainFrame::GetInstance()->DeltaTime()), 0.0f, 0.0f });
			}
			else if (m_player->GetArrow() == Arrow::right &&
				m_gameObj->Position().x > m_player->GetGameObject()->Position().x &&
				m_gameObj->Position().x < m_player->GetGameObject()->Position().x + m_area)
			{
				m_gameObj->AddPosition({(float)(- m_speed * MainFrame::GetInstance()->DeltaTime()), 0.0f, 0.0f});
			}
		}
	}
}

PlayerMode ChangeObject::GetMode()
{
	return m_mode;
}
