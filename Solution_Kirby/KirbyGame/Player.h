#pragma once
#include "Component.h"
#include "Rigidbody.h"
#include "game.h"

class Player : public Component
{
protected:
	Rigidbody* m_rig = nullptr;
	AnimationRender* m_ar = nullptr;
	BoxCollider* m_bo = nullptr;
	Animation m_arrAnim[(int)PlayerMode::max][(int)Arrow::max][(int)PlayerAState::max];
	PlayerMode m_mode;
	Arrow m_arrow;
	PlayerAState m_state;
	PlayerMode m_eatMode = PlayerMode::mDefault;
	Timer m_flyTimer = Timer();
	Timer m_leftKeyTimer = Timer();
	Timer m_rightKeyTimer = Timer();
	Vector2D m_dSize = { UNITSIZE,UNITSIZE };
	Vector2D m_cSize = { 40,40 };
	Vector2D m_dOffset = { 30,50 };
	float m_atkRange = 60.0f;
	bool m_leftKeyTrigger = false;
	bool m_rightKeyTrigger = false;
	bool m_jumpFlyTrigger = false;
	bool m_atkTrigger = false;

	float m_dblTime = 0.2f;

	int m_leftKey = 0;
	int m_rightKey = 0;
	int m_jumpKey = 0;
	int m_atkKey = 0;
	int m_changeKey = 0;
	int m_upKey = 0;

	float m_speed = 250.0f;
	float m_speed_run = 500.0f;
	float m_speed_fly = 150.0f;
	float m_eatSpeed = 150.0f;
	float m_curXSpeed = 0.0f;
	float m_JumpV = 600.0f;

	float m_startGravity = 1200.0f;
	float m_flyGravity = 450.0f;
	float m_stoneGravity = 2400.0f;

	bool isStoneAtked = false;

	void (Player::* m_attackFunc[(int)PlayerMode::max])();
protected:
	void UpdateAnim(bool isOneTime);
	void FlyAction();
	void MoveLeft();
	void MoveRight();
	void JumpAction();
	void Idle();
	void StoneAttacking();
protected:
	void Attack_default();
	void Attack_sword();
	void Attack_stone();
protected:
	void CollisionEnter(Collider* other) override;
	void CollisionExit(Collider* other) override;
	void Collision(Collider* other) override;
public:
	Player();
	~Player();
	void Initialize() override;
	void Release() override;
	void Start() override;
	void Update() override;
	void SetPlayerMode(PlayerMode mode);
	PlayerMode GetPlayerMode();
	PlayerAState GetState();
	Arrow GetArrow();
};