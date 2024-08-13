#pragma once
class AttakObject : public Component
{
protected:
	IDirect3DTexture9* m_texture = NULL;
	float m_speedX = 0.0f;
	virtual void CollisionEnter(Collider* other);
	virtual void CollisionExit(Collider* other);
	virtual void Collision(Collider* other);
public:
	AttakObject();
	virtual ~AttakObject();
	void Initialize() override;
	void Release() override;
	void Start() override;
	void Update() override;
	void SetSpeed(float speed);
};

