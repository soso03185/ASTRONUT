#pragma once

constexpr float partDefaultScale = 100;

enum class ePartType
{
	GEAR,

	END
};

// ������Ʈ Part�� ���� POD
struct PartPOD
{
	ePartType partType;
	Vector3 position;
	Quaternion rotation;
	Vector3 scale = { 1 / partDefaultScale, 1 / partDefaultScale, 1 / partDefaultScale };
};