#include "pch.h"
#include "Collider.h"

void CheckBody(b2Body* body)
{
	if (body->IsFixedRotation()) 
	{
		body->SetFixedRotation(false);  // 회전을 고정하지 않도록 설정
	}

	b2MassData massData;
	massData = body->GetMassData();

	if (massData.I < 1e-5) 
	{
		massData.I = 1.0f;  // 적절한 값으로 설정
		body->SetMassData(&massData);
	}

	b2Fixture* fixture = body->GetFixtureList();
	while (fixture)
	{
		float friction = fixture->GetFriction();
		if (friction < 0.1f)
		{
			fixture->SetFriction(0.5f);  // 적절한 값으로 설정
		}
		fixture = fixture->GetNext();
	}
}

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
		{-0.5f, -0.5f, 0.0f, DEBUGCOLORDX2},
		{ 0.5f, -0.5f, 0.0f, DEBUGCOLORDX2},
		{ 0.5f, -0.5f, 0.0f, DEBUGCOLORDX2},
		{ 0.5f,  0.5f, 0.0f, DEBUGCOLORDX2},
		{ 0.5f,  0.5f, 0.0f, DEBUGCOLORDX2},
		{-0.5f,  0.5f, 0.0f, DEBUGCOLORDX2},
		{-0.5f,  0.5f, 0.0f, DEBUGCOLORDX2},
		{-0.5f, -0.5f, 0.0f, DEBUGCOLORDX2},
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

	m_gameObj->SetAngle(m_body->GetAngle());

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
	bodyDef.fixedRotation = false;
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

	/*float inertia = m_body->GetInertia();
	b2MassData data;
	data.center = b2Vec2(0, 0);
	data.mass = 1.0f;
	data.I = inertia;
	m_body->SetMassData(&data);*/

	m_body->SetFixedRotation(false);
	//m_body->SetGravityScale(30.0);
	m_body->GetUserData().pointer = (uintptr_t)this;

	CheckBody(m_body);
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
	m_body = nullptr;
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

