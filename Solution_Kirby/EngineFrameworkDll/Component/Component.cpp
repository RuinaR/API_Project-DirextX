#include "pch.h"
#include "Component.h"
#include "GameObject.h"
#include "SceneJsonUtility.h"

namespace
{
	int& GetNextPersistentComponentIdStorage()
	{
		static int nextComponentId = 1;
		return nextComponentId;
	}

	int GenerateNextPersistentComponentId()
	{
		int& nextComponentId = GetNextPersistentComponentIdStorage();
		return nextComponentId++;
	}

	void SyncPersistentComponentIdAllocator(int componentId)
	{
		if (componentId < 0)
		{
			return;
		}

		int& nextComponentId = GetNextPersistentComponentIdStorage();
		if (componentId >= nextComponentId)
		{
			nextComponentId = componentId + 1;
		}
	}

	bool IsSameComponentType(Component* component, const std::string& typeName)
	{
		if (component == nullptr)
		{
			return false;
		}

		const char* serializableType = component->GetSerializableType();
		if (serializableType != nullptr && strlen(serializableType) > 0)
		{
			return typeName == serializableType;
		}

		const char* inspectorName = component->GetInspectorName();
		if (inspectorName == nullptr)
		{
			return false;
		}

		const std::string inspectorNameString = inspectorName;
		return inspectorNameString == typeName
			|| inspectorNameString == "class " + typeName
			|| inspectorNameString == "struct " + typeName;
	}

	bool IsJsonObjectEffectivelyEmpty(const std::string& json)
	{
		for (std::string::const_iterator itr = json.begin(); itr != json.end(); ++itr)
		{
			if (*itr == '{' || *itr == '}' || isspace(static_cast<unsigned char>(*itr)))
			{
				continue;
			}

			return false;
		}

		return true;
	}

	std::string AppendObjectField(const std::string& baseObjectJson, const std::string& fieldName, const std::string& fieldValueJson)
	{
		if (fieldValueJson.empty())
		{
			return baseObjectJson;
		}

		const size_t closingBracePos = baseObjectJson.find_last_of('}');
		if (closingBracePos == std::string::npos)
		{
			return baseObjectJson;
		}

		const bool isEmptyObject = IsJsonObjectEffectivelyEmpty(baseObjectJson);
		std::ostringstream oss;
		oss << baseObjectJson.substr(0, closingBracePos);
		if (!isEmptyObject)
		{
			oss << ", ";
		}
		else
		{
			oss << " ";
		}
		oss << "\"" << SceneJson::EscapeString(fieldName) << "\": " << fieldValueJson;
		oss << " }";
		return oss.str();
	}
}

void ReferenceFieldRegistry::RegisterGameObjectRef(const char* label, GameObject** field)
{
	if (field == nullptr)
	{
		return;
	}

	GameObjectRefField refField;
	refField.label = label;
	refField.field = field;
	m_gameObjectRefs.push_back(refField);
}

void ReferenceFieldRegistry::RegisterComponentRef(const char* label, Component** field, const char* expectedType)
{
	if (field == nullptr)
	{
		return;
	}

	ComponentRefField refField;
	refField.label = label;
	refField.field = field;
	refField.expectedType = expectedType;
	m_componentRefs.push_back(refField);
}

const std::vector<GameObjectRefField>& ReferenceFieldRegistry::GetGameObjectRefs() const
{
	return m_gameObjectRefs;
}

const std::vector<ComponentRefField>& ReferenceFieldRegistry::GetComponentRefs() const
{
	return m_componentRefs;
}

Component::Component()
{
	m_componentId = GenerateNextPersistentComponentId();
}

Component::~Component()
{
}

void Component::InitGameObj(GameObject* obj)
{
	m_gameObj = obj;
}

GameObject* Component::GetGameObject()
{
	return this->m_gameObj;
}

int Component::GetComponentId() const
{
	return m_componentId;
}

void Component::SetComponentId(int componentId)
{
	if (componentId < 0)
	{
		m_componentId = GenerateNextPersistentComponentId();
		return;
	}

	m_componentId = componentId;
	SyncPersistentComponentIdAllocator(componentId);
}

const char* Component::GetInspectorName() const
{
	return typeid(*this).name();
}

void Component::DrawInspector()
{
}

const char* Component::GetSerializableType() const
{
	return "";
}

std::string Component::Serialize() const
{
	return "{}";
}

std::string Component::SerializeWithRegisteredReferenceFields() const
{
	ReferenceFieldRegistry registry;
	const_cast<Component*>(this)->RegisterReferenceFields(registry);

	const std::vector<GameObjectRefField>& gameObjectRefs = registry.GetGameObjectRefs();
	const std::vector<ComponentRefField>& componentRefs = registry.GetComponentRefs();
	if (gameObjectRefs.empty() && componentRefs.empty())
	{
		return Serialize();
	}

	std::ostringstream referenceFieldsOss;
	referenceFieldsOss << "{ ";
	bool isFirstEntry = true;

	for (std::vector<GameObjectRefField>::const_iterator itr = gameObjectRefs.begin(); itr != gameObjectRefs.end(); ++itr)
	{
		if (itr->field == nullptr || itr->label == nullptr)
		{
			continue;
		}

		GameObject* referencedObject = *(itr->field);
		const int referencedObjectId = referencedObject != nullptr ? referencedObject->GetId() : -1;
		if (!isFirstEntry)
		{
			referenceFieldsOss << ", ";
		}

		referenceFieldsOss
			<< "\"" << SceneJson::EscapeString(itr->label) << "\": "
			<< "{ \"kind\": \"GameObject\", \"gameObjectId\": " << referencedObjectId << " }";
		isFirstEntry = false;
	}

	for (std::vector<ComponentRefField>::const_iterator itr = componentRefs.begin(); itr != componentRefs.end(); ++itr)
	{
		if (itr->field == nullptr || itr->label == nullptr)
		{
			continue;
		}

		Component* referencedComponent = *(itr->field);
		const int referencedComponentId = referencedComponent != nullptr ? referencedComponent->GetComponentId() : -1;
		const char* expectedType = itr->expectedType != nullptr ? itr->expectedType : "";

		if (!isFirstEntry)
		{
			referenceFieldsOss << ", ";
		}

		referenceFieldsOss
			<< "\"" << SceneJson::EscapeString(itr->label) << "\": "
			<< "{ \"kind\": \"Component\", \"componentId\": " << referencedComponentId
			<< ", \"expectedType\": \"" << SceneJson::EscapeString(expectedType) << "\" }";
		isFirstEntry = false;
	}

	referenceFieldsOss << " }";
	if (isFirstEntry)
	{
		return Serialize();
	}

	return AppendObjectField(Serialize(), "referenceFields", referenceFieldsOss.str());
}

bool Component::Deserialize(const std::string& componentJson)
{
	return true;
}

void Component::RegisterReferenceFields(ReferenceFieldRegistry& registry)
{
	(void)registry;
}

void Component::LoadReferenceFieldIds(const std::string& componentJson)
{
	m_pendingGameObjectRefs.clear();
	m_pendingComponentRefs.clear();

	std::string referenceFieldsJson;
	if (!SceneJson::ExtractObject(componentJson, "referenceFields", referenceFieldsJson))
	{
		return;
	}

	ReferenceFieldRegistry registry;
	RegisterReferenceFields(registry);

	const std::vector<GameObjectRefField>& gameObjectRefs = registry.GetGameObjectRefs();
	for (std::vector<GameObjectRefField>::const_iterator itr = gameObjectRefs.begin(); itr != gameObjectRefs.end(); ++itr)
	{
		if (itr->label == nullptr)
		{
			continue;
		}

		std::string entryJson;
		if (!SceneJson::ExtractObject(referenceFieldsJson, itr->label, entryJson))
		{
			continue;
		}

		PendingGameObjectReference pendingRef;
		pendingRef.label = itr->label;
		SceneJson::ReadInt(entryJson, "gameObjectId", pendingRef.gameObjectId);
		m_pendingGameObjectRefs.push_back(pendingRef);
	}

	const std::vector<ComponentRefField>& componentRefs = registry.GetComponentRefs();
	for (std::vector<ComponentRefField>::const_iterator itr = componentRefs.begin(); itr != componentRefs.end(); ++itr)
	{
		if (itr->label == nullptr)
		{
			continue;
		}

		std::string entryJson;
		if (!SceneJson::ExtractObject(referenceFieldsJson, itr->label, entryJson))
		{
			continue;
		}

		PendingComponentReference pendingRef;
		pendingRef.label = itr->label;
		SceneJson::ReadInt(entryJson, "componentId", pendingRef.componentId);
		SceneJson::ReadString(entryJson, "expectedType", pendingRef.expectedType);
		m_pendingComponentRefs.push_back(pendingRef);
	}
}

void Component::ResolveRegisteredReferenceFields(
	const std::unordered_map<int, GameObject*>& objectMap,
	const std::unordered_map<int, Component*>& componentMap)
{
	ReferenceFieldRegistry registry;
	RegisterReferenceFields(registry);

	const std::vector<GameObjectRefField>& gameObjectRefs = registry.GetGameObjectRefs();
	for (std::vector<GameObjectRefField>::const_iterator fieldItr = gameObjectRefs.begin(); fieldItr != gameObjectRefs.end(); ++fieldItr)
	{
		if (fieldItr->label == nullptr || fieldItr->field == nullptr)
		{
			continue;
		}

		*(fieldItr->field) = nullptr;
		for (std::vector<PendingGameObjectReference>::const_iterator pendingItr = m_pendingGameObjectRefs.begin();
			pendingItr != m_pendingGameObjectRefs.end();
			++pendingItr)
		{
			if (pendingItr->label != fieldItr->label)
			{
				continue;
			}

			std::unordered_map<int, GameObject*>::const_iterator objectItr = objectMap.find(pendingItr->gameObjectId);
			if (objectItr != objectMap.end())
			{
				*(fieldItr->field) = objectItr->second;
			}
			break;
		}
	}

	const std::vector<ComponentRefField>& componentRefs = registry.GetComponentRefs();
	for (std::vector<ComponentRefField>::const_iterator fieldItr = componentRefs.begin(); fieldItr != componentRefs.end(); ++fieldItr)
	{
		if (fieldItr->label == nullptr || fieldItr->field == nullptr)
		{
			continue;
		}

		*(fieldItr->field) = nullptr;
		for (std::vector<PendingComponentReference>::const_iterator pendingItr = m_pendingComponentRefs.begin();
			pendingItr != m_pendingComponentRefs.end();
			++pendingItr)
		{
			if (pendingItr->label != fieldItr->label)
			{
				continue;
			}

			std::unordered_map<int, Component*>::const_iterator componentItr = componentMap.find(pendingItr->componentId);
			if (componentItr != componentMap.end())
			{
				Component* resolvedComponent = componentItr->second;
				const char* expectedType = fieldItr->expectedType;
				if ((expectedType == nullptr || strlen(expectedType) == 0 || IsSameComponentType(resolvedComponent, expectedType)) &&
					(pendingItr->expectedType.empty() || IsSameComponentType(resolvedComponent, pendingItr->expectedType)))
				{
					*(fieldItr->field) = resolvedComponent;
				}
			}
			break;
		}
	}

	m_pendingGameObjectRefs.clear();
	m_pendingComponentRefs.clear();
}

void Component::ResolveReferences(const std::unordered_map<int, GameObject*>& objectMap)
{
	(void)objectMap;
}

void Component::OnDeviceLost()
{
}

void Component::OnDeviceReset()
{
}
