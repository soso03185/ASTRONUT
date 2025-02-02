#pragma once
#include "WinSock.h"

/// <summary>
/// ClientNetworkManager���� ����ϴ� ����
/// WinSock�� ��ӹ޾� ���� ������ � �̺�Ʈ �������� Ȯ���� �Ŀ� 
/// �׿� �˸´� �Լ� ȣ��.
/// </summary>
class ClientSocket : public WinSock
{
public:
	ClientSocket() = default;
	~ClientSocket() = default;

	virtual bool OnAccept(std::shared_ptr<WinSock> pSocket) override { return false; }
	virtual bool OnConnect() override { return false; }
	virtual bool OnClose() override { return false; }
	virtual bool OnSend() override { return false; }
	virtual bool OnReceive() override { return false; }
};
