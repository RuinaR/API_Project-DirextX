#pragma once

#include <string>
#include <vector>

#include "Component.h"

class ObjectPoolComponent : public Component
{
public:
	static constexpr const char* kComponentType = "ObjectPoolComponent";

	ObjectPoolComponent() = default;

	void Initialize() override;
	void Release() override;
	void Start() override;
	void Update() override;

	void CollectChildren();
	void RecycleObject(GameObject* objectToRecycle);
	GameObject* ActivateNextInactiveObject();
	const std::vector<GameObject*>& GetPoolObjects() const;

	float GetLeftBound() const;
	void SetLeftBound(float leftBound);
	float GetRightSpawnX() const;
	void SetRightSpawnX(float rightSpawnX);
	float GetSpacing() const;
	void SetSpacing(float spacing);
	bool GetRepositionInsteadOfDisable() const;
	void SetRepositionInsteadOfDisable(bool repositionInsteadOfDisable);
	const std::string& GetPoolTag() const;
	void SetPoolTag(const std::string& poolTag);

	const char* GetInspectorName() const override;
	void DrawInspector() override;
	const char* GetSerializableType() const override;
	std::string Serialize() const override;
	bool Deserialize(const std::string& componentJson) override;

protected:
	void SetPoolMode(bool repositionInsteadOfDisable);
	void DrawSharedInspector(bool showModeToggle);

private:
	void DeactivateAllPoolObjects();
	void ActivateInitialPoolObjects();

	std::vector<GameObject*> m_poolObjects;
	float m_leftBound = -900.0f;
	float m_rightSpawnX = 900.0f;
	float m_spacing = 420.0f;
	bool m_repositionInsteadOfDisable = true;
	std::string m_poolTag;
};
