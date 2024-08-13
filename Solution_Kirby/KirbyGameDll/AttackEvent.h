#pragma once

class AttackEvent : public Component
{
protected:
	Timer m_timer;
	int m_dmg;
protected:
	void CollisionEnter(Collider* other)override;
	void CollisionExit(Collider* other)override;
	void Collision(Collider* other)override;

public:
	AttackEvent() {};
	~AttackEvent()override {};
	void Initialize() override;
	void Release() override;
	void Start() override;
	void Update() override;
	void SetDmg(int dmg);
};

