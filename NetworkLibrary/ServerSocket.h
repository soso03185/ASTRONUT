#pragma once
#include "WinSock.h"

/// <summary>
/// ServerNetworkManager���� ����ϴ� ����
/// WinSock�� ��ӹ޾� ���� ������ � �̺�Ʈ �������� Ȯ���� �Ŀ� 
/// �׿� �˸´� �Լ� ȣ��.
/// </summary>
class ServerSocket : public WinSock
{
public:
	ServerSocket() = default;
	~ServerSocket() = default;

	virtual bool OnAccept(std::shared_ptr<WinSock> pSocket) override
	{
		return (Accept(*pSocket));
	}
	virtual bool OnConnect() override { return false; }
	virtual bool OnClose() override { return false; }
	virtual bool OnSend() override { return false; }
	virtual bool OnReceive() override { return false; }
};


