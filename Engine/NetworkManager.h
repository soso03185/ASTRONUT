#pragma once
#include "CircularQueue.h"
#include "../NetworkLibrary/MyProtocol.h"

/// <summary>
/// Ŭ���̾�Ʈ�� �������� ���� �����͸� ���ӷ����� �����ϴ� ��.(�ݴ�ε�)
/// �������� �ؼ��ϴ� ��.
/// ����(ť)�� ���⼭ ����
/// </summary>

class NetworkManager
{
public:
	void Initialize();
	void Finalize();
};
