#pragma once
#include "Component.h"

class UIElement : public Component
{
protected:
	D3DXVECTOR2 m_pendingPosition = D3DXVECTOR2(0.0f, 0.0f);
	D3DXVECTOR2 m_pendingSize = D3DXVECTOR2(0.0f, 0.0f);
	bool m_hasPendingPosition = false;
	bool m_hasPendingSize = false;
	bool m_visible = true;
	bool m_enabled = true;
	int m_orderInLayer = 0;

	void ApplyPendingTransform();

public:
	void Initialize() override;
	void Release() override;
	void Start() override;
	void Update() override;
	virtual void RenderUI();
	void InitGameObj(GameObject* obj) override;

	virtual void SetPosition(const D3DXVECTOR2* position);
	D3DXVECTOR2 GetPosition() const;
	virtual void SetLocalOffset(const D3DXVECTOR2* offset);
	D3DXVECTOR2 GetLocalOffset() const;
	virtual void SetSize(const D3DXVECTOR2* size);
	D3DXVECTOR2 GetSize() const;

	virtual void SetVisible(bool visible);
	bool IsVisible() const;
	void SetEnabled(bool enabled);
	bool IsEnabled() const;

	virtual void SetOrderInLayer(int orderInLayer);
	int GetOrderInLayer() const;

	RECT GetRect() const;
	bool ContainsPoint(const D3DXVECTOR2* point) const;
	const char* GetInspectorName() const override;
	void DrawInspector() override;
	std::string Serialize() const override;
	bool Deserialize(const std::string& componentJson) override;
};
