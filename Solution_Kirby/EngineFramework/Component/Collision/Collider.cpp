#include "pch.h"
#include "Collider.h"
#include "SceneJsonUtility.h"

void Collider::RenderCollider()
{
		if (m_body == nullptr)
			return;

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
	if (m_body == nullptr)
		return;

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

	if (m_body != nullptr)
	{
		MainFrame::GetInstance()->GetBox2dWorld()->DestroyBody(m_body);
		m_body = nullptr;
	}

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
	fixtureDef.isSensor = m_isTrigger;
	m_body->CreateFixture(&fixtureDef);
	//m_body->SetGravityScale(30.0);
	m_body->GetUserData().pointer = (uintptr_t)this;
}

void Collider::SetBodyType(b2BodyType type)
{
	if (m_type == type)
	{
		return;
	}

	const bool fixedRotation = GetFixedRotation();
	m_type = type;
	if (m_body != nullptr)
	{
		CreateBody(m_colOffset, m_colSize, fixedRotation);
	}
}

void Collider::SetColliderSize(Vector2D size)
{
	if (size.x < 0.0f)
	{
		size.x = 0.0f;
	}
	if (size.y < 0.0f)
	{
		size.y = 0.0f;
	}

	const bool fixedRotation = GetFixedRotation();
	if (m_body != nullptr && size.x > 0.0f && size.y > 0.0f)
	{
		CreateBody(m_colOffset, size, fixedRotation);
		return;
	}
	m_colSize = size;
}

void Collider::SetColliderOffset(Vector2D offset)
{
	const bool fixedRotation = GetFixedRotation();
	if (m_body != nullptr && m_colSize.x > 0.0f && m_colSize.y > 0.0f)
	{
		CreateBody(offset, m_colSize, fixedRotation);
		return;
	}
	m_colOffset = offset;
}

void Collider::SetFixedRotation(bool fixedRotation)
{
	if (m_body != nullptr)
	{
		m_body->SetFixedRotation(fixedRotation);
	}
}

bool Collider::GetFixedRotation() const
{
	return m_body != nullptr && m_body->IsFixedRotation();
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
	if (m_body != nullptr)
	{
		MainFrame::GetInstance()->GetBox2dWorld()->DestroyBody(m_body);
		m_body = nullptr;
	}
	RenderManager::GetInstance()->UnregisterDebug(this);

	ColRelease();
}

void Collider::SetTrigger(bool b)
{
	m_isTrigger = b;
	if (m_body != nullptr)
	{
		for (b2Fixture* fixture = m_body->GetFixtureList(); fixture != nullptr; fixture = fixture->GetNext())
		{
			fixture->SetSensor(m_isTrigger);
		}
	}
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

const char* Collider::GetInspectorName() const
{
	return "Collider";
}

void Collider::DrawInspector()
{
	int bodyType = static_cast<int>(m_type);
	const char* bodyTypeNames[] = { "Static", "Kinematic", "Dynamic" };
	if (ImGui::Combo("Body Type", &bodyType, bodyTypeNames, IM_ARRAYSIZE(bodyTypeNames)))
	{
		SetBodyType(static_cast<b2BodyType>(bodyType));
	}

	bool trigger = m_isTrigger;
	if (ImGui::Checkbox("Trigger", &trigger))
	{
		SetTrigger(trigger);
	}

	bool fixedRotation = GetFixedRotation();
	if (ImGui::Checkbox("Fixed Rotation", &fixedRotation))
	{
		SetFixedRotation(fixedRotation);
	}

	float size[2] = { m_colSize.x, m_colSize.y };
	if (ImGui::DragFloat2("Size", size, 1.0f, 0.0f, 10000.0f))
	{
		SetColliderSize({ size[0], size[1] });
	}

	float offset[2] = { m_colOffset.x, m_colOffset.y };
	if (ImGui::DragFloat2("Offset", offset, 1.0f))
	{
		SetColliderOffset({ offset[0], offset[1] });
	}

	ImGui::Text("Body: %s", m_body ? "Created" : "None");

	if (m_body)
	{
		const b2Vec2 position = m_body->GetPosition();
		const b2Vec2 velocity = m_body->GetLinearVelocity();
		ImGui::Text("Body Active: %s", m_body->IsEnabled() ? "true" : "false");
		ImGui::Text("Fixed Rotation: %s", m_body->IsFixedRotation() ? "true" : "false");
		ImGui::Text("Body Position: %.2f, %.2f", position.x, position.y);
		ImGui::Text("Body Angle: %.3f", m_body->GetAngle());
		ImGui::Text("Linear Velocity: %.2f, %.2f", velocity.x, velocity.y);
	}
}

std::string Collider::Serialize() const
{
	std::ostringstream oss;
	oss << "{ ";
	oss << "\"bodyType\": " << static_cast<int>(m_type) << ", ";
	oss << "\"trigger\": " << (m_isTrigger ? "true" : "false") << ", ";
	oss << "\"size\": { \"x\": " << m_colSize.x << ", \"y\": " << m_colSize.y << " }, ";
	oss << "\"offset\": { \"x\": " << m_colOffset.x << ", \"y\": " << m_colOffset.y << " }, ";
	oss << "\"fixedRotation\": " << (m_body != nullptr && m_body->IsFixedRotation() ? "true" : "false");
	oss << " }";
	return oss.str();
}

bool Collider::Deserialize(const std::string& componentJson)
{
	int bodyType = static_cast<int>(m_type);
	SceneJson::ReadInt(componentJson, "bodyType", bodyType);
	m_type = static_cast<b2BodyType>(bodyType);
	SceneJson::ReadBool(componentJson, "trigger", m_isTrigger);

	D3DXVECTOR2 size = { static_cast<float>(m_colSize.x), static_cast<float>(m_colSize.y) };
	D3DXVECTOR2 offset = { static_cast<float>(m_colOffset.x), static_cast<float>(m_colOffset.y) };
	SceneJson::ReadVector2(componentJson, "size", &size);
	SceneJson::ReadVector2(componentJson, "offset", &offset);

	bool fixedRotation = false;
	SceneJson::ReadBool(componentJson, "fixedRotation", fixedRotation);

	if (size.x > 0.0f && size.y > 0.0f && m_gameObj != nullptr && m_body == nullptr)
	{
		CreateBody({ offset.x, offset.y }, { size.x, size.y }, fixedRotation);
	}
	else
	{
		m_colSize = { size.x, size.y };
		m_colOffset = { offset.x, offset.y };
	}
	return true;
}

