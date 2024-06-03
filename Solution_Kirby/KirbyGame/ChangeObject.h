#pragma once
#include "Player.h"

class ChangeObject : public Component
{
protected:
	PlayerMode m_mode = PlayerMode::mDefault;
	Player* m_player = nullptr;
	float m_area = 300.0f;
	float m_speed = 300.0f;
public:
	ChangeObject(PlayerMode mode, Player* player);	
	virtual ~ChangeObject();
	void Initialize() override;
	void Release() override;
	void Start() override;
	void Update() override;

	PlayerMode GetMode();
};

