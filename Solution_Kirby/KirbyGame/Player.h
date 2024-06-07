#pragma once
#include "Component.h"
#include "Rigidbody.h"
#include "game.h"
#include "BoxCollider.h"

class Player : public Component
{
protected:
	//Rigidbody* m_rig = nullptr;
	AnimationRender* m_ar = nullptr;
	BoxCollider* m_bo = nullptr;
	Animation m_arrAnim[(int)Arrow::max];
	bool m_isClear = false;

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
};