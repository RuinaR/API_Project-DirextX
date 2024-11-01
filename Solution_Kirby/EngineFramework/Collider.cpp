#include "pch.h"
#include "Collider.h"

void Collider::RenderCollider()
{
		D3DXMATRIX matTranslate, matScale, matRotate, matWorld;

		b2Vec2 position = m_body->GetPosition();

		D3DXMatrixTranslation(&matTranslate, position.x, position.y, -9.0f);
		D3DXMatrixScaling(&matScale, m_colSize.x + 1, m_colSize.y + 1, 1.0f);
		D3DXMatrixRotationZ(&matRotate, m_body->GetAngle());
		matWorld = matScale * matRotate * matTranslate;

		MainFrame::GetInstance()->GetDevice()->SetTexture(0, nullptr);
		MainFrame::GetInstance()->GetDevice()->SetTransform(D3DTS_WORLD, &matWorld);
		MainFrame::GetInstance()->GetDevice()->SetFVF(D3DFVF_DEBUGVERTEX);

		DEBUGVERTEX vertices[] =
		{
			{-0.5f, -0.5f, 0.0f, DEBUGCOLORDX1},
			{ 0.5f, -0.5f, 0.0f, DEBUGCOLORDX1},
			{ 0.5f, -0.5f, 0.0f, DEBUGCOLORDX1},
			{ 0.5f,  0.5f, 0.0f, DEBUGCOLORDX1},
			{ 0.5f,  0.5f, 0.0f, DEBUGCOLORDX1},
			{-0.5f,  0.5f, 0.0f, DEBUGCOLORDX1},
			{-0.5f,  0.5f, 0.0f, DEBUGCOLORDX1},
			{-0.5f, -0.5f, 0.0f, DEBUGCOLORDX1},
		};

		MainFrame::GetInstance()->GetDevice()->DrawPrimitiveUP(D3DPT_LINELIST, 4, vertices, sizeof(DEBUGVERTEX));
}

Collider::Collider(b2BodyType type)
	:m_type(type)
{
}
void Collider::Start()
{
}
void Collider::Update()
{
	m_gameObj->SetPosition({
		m_body->GetPosition().x,
		m_body->GetPosition().y,
		m_gameObj->Position().z });

	m_gameObj->SetAngleZ(m_body->GetAngle());

}
const Vector2D& Collider::GetColSize()
{
	return m_colSize;
}

const Vector2D& Collider::GetColOffset()
{
	return m_colOffset;
}

void Collider::CreateBody(Vector2D offset, Vector2D size, bool fixedRotation)
{
	m_colSize = size;
	m_colOffset = offset;

	b2BodyDef bodyDef;
	bodyDef.type = m_type;
	bodyDef.fixedRotation = fixedRotation;
	bodyDef.position.Set(
		m_gameObj->Position().x + (float)m_colOffset.x,
		m_gameObj->Position().y + (float)m_colOffset.y);
	bodyDef.angle = 0.0f;
	m_body = MainFrame::GetInstance()->GetBox2dWorld()->CreateBody(&bodyDef);
	
	b2PolygonShape box;
	box.SetAsBox(m_colSize.x / 2.0f, m_colSize.y / 2.0f);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &box;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.3f;
	fixtureDef.restitution = 0.5f;
	m_body->CreateFixture(&fixtureDef);
	//m_body->SetGravityScale(30.0);
	m_body->GetUserData().pointer = (uintptr_t)this;
}
void Collider::Initialize()
{
	//CollisionManager::GetInstance()->AddCollider(this);
	ColInit();
	RenderManager::GetInstance()->RegisterDebug(this);
}

void Collider::Release()
{
	//CollisionManager::GetInstance()->UnregisterCollider(this);
	MainFrame::GetInstance()->GetBox2dWorld()->DestroyBody(m_body);
	m_body = nullptr;
	RenderManager::GetInstance()->UnregisterDebug(this);

	ColRelease();
}

void Collider::SetTrigger(bool b)
{
	m_isTrigger = b;
}

bool Collider::GetTrigger()
{
	return m_isTrigger;
}

b2Body* Collider::GetBody()
{
	return m_body;
}

void Collider::DebugRenderUpdate()
{
	RenderCollider();
}

