#pragma once

struct Ray
{
	D3DXVECTOR3 origin = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 direction = D3DXVECTOR3(0.0f, 0.0f, 1.0f);

	Ray() = default;

	Ray(const D3DXVECTOR3& rayOrigin, const D3DXVECTOR3& rayDirection)
		: origin(rayOrigin), direction(rayDirection)
	{
		NormalizeDirection();
	}

	void NormalizeDirection()
	{
		const float lengthSq = D3DXVec3LengthSq(&direction);
		if (lengthSq <= 0.000001f)
		{
			direction = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
			return;
		}

		D3DXVec3Normalize(&direction, &direction);
	}
};
