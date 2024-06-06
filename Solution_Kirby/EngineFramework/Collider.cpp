#include "pch.h"
#include "Collider.h"

void Collider::RenderCollider()
{
	D3DXMATRIX matTranslate, matScale, matWorld;

	b2Vec2 position = m_body->GetPosition();
	
	D3DXMatrixTranslation(&matTranslate, position.x, position.y, -9.0f);
	D3DXMatrixScaling(&matScale, m_colSize.x + 1, m_colSize.y + 1, 1.0f);
	matWorld = matScale * matTranslate;

	MainFrame::GetInstance()->GetDevice()->SetTransform(D3DTS_WORLD, &matWorld);
	MainFrame::GetInstance()->GetDevice()->SetFVF(D3DFVF_DEBUGVERTEX);

	DEBUGVERTEX vertices[] =
	{
		{-0.5f, -0.5f, 0.0f, D3DCOLOR_XRGB(255,255,255)},
		{ 0.5f, -0.5f, 0.0f, D3DCOLOR_XRGB(255,255,255)},
		{ 0.5f, -0.5f, 0.0f, D3DCOLOR_XRGB(255,255,255)},
		{ 0.5f,  0.5f, 0.0f, D3DCOLOR_XRGB(255,255,255)},
		{ 0.5f,  0.5f, 0.0f, D3DCOLOR_XRGB(255,255,255)},
		{-0.5f,  0.5f, 0.0f, D3DCOLOR_XRGB(255,255,255)},
		{-0.5f,  0.5f, 0.0f, D3DCOLOR_XRGB(255,255,255)},
		{-0.5f, -0.5f, 0.0f, D3DCOLOR_XRGB(255,255,255)},
	};

	MainFrame::GetInstance()->GetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
	MainFrame::GetInstance()->GetDevice()->DrawPrimitiveUP(D3DPT_LINELIST, 4, vertices, sizeof(DEBUGVERTEX));
	MainFrame::GetInstance()->GetDevice()->SetRenderState(D3DRS_LIGHTING, TRUE);
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

	if (DEBUGMODE)
		RenderCollider();
}
const Vector2D& Collider::GetColSize()
{
	return m_colSize;
}

const Vector2D& Collider::GetColOffset()
{
	return m_colOffset;
}

void Collider::CreateBody(Vector2D offset, Vector2D size)
{
	m_colSize = size;
	m_colOffset = offset;

	b2BodyDef bodyDef;
	bodyDef.type = m_type;
	bodyDef.fixedRotation = true;
	bodyDef.position.Set(
		m_gameObj->Position().x + (float)m_colOffset.x,
		m_gameObj->Position().y + (float)m_colOffset.y);
	bodyDef.angle = 0.0f;
	if (!MainFrame::GetInstance()->GetBox2dWorld()->IsLocked())
	{
		m_body = MainFrame::GetInstance()->GetBox2dWorld()->CreateBody(&bodyDef);
	}
	else 
	{
		cerr << "Failed to create body!" << endl;
		return;
	}
	b2PolygonShape box;
	box.SetAsBox(m_colSize.x / 2.0f, m_colSize.y / 2.0f);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &box;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.3f;
	fixtureDef.restitution = 0.1f;
	m_body->CreateFixture(&fixtureDef);

	float inertia = m_body->GetInertia();
	b2MassData data;
	data.center = b2Vec2(0, 0);
	data.mass = 1.0f;
	data.I = inertia;
	m_body->SetMassData(&data);

	m_body->SetGravityScale(30.0);
	m_body->GetUserData().pointer = (uintptr_t)this;
}
void Collider::Initialize()
{
	//CollisionManager::GetInstance()->AddCollider(this);
	ColInit();
}

void Collider::Release()
{
	//CollisionManager::GetInstance()->UnregisterCollider(this);
	MainFrame::GetInstance()->GetBox2dWorld()->DestroyBody(m_body);
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

