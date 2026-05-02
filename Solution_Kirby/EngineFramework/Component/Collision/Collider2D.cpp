#include "pch.h"
#include "Collider2D.h"
#include "Rigidbody2D.h"
#include "SceneJsonUtility.h"

namespace
{
	bool IsBodyInWorld(b2World* world, b2Body* body)
	{
		if (world == nullptr || body == nullptr)
		{
			return false;
		}

		for (b2Body* itr = world->GetBodyList(); itr != nullptr; itr = itr->GetNext())
		{
			if (itr == body)
			{
				return true;
			}
		}

		return false;
	}

	bool IsFixtureAttachedToBody(b2Body* body, b2Fixture* fixture)
	{
		if (body == nullptr || fixture == nullptr)
		{
			return false;
		}

		for (b2Fixture* itr = body->GetFixtureList(); itr != nullptr; itr = itr->GetNext())
		{
			if (itr == fixture)
			{
				return true;
			}
		}

		return false;
	}

	int CountCollidersOnGameObject(GameObject* gameObject)
	{
		if (gameObject == nullptr)
		{
			return 0;
		}

		vector<Component*>* components = gameObject->GetComponentVec();
		if (components == nullptr)
		{
			return 0;
		}

		int colliderCount = 0;
		for (vector<Component*>::iterator itr = components->begin(); itr != components->end(); itr++)
		{
			if (dynamic_cast<Collider2D*>(*itr) != nullptr)
			{
				++colliderCount;
			}
		}

		return colliderCount;
	}
}

void Collider2D::RenderCollider()
{
		// base Collider2D debug render는 box outline 전용이다.
		if (!CanRenderDebugCollider())
			return;

		D3DXMATRIX matTranslate, matScale, matRotate, matWorld;

		b2Vec2 position = m_body->GetPosition();
		float renderX = position.x;
		float renderY = position.y;
		if (!m_ownsBody)
		{
			const float angle = m_body->GetAngle();
			const float cosAngle = cosf(angle);
			const float sinAngle = sinf(angle);
			renderX += (m_colOffset.x * cosAngle) - (m_colOffset.y * sinAngle);
			renderY += (m_colOffset.x * sinAngle) + (m_colOffset.y * cosAngle);
		}

		D3DXMatrixTranslation(&matTranslate, renderX, renderY, 0.0f);
		D3DXMatrixScaling(&matScale, m_colSize.x + 1, m_colSize.y + 1, 1.0f);
		D3DXMatrixRotationZ(&matRotate, m_body->GetAngle());
		matWorld = matScale * matRotate * matTranslate;

		MainFrame::GetInstance()->GetDevice()->SetTexture(0, nullptr);
		MainFrame::GetInstance()->GetDevice()->SetTransform(D3DTS_WORLD, &matWorld);
		MainFrame::GetInstance()->GetDevice()->SetFVF(D3DFVF_DEBUGVERTEX);

		const D3DCOLOR debugColor = GetDebugRenderColor();
		DEBUGVERTEX vertices[] =
		{
			{-0.5f, -0.5f, 0.0f, debugColor},
			{ 0.5f, -0.5f, 0.0f, debugColor},
			{ 0.5f, -0.5f, 0.0f, debugColor},
			{ 0.5f,  0.5f, 0.0f, debugColor},
			{ 0.5f,  0.5f, 0.0f, debugColor},
			{-0.5f,  0.5f, 0.0f, debugColor},
			{-0.5f,  0.5f, 0.0f, debugColor},
			{-0.5f, -0.5f, 0.0f, debugColor},
		};

		MainFrame::GetInstance()->GetDevice()->DrawPrimitiveUP(D3DPT_LINELIST, 4, vertices, sizeof(DEBUGVERTEX));
}

Collider2D::Collider2D(b2BodyType type)
	:m_type(type)
{
}
void Collider2D::Start()
{
	if (m_gameObj == nullptr)
	{
		return;
	}

	if (m_colSize.x <= 0.0f || m_colSize.y <= 0.0f)
	{
		const D3DXVECTOR2 objectSize = m_gameObj->Size2D();
		if (m_colSize.x <= 0.0f)
		{
			m_colSize.x = objectSize.x;
		}
		if (m_colSize.y <= 0.0f)
		{
			m_colSize.y = objectSize.y;
		}
	}

	if (m_body == nullptr && m_colSize.x > 0.0f && m_colSize.y > 0.0f)
	{
		CreateBody(m_colOffset, m_colSize, GetFixedRotation());
	}
}
void Collider2D::Update()
{
	if (m_body == nullptr || !m_body->IsEnabled())
		return;

	m_gameObj->SetPosition({
		m_body->GetPosition().x,
		m_body->GetPosition().y,
		m_gameObj->Position().z });

	m_gameObj->SetAngleZ(m_body->GetAngle());

}
const Vector2D& Collider2D::GetColSize()
{
	return m_colSize;
}

const Vector2D& Collider2D::GetColOffset()
{
	return m_colOffset;
}

void Collider2D::CreateBody(Vector2D offset, Vector2D size, bool fixedRotation)
{
	m_colSize = size;
	m_colOffset = offset;

	b2World* world = MainFrame::GetInstance() != nullptr ? MainFrame::GetInstance()->GetBox2dWorld() : nullptr;
	const bool hasLiveBody = IsBodyInWorld(world, m_body);

	if (m_fixture != nullptr && hasLiveBody && IsFixtureAttachedToBody(m_body, m_fixture))
	{
		m_body->DestroyFixture(m_fixture);
	}
	m_fixture = nullptr;

	if (m_ownsBody && hasLiveBody)
	{
		world->DestroyBody(m_body);
		m_body = nullptr;
		m_ownsBody = false;
	}
	else if (!hasLiveBody)
	{
		m_body = nullptr;
		m_ownsBody = false;
	}

	Rigidbody2D* rigidbody2D = m_gameObj != nullptr ? m_gameObj->GetComponent<Rigidbody2D>() : nullptr;
	if (rigidbody2D != nullptr)
	{
		if (rigidbody2D->GetBody() == nullptr)
		{
			rigidbody2D->CreateBody(false);
		}
		m_body = rigidbody2D->GetBody();
		m_ownsBody = false;
	}
	else
	{
		b2BodyDef bodyDef;
		bodyDef.type = m_type;
		bodyDef.fixedRotation = fixedRotation;
		bodyDef.position.Set(
			m_gameObj->Position().x + (float)m_colOffset.x,
			m_gameObj->Position().y + (float)m_colOffset.y);
		bodyDef.angle = 0.0f;
		m_body = MainFrame::GetInstance()->GetBox2dWorld()->CreateBody(&bodyDef);
		m_ownsBody = true;
	}

	if (m_body == nullptr)
	{
		return;
	}

	m_fixture = CreateFixtureOnBody(m_body);
	if (m_fixture != nullptr)
	{
		m_fixture->GetUserData().pointer = reinterpret_cast<uintptr_t>(this);
	}

	if (!m_ownsBody)
	{
		m_body->ResetMassData();
		if (m_gameObj == nullptr || m_gameObj->GetActive())
		{
			m_body->SetAwake(true);
		}
	}

	//m_body->SetGravityScale(30.0);
	m_body->GetUserData().pointer = (uintptr_t)this;
	if (m_gameObj != nullptr && !m_gameObj->GetActive())
	{
		m_body->SetEnabled(false);
	}
}

void Collider2D::SetBodyType(b2BodyType type)
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

void Collider2D::SetColliderSize(Vector2D size)
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

void Collider2D::SetColliderOffset(Vector2D offset)
{
	const bool fixedRotation = GetFixedRotation();
	if (m_body != nullptr && m_colSize.x > 0.0f && m_colSize.y > 0.0f)
	{
		CreateBody(offset, m_colSize, fixedRotation);
		return;
	}
	m_colOffset = offset;
}

void Collider2D::SetFixedRotation(bool fixedRotation)
{
	if (m_body != nullptr)
	{
		m_body->SetFixedRotation(fixedRotation);
	}
}

bool Collider2D::GetFixedRotation() const
{
	return m_body != nullptr && m_body->IsFixedRotation();
}

void Collider2D::Initialize()
{
	//CollisionManager::GetInstance()->AddCollider(this);
	ColInit();
	RenderManager::GetInstance()->RegisterDebug(this);
}

void Collider2D::Release()
{
	//CollisionManager::GetInstance()->UnregisterCollider(this);
	b2World* world = MainFrame::GetInstance() != nullptr ? MainFrame::GetInstance()->GetBox2dWorld() : nullptr;
	if (MainFrame::GetInstance() != nullptr)
	{
		MainFrame::GetInstance()->RemoveCollisionPairsForCollider(this);
		MainFrame::GetInstance()->RemoveTriggerPairsForCollider(this);
	}
	const bool hasLiveBody = IsBodyInWorld(world, m_body);

	if (hasLiveBody && m_body->GetUserData().pointer == reinterpret_cast<uintptr_t>(this))
	{
		m_body->GetUserData().pointer = 0;
	}
	if (m_fixture != nullptr && hasLiveBody && IsFixtureAttachedToBody(m_body, m_fixture))
	{
		m_fixture->GetUserData().pointer = 0;
		m_body->DestroyFixture(m_fixture);
	}
	m_fixture = nullptr;
	if (m_ownsBody && hasLiveBody)
	{
		world->DestroyBody(m_body);
	}
	m_body = nullptr;
	m_ownsBody = false;
	RenderManager::GetInstance()->UnregisterDebug(this);

	ColRelease();
}

void Collider2D::SetTrigger(bool b)
{
	if (m_isTrigger == b)
	{
		return;
	}

	if (MainFrame::GetInstance() != nullptr)
	{
		// Trigger 상태 전환은 기존 collision/trigger contact를 강제 종료시킨다.
		MainFrame::GetInstance()->ResetCollisionPairsForCollider(this);
		MainFrame::GetInstance()->ResetTriggerPairsForCollider(this);
	}

	m_isTrigger = b;
	if (m_fixture != nullptr)
	{
		m_fixture->SetSensor(m_isTrigger);
		m_fixture->Refilter();
	}
	if (m_body != nullptr)
	{
		m_body->SetAwake(true);
	}
}

bool Collider2D::GetTrigger()
{
	return m_isTrigger;
}

b2Body* Collider2D::GetBody()
{
	return m_body;
}

void Collider2D::SetPhysicsActive(bool active)
{
	if (m_body == nullptr)
	{
		return;
	}

	if (active)
	{
		SyncBodyToGameObjectTransform();
		m_body->SetEnabled(true);
		if (!m_ownsBody)
		{
			m_body->ResetMassData();
		}
		if (m_fixture != nullptr)
		{
			m_fixture->Refilter();
		}
		if (m_body->GetType() != b2_staticBody)
		{
			m_body->SetAwake(true);
		}
		return;
	}

	m_body->SetEnabled(false);
}

void Collider2D::SyncBodyToGameObjectTransform()
{
	if (m_body == nullptr || m_gameObj == nullptr)
	{
		return;
	}

	if (m_ownsBody)
	{
		m_body->SetTransform(
			{ m_gameObj->Position().x + static_cast<float>(m_colOffset.x),
			  m_gameObj->Position().y + static_cast<float>(m_colOffset.y) },
			0.0f);
	}
	else
	{
		m_body->SetTransform(
			{ m_gameObj->Position().x, m_gameObj->Position().y },
			m_gameObj->GetAngleZ());
	}

	if (m_body->GetType() != b2_staticBody)
	{
		m_body->SetAwake(true);
	}
}

void Collider2D::ClearBodyReferenceIfMatches(b2Body* body)
{
	if (m_body != body)
	{
		return;
	}

	m_body = nullptr;
	m_fixture = nullptr;
	m_ownsBody = false;
}

bool Collider2D::OwnsBody() const
{
	return m_ownsBody;
}

bool Collider2D::CanRenderDebugCollider() const
{
	if (m_body == nullptr || !m_body->IsEnabled())
	{
		return false;
	}

	if (m_gameObj == nullptr || !m_gameObj->GetActive() || m_gameObj->GetDestroy())
	{
		return false;
	}

	if (MainFrame::GetInstance() == nullptr || MainFrame::GetInstance()->GetDevice() == nullptr)
	{
		return false;
	}

	return true;
}

D3DCOLOR Collider2D::GetDebugRenderColor() const
{
	if (m_isTrigger)
	{
		return D3DCOLOR_XRGB(0, 160, 0);
	}

	return DEBUGCOLORDX1;
}

void Collider2D::DebugRenderUpdate()
{
	RenderCollider();
}

const char* Collider2D::GetInspectorName() const
{
	return "Collider 2D";
}

void Collider2D::DrawInspector()
{
	Rigidbody2D* rigidbody2D = m_gameObj != nullptr ? m_gameObj->GetComponent<Rigidbody2D>() : nullptr;
	if (rigidbody2D == nullptr)
	{
		if (m_type != b2_staticBody)
		{
			SetBodyType(b2_staticBody);
		}
		ImGui::TextDisabled("Body Type: Static");
	}
	else
	{
		ImGui::TextDisabled("Body Type is controlled by Rigidbody 2D");
	}

	if (rigidbody2D == nullptr && CountCollidersOnGameObject(m_gameObj) >= 2)
	{
		ImGui::Spacing();
		ImGui::TextColored(
			ImVec4(1.0f, 0.8f, 0.2f, 1.0f),
			"Multiple colliders without Rigidbody2D are not recommended.");
		ImGui::TextDisabled("Add Rigidbody2D to share one physics body.");
	}

	bool trigger = m_isTrigger;
	if (ImGui::Checkbox("Trigger", &trigger))
	{
		SetTrigger(trigger);
	}

	if (rigidbody2D == nullptr)
	{
		bool fixedRotation = GetFixedRotation();
		if (ImGui::Checkbox("Fixed Rotation", &fixedRotation))
		{
			SetFixedRotation(fixedRotation);
		}
	}
	else
	{
		ImGui::TextDisabled("Fixed Rotation is controlled by Rigidbody 2D");
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

std::string Collider2D::Serialize() const
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

bool Collider2D::Deserialize(const std::string& componentJson)
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


