#include "pch.h"
#include "CircleCollider2D.h"
#include "Rigidbody2D.h"
#include "SceneJsonUtility.h"

namespace
{
	// circle debug render는 가벼운 윤곽 표시가 목적이므로 고정 세그먼트 근사로 유지한다.
	constexpr int kCircleDebugSegmentCount = 32;
}

void CircleCollider2D::ColInit()
{
}

void CircleCollider2D::ColRelease()
{
}

b2Fixture* CircleCollider2D::CreateFixtureOnBody(b2Body* body)
{
	if (body == nullptr)
	{
		return nullptr;
	}

	b2CircleShape circle;
	circle.m_radius = GetRadius();
	if (m_ownsBody)
	{
		circle.m_p.Set(0.0f, 0.0f);
	}
	else
	{
		circle.m_p.Set(m_colOffset.x, m_colOffset.y);
	}

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &circle;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.8f;
	fixtureDef.restitution = 0.0f;
	fixtureDef.isSensor = m_isTrigger;
	return body->CreateFixture(&fixtureDef);
}

CircleCollider2D::CircleCollider2D(b2BodyType type)
	: Collider2D(type)
{
}

CircleCollider2D::~CircleCollider2D()
{
}

float CircleCollider2D::GetRadius() const
{
	return m_colSize.x * 0.5f;
}

void CircleCollider2D::SetRadius(float radius)
{
	if (radius < 0.0f)
	{
		radius = 0.0f;
	}

	SetColliderSize({ radius * 2.0f, radius * 2.0f });
}

void CircleCollider2D::DebugRenderUpdate()
{
	// CircleCollider2D는 base box debug render를 쓰지 않고 원형 윤곽을 직접 그린다.
	if (!CanRenderDebugCollider())
	{
		return;
	}

	D3DXVECTOR3 center(m_body->GetPosition().x, m_body->GetPosition().y, 0.0f);
	if (!m_ownsBody)
	{
		const float angle = m_body->GetAngle();
		const float cosAngle = cosf(angle);
		const float sinAngle = sinf(angle);
		center.x += (m_colOffset.x * cosAngle) - (m_colOffset.y * sinAngle);
		center.y += (m_colOffset.x * sinAngle) + (m_colOffset.y * cosAngle);
	}

	const float radius = GetRadius();
	if (radius <= 0.0f)
	{
		return;
	}

	D3DXMATRIX identityWorld;
	D3DXMatrixIdentity(&identityWorld);
	MainFrame::GetInstance()->GetDevice()->SetTexture(0, nullptr);
	MainFrame::GetInstance()->GetDevice()->SetTransform(D3DTS_WORLD, &identityWorld);
	MainFrame::GetInstance()->GetDevice()->SetFVF(D3DFVF_DEBUGVERTEX);

	const D3DCOLOR debugColor = GetDebugRenderColor();
	DEBUGVERTEX vertices[kCircleDebugSegmentCount * 2];
	const float twoPi = 6.28318530718f;
	for (int i = 0; i < kCircleDebugSegmentCount; i++)
	{
		const float t0 = (static_cast<float>(i) / static_cast<float>(kCircleDebugSegmentCount)) * twoPi;
		const float t1 = (static_cast<float>(i + 1) / static_cast<float>(kCircleDebugSegmentCount)) * twoPi;

		vertices[i * 2] =
		{
			center.x + cosf(t0) * radius,
			center.y + sinf(t0) * radius,
			0.0f,
			debugColor
		};
		vertices[(i * 2) + 1] =
		{
			center.x + cosf(t1) * radius,
			center.y + sinf(t1) * radius,
			0.0f,
			debugColor
		};
	}

	MainFrame::GetInstance()->GetDevice()->DrawPrimitiveUP(
		D3DPT_LINELIST,
		kCircleDebugSegmentCount,
		vertices,
		sizeof(DEBUGVERTEX));
}

const char* CircleCollider2D::GetInspectorName() const
{
	return "Circle Collider 2D";
}

void CircleCollider2D::DrawInspector()
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

	float radius = GetRadius();
	if (ImGui::DragFloat("Radius", &radius, 1.0f, 0.0f, 10000.0f))
	{
		SetRadius(radius);
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

const char* CircleCollider2D::GetSerializableType() const
{
	return "CircleCollider2D";
}

std::string CircleCollider2D::Serialize() const
{
	std::ostringstream oss;
	oss << "{ ";
	oss << "\"bodyType\": " << static_cast<int>(m_type) << ", ";
	oss << "\"trigger\": " << (m_isTrigger ? "true" : "false") << ", ";
	oss << "\"radius\": " << GetRadius() << ", ";
	oss << "\"offset\": { \"x\": " << m_colOffset.x << ", \"y\": " << m_colOffset.y << " }, ";
	oss << "\"fixedRotation\": " << (m_body != nullptr && m_body->IsFixedRotation() ? "true" : "false");
	oss << " }";
	return oss.str();
}

bool CircleCollider2D::Deserialize(const std::string& componentJson)
{
	int bodyType = static_cast<int>(m_type);
	SceneJson::ReadInt(componentJson, "bodyType", bodyType);
	m_type = static_cast<b2BodyType>(bodyType);
	SceneJson::ReadBool(componentJson, "trigger", m_isTrigger);

	float radius = GetRadius();
	D3DXVECTOR2 offset = { static_cast<float>(m_colOffset.x), static_cast<float>(m_colOffset.y) };
	SceneJson::ReadFloat(componentJson, "radius", radius);
	SceneJson::ReadVector2(componentJson, "offset", &offset);

	bool fixedRotation = false;
	SceneJson::ReadBool(componentJson, "fixedRotation", fixedRotation);

	if (radius > 0.0f && m_gameObj != nullptr && m_body == nullptr)
	{
		CreateBody({ offset.x, offset.y }, { radius * 2.0f, radius * 2.0f }, fixedRotation);
	}
	else
	{
		m_colSize = { radius * 2.0f, radius * 2.0f };
		m_colOffset = { offset.x, offset.y };
	}
	return true;
}
