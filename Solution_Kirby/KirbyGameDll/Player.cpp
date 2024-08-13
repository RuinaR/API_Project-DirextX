#include "pch.h"
#include "Player.h"
#include "GameObject.h"
#include "Component.h"
#include "Collider.h"
#include "BoxCollider.h"
#include "Rigidbody.h"
#include "ChangeObject.h"
#include "AttakObject.h"
#include "AttackEvent.h"
#include "DebugWindow.h"
#include "SceneChanger.h"
#include "StageMaker.h"
#include "Button.h"

float Lerp(float start, float end, float t)
{
	return start + ((end - start) * t);
}

void Player::CollisionEnter(Collider* other)
{
}

void Player::CollisionExit(Collider* other)
{
}

void Player::Collision(Collider* other)
{
	if (other == nullptr)
		return;
	if (other->GetGameObject()->GetTag() == TAG_DOOR)
	{
		m_isClear = true;
		return;
	}
}

Player::Player()
	: Component()
{
}

Player::~Player()
{
}

void Player::Initialize()
{
	m_gameObj->SetTag(TAG_PLAYER);
	m_gameObj->Size() = {(float)80.0f, (float)80.0f};
	wstring arrowStr[(int)Arrow::max] = { L"left", L"right" };
	for (int i = 0; i < (int)Arrow::max; i++)
	{
		wstring path = L"Bitmaps\\Player\\default\\" + arrowStr[i] + L"\\idle";
		float atime = 0.25f;
		m_arrAnim[i] = AnimationManager::LoadAnimation(path, atime);
	}
	m_ar = new AnimationRender(m_arrAnim[(int)Arrow::right]);
	//m_rig = new Rigidbody();
	m_bo = new BoxCollider(b2BodyType::b2_dynamicBody);
	//m_rig->SetGravity(0.0f);
	m_gameObj->AddComponent(m_bo);
	m_gameObj->AddComponent(m_ar);
	m_bo->CreateBody({ 20.0f, 20.0f}, {50.0f, 50.0f}, false);
	//m_gameObj->AddComponent(m_rig);
}

void Player::Release()
{
	for (int i = 0; i < (int)Arrow::max; i++)
	{
		AnimationManager::ReleaseAnimation(m_arrAnim[i]);
	}
}

void Player::Start()
{
}

void Player::Update()
{
	if (m_isClear)
	{
		SceneChanger::GetInstance()->ChangeGameScene();
		return;
	}
	D3DXVECTOR3 playerPos = m_gameObj->Position();
	D3DXVECTOR3 camPos = Camera::GetInstance()->GetPos();
	float smoothFactor = 0.02f;
	D3DXVECTOR2 newCamPos = {
		Lerp(camPos.x, playerPos.x, smoothFactor),
		Lerp(camPos.y, playerPos.y, smoothFactor)
	};
	Camera::GetInstance()->SetPos(newCamPos.x, newCamPos.y);

	D3DXVECTOR3 mousePos = {
		Mouse::GetInstance()->GetDXPos().x,
		Mouse::GetInstance()->GetDXPos().y,
		playerPos.z};
	D3DXVECTOR3 newPlayerPos = {
		Lerp(playerPos.x, mousePos.x + camPos.x, smoothFactor),
		Lerp(playerPos.y, mousePos.y + camPos.y, smoothFactor),
		playerPos.z
	};
	D3DXVECTOR3 force = newPlayerPos - playerPos;
	force *= 200.0f;
	b2Vec2 b2force = { force.x, force.y };
	m_bo->GetBody()->SetLinearVelocity(b2force);

}