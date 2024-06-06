#pragma once
#include "game.h"
#include "LandChecker.h"
#include "AnimationRender.h"

class MonsterAI : public Component
{
private:
	int m_arrowVal = 0;
	float m_speed = 700.0f;
	Arrow m_arrow = Arrow::right;
	Animation m_anim[(int)Arrow::max];
private:
	//LandChecker* m_checker = nullptr;
	AnimationRender* m_ar = nullptr;
	BoxCollider* m_bo = nullptr;
protected:
	void CollisionEnter(Collider* other)override;
	void CollisionExit(Collider* other)override;
	void Collision(Collider* other)override;
public:
	void Initialize() override;
	void Release() override;
	void Start() override;
	void Update() override;
public:
	MonsterAI(Animation left, Animation right);
	~MonsterAI();
};

