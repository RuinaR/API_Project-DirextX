#include "pch.h"
#include "Rigidbody2D.h"

#include "Collider2D.h"
#include "GameObject.h"
#include "MainFrame.h"
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

	const char* ToBodyTypeString(b2BodyType bodyType)
	{
		switch (bodyType)
		{
		case b2_kinematicBody:
			return "Kinematic";
		case b2_dynamicBody:
			return "Dynamic";
		case b2_staticBody:
		default:
			return "Static";
		}
	}

	b2BodyType ParseBodyTypeString(const std::string& bodyType)
	{
		if (bodyType == "Kinematic")
		{
			return b2_kinematicBody;
		}
		if (bodyType == "Dynamic")
		{
			return b2_dynamicBody;
		}
		return b2_staticBody;
	}

	const char* ToCollisionDetectionString(Rigidbody2DCollisionDetection collisionDetection)
	{
		switch (collisionDetection)
		{
		case Rigidbody2DCollisionDetection::Continuous:
			return "Continuous";
		case Rigidbody2DCollisionDetection::Discrete:
		default:
			return "Discrete";
		}
	}

	Rigidbody2DCollisionDetection ParseCollisionDetectionString(const std::string& collisionDetection)
	{
		if (collisionDetection == "Continuous")
		{
			return Rigidbody2DCollisionDetection::Continuous;
		}
		return Rigidbody2DCollisionDetection::Discrete;
	}

	const char* ToInterpolationString(Rigidbody2DInterpolation interpolate)
	{
		switch (interpolate)
		{
		case Rigidbody2DInterpolation::Interpolate:
			return "Interpolate";
		case Rigidbody2DInterpolation::Extrapolate:
			return "Extrapolate";
		case Rigidbody2DInterpolation::None:
		default:
			return "None";
		}
	}

	Rigidbody2DInterpolation ParseInterpolationString(const std::string& interpolate)
	{
		if (interpolate == "Interpolate")
		{
			return Rigidbody2DInterpolation::Interpolate;
		}
		if (interpolate == "Extrapolate")
		{
			return Rigidbody2DInterpolation::Extrapolate;
		}
		return Rigidbody2DInterpolation::None;
	}

	bool CanApplyVelocityToBody(b2Body* body)
	{
		return body != nullptr && body->GetType() != b2_staticBody;
	}

	bool CanApplyDynamicForce(b2Body* body)
	{
		return body != nullptr && body->GetType() == b2_dynamicBody;
	}
}

Rigidbody2D::Rigidbody2D()
{
}

void Rigidbody2D::Initialize()
{
}

void Rigidbody2D::Release()
{
	b2World* world = MainFrame::GetInstance() != nullptr ? MainFrame::GetInstance()->GetBox2dWorld() : nullptr;
	const bool shouldRestoreColliderFallback =
		m_gameObj != nullptr &&
		m_gameObj->GetComponent<Rigidbody2D>() == nullptr &&
		!m_gameObj->GetDestroy();

	b2Body* releasedBody = m_body;
	if (IsBodyInWorld(world, m_body))
	{
		world->DestroyBody(m_body);
	}
	m_body = nullptr;

	if (releasedBody != nullptr && m_gameObj != nullptr)
	{
		vector<Component*>* components = m_gameObj->GetComponentVec();
		if (components != nullptr)
		{
			for (vector<Component*>::iterator itr = components->begin(); itr != components->end(); itr++)
			{
				Collider2D* collider = dynamic_cast<Collider2D*>(*itr);
				if (collider != nullptr)
				{
					collider->ClearBodyReferenceIfMatches(releasedBody);
					if (shouldRestoreColliderFallback)
					{
						const Vector2D size = collider->GetColSize();
						if (size.x > 0.0f && size.y > 0.0f)
						{
							collider->CreateBody(collider->GetColOffset(), size, false);
						}
					}
				}
			}
		}
	}
}

void Rigidbody2D::Start()
{
	if (m_body == nullptr)
	{
		CreateBody();
	}
}

void Rigidbody2D::Update()
{
	if (m_body == nullptr || m_gameObj == nullptr || !m_body->IsEnabled())
	{
		return;
	}

	const b2Vec2 position = m_body->GetPosition();
	m_gameObj->SetPosition(D3DXVECTOR3(position.x, position.y, m_gameObj->Position().z));
	m_gameObj->SetAngleZ(m_body->GetAngle());
}

void Rigidbody2D::CreateBody(bool rebuildAttachedColliders)
{
	b2World* world = MainFrame::GetInstance() != nullptr ? MainFrame::GetInstance()->GetBox2dWorld() : nullptr;
	if (m_gameObj == nullptr || world == nullptr)
	{
		return;
	}

	b2Body* previousBody = m_body;
	if (previousBody != nullptr && m_gameObj != nullptr)
	{
		vector<Component*>* components = m_gameObj->GetComponentVec();
		if (components != nullptr)
		{
			for (vector<Component*>::iterator itr = components->begin(); itr != components->end(); itr++)
			{
				Collider2D* collider = dynamic_cast<Collider2D*>(*itr);
				if (collider != nullptr)
				{
					collider->ClearBodyReferenceIfMatches(previousBody);
				}
			}
		}
	}

	if (IsBodyInWorld(world, m_body))
	{
		world->DestroyBody(m_body);
		m_body = nullptr;
	}
	else
	{
		m_body = nullptr;
	}

	b2BodyDef bodyDef;
	bodyDef.type = m_bodyType;
	bodyDef.position.Set(m_gameObj->Position().x, m_gameObj->Position().y);
	bodyDef.angle = m_gameObj->GetAngleZ();
	m_body = world->CreateBody(&bodyDef);
	ApplyBodySettings();
	if (m_gameObj != nullptr && !m_gameObj->GetActive())
	{
		m_body->SetEnabled(false);
	}

	if (rebuildAttachedColliders)
	{
		RebuildAttachedColliders();
	}
}

void Rigidbody2D::ApplyBodySettings()
{
	if (m_body == nullptr)
	{
		return;
	}

	const bool lockRotation = m_fixedRotation || m_freezeRotationZ;
	m_body->SetType(m_bodyType);
	m_body->SetLinearDamping(m_drag);
	m_body->SetAngularDamping(m_angularDrag);
	m_body->SetGravityScale(m_useGravity ? m_gravityScale : 0.0f);
	m_body->SetFixedRotation(lockRotation);
	m_body->SetBullet(m_collisionDetection == Rigidbody2DCollisionDetection::Continuous);
	if (CanApplyVelocityToBody(m_body))
	{
		m_body->SetLinearVelocity(b2Vec2(m_linearVelocity.x, m_linearVelocity.y));
		m_body->SetAngularVelocity(lockRotation ? 0.0f : m_angularVelocity);
	}
}

void Rigidbody2D::RebuildAttachedColliders()
{
	if (m_gameObj == nullptr || m_body == nullptr)
	{
		return;
	}

	vector<Component*>* components = m_gameObj->GetComponentVec();
	if (components == nullptr)
	{
		return;
	}

	for (vector<Component*>::iterator itr = components->begin(); itr != components->end(); itr++)
	{
		Collider2D* collider = dynamic_cast<Collider2D*>(*itr);
		if (collider == nullptr)
		{
			continue;
		}

		const Vector2D size = collider->GetColSize();
		if (size.x <= 0.0f || size.y <= 0.0f)
		{
			continue;
		}

		collider->CreateBody(collider->GetColOffset(), size, false);
	}
}

void Rigidbody2D::SetPhysicsActive(bool active)
{
	if (m_body == nullptr)
	{
		return;
	}

	if (active)
	{
		SyncBodyToGameObjectTransform();
		m_body->SetEnabled(true);
		ApplyBodySettings();
		if (m_body->GetType() != b2_staticBody)
		{
			m_body->SetAwake(true);
		}
		return;
	}

	m_body->SetEnabled(false);
}

void Rigidbody2D::SyncBodyToGameObjectTransform()
{
	if (m_body == nullptr || m_gameObj == nullptr)
	{
		return;
	}

	m_body->SetTransform(
		{ m_gameObj->Position().x, m_gameObj->Position().y },
		m_gameObj->GetAngleZ());

	if (m_body->GetType() != b2_staticBody)
	{
		m_body->SetAwake(true);
	}
}

void Rigidbody2D::SetVelocity(const D3DXVECTOR2& velocity)
{
	m_linearVelocity = velocity;
	if (CanApplyVelocityToBody(m_body))
	{
		m_body->SetLinearVelocity(b2Vec2(velocity.x, velocity.y));
	}
}

D3DXVECTOR2 Rigidbody2D::GetVelocity() const
{
	if (m_body != nullptr)
	{
		const b2Vec2 velocity = m_body->GetLinearVelocity();
		return D3DXVECTOR2(velocity.x, velocity.y);
	}

	return m_linearVelocity;
}

void Rigidbody2D::SetAngularVelocity(float angularVelocity)
{
	const bool lockRotation = m_fixedRotation || m_freezeRotationZ;
	m_angularVelocity = lockRotation ? 0.0f : angularVelocity;
	if (CanApplyVelocityToBody(m_body))
	{
		m_body->SetAngularVelocity(lockRotation ? 0.0f : angularVelocity);
	}
}

float Rigidbody2D::GetAngularVelocity() const
{
	if (m_body != nullptr)
	{
		return m_body->GetAngularVelocity();
	}

	return m_angularVelocity;
}

void Rigidbody2D::AddForce(const D3DXVECTOR2& force)
{
	// force는 dynamic body에서만 적용된다.
	if (!CanApplyDynamicForce(m_body))
	{
		return;
	}

	m_body->ApplyForceToCenter(b2Vec2(force.x, force.y), true);
}

void Rigidbody2D::AddImpulse(const D3DXVECTOR2& impulse)
{
	// impulse는 dynamic body에서만 중심 기준으로 적용된다.
	if (!CanApplyDynamicForce(m_body))
	{
		return;
	}

	m_body->ApplyLinearImpulseToCenter(b2Vec2(impulse.x, impulse.y), true);
}

void Rigidbody2D::AddTorque(float torque)
{
	// torque는 dynamic body에서만 적용된다.
	if (!CanApplyDynamicForce(m_body))
	{
		return;
	}

	m_body->ApplyTorque(torque, true);
}

b2Body* Rigidbody2D::GetBody() const
{
	return m_body;
}

const char* Rigidbody2D::GetInspectorName() const
{
	return "Rigidbody 2D";
}

void Rigidbody2D::DrawInspector()
{
	bool needsApplySettings = false;

	int bodyType = static_cast<int>(m_bodyType);
	const char* bodyTypeNames[] = { "Static", "Kinematic", "Dynamic" };
	if (ImGui::Combo("Body Type", &bodyType, bodyTypeNames, IM_ARRAYSIZE(bodyTypeNames)))
	{
		m_bodyType = static_cast<b2BodyType>(bodyType);
		if (m_body == nullptr)
		{
			CreateBody();
		}
		else
		{
			m_body->SetType(m_bodyType);
		}
		needsApplySettings = true;
	}

	if (m_bodyType == b2_staticBody)
	{
		ImGui::TextDisabled("Static: Velocity is cached only. Force / impulse / torque are ignored.");
	}
	else if (m_bodyType == b2_kinematicBody)
	{
		ImGui::TextDisabled("Kinematic: Velocity is applied. Force / impulse / torque are ignored.");
	}
	else
	{
		ImGui::TextDisabled("Dynamic: Velocity, force, impulse, and torque are applied.");
	}

	if (ImGui::DragFloat("Drag", &m_drag, 0.01f, 0.0f, 100.0f, "%.2f"))
	{
		needsApplySettings = true;
	}

	if (ImGui::DragFloat("Angular Drag", &m_angularDrag, 0.01f, 0.0f, 100.0f, "%.2f"))
	{
		needsApplySettings = true;
	}

	if (ImGui::Checkbox("Use Gravity", &m_useGravity))
	{
		needsApplySettings = true;
	}

	if (ImGui::DragFloat("Gravity Scale", &m_gravityScale, 0.01f, 0.0f, 100.0f, "%.2f"))
	{
		needsApplySettings = true;
	}

	bool fixedRotation = m_fixedRotation || m_freezeRotationZ;
	if (ImGui::Checkbox("Fixed Rotation", &fixedRotation))
	{
		m_fixedRotation = fixedRotation;
		m_freezeRotationZ = fixedRotation;
		needsApplySettings = true;
	}

	float linearVelocity[2] = { m_linearVelocity.x, m_linearVelocity.y };
	if (ImGui::DragFloat2("Linear Velocity", linearVelocity, 0.1f))
	{
		SetVelocity(D3DXVECTOR2(linearVelocity[0], linearVelocity[1]));
		needsApplySettings = false;
	}

	if (ImGui::DragFloat("Angular Velocity", &m_angularVelocity, 0.1f))
	{
		SetAngularVelocity(m_angularVelocity);
		needsApplySettings = false;
	}

	int collisionDetection = static_cast<int>(m_collisionDetection);
	const char* collisionDetectionNames[] = { "Discrete", "Continuous" };
	if (ImGui::Combo("Collision Detection", &collisionDetection, collisionDetectionNames, IM_ARRAYSIZE(collisionDetectionNames)))
	{
		m_collisionDetection = static_cast<Rigidbody2DCollisionDetection>(collisionDetection);
		needsApplySettings = true;
	}

	int interpolate = static_cast<int>(m_interpolate);
	const char* interpolateNames[] = { "None", "Interpolate", "Extrapolate" };
	if (ImGui::Combo("Interpolate", &interpolate, interpolateNames, IM_ARRAYSIZE(interpolateNames)))
	{
		m_interpolate = static_cast<Rigidbody2DInterpolation>(interpolate);
	}

	ImGui::Separator();
	ImGui::Text("Constraints");

	bool freezeRotationZ = m_freezeRotationZ;
	if (ImGui::Checkbox("Freeze Rotate Z", &freezeRotationZ))
	{
		m_freezeRotationZ = freezeRotationZ;
		m_fixedRotation = freezeRotationZ;
		if (freezeRotationZ)
		{
			m_angularVelocity = 0.0f;
			if (m_body != nullptr)
			{
				m_body->SetAngularVelocity(0.0f);
			}
		}
		needsApplySettings = true;
	}

	if (needsApplySettings)
	{
		ApplyBodySettings();
	}

	ImGui::Text("Body: %s", m_body != nullptr ? "Created" : "None");
	if (m_body != nullptr)
	{
		const b2Vec2 bodyVelocity = m_body->GetLinearVelocity();
		ImGui::Text("Runtime Velocity: %.2f, %.2f", bodyVelocity.x, bodyVelocity.y);
		ImGui::Text("Runtime Angular Velocity: %.2f", m_body->GetAngularVelocity());
	}
}

const char* Rigidbody2D::GetSerializableType() const
{
	return "Rigidbody2D";
}

std::string Rigidbody2D::Serialize() const
{
	std::ostringstream oss;
	oss << "{ ";
	oss << "\"bodyType\": \"" << ToBodyTypeString(m_bodyType) << "\", ";
	oss << "\"drag\": " << m_drag << ", ";
	oss << "\"angularDrag\": " << m_angularDrag << ", ";
	oss << "\"useGravity\": " << (m_useGravity ? "true" : "false") << ", ";
	oss << "\"gravityScale\": " << m_gravityScale << ", ";
	oss << "\"fixedRotation\": " << (m_fixedRotation ? "true" : "false") << ", ";
	oss << SceneJson::WriteVector2("linearVelocity", &m_linearVelocity) << ", ";
	oss << "\"angularVelocity\": " << m_angularVelocity << ", ";
	oss << "\"collisionDetection\": \"" << ToCollisionDetectionString(m_collisionDetection) << "\", ";
	oss << "\"interpolate\": \"" << ToInterpolationString(m_interpolate) << "\", ";
	oss << "\"freezeRotationZ\": " << (m_freezeRotationZ ? "true" : "false");
	oss << " }";
	return oss.str();
}

bool Rigidbody2D::Deserialize(const std::string& componentJson)
{
	std::string bodyTypeString;
	if (SceneJson::ReadString(componentJson, "bodyType", bodyTypeString))
	{
		m_bodyType = ParseBodyTypeString(bodyTypeString);
	}

	SceneJson::ReadFloat(componentJson, "drag", m_drag);
	SceneJson::ReadFloat(componentJson, "angularDrag", m_angularDrag);
	SceneJson::ReadBool(componentJson, "useGravity", m_useGravity);
	SceneJson::ReadFloat(componentJson, "gravityScale", m_gravityScale);
	SceneJson::ReadBool(componentJson, "fixedRotation", m_fixedRotation);
	SceneJson::ReadVector2(componentJson, "linearVelocity", &m_linearVelocity);
	SceneJson::ReadFloat(componentJson, "angularVelocity", m_angularVelocity);

	std::string collisionDetectionString;
	if (SceneJson::ReadString(componentJson, "collisionDetection", collisionDetectionString))
	{
		m_collisionDetection = ParseCollisionDetectionString(collisionDetectionString);
	}

	std::string interpolateString;
	if (SceneJson::ReadString(componentJson, "interpolate", interpolateString))
	{
		m_interpolate = ParseInterpolationString(interpolateString);
	}

	SceneJson::ReadBool(componentJson, "freezeRotationZ", m_freezeRotationZ);

	if (m_freezeRotationZ)
	{
		m_fixedRotation = true;
		m_angularVelocity = 0.0f;
	}

	if (m_body != nullptr)
	{
		ApplyBodySettings();
	}

	return true;
}

