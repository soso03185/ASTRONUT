#pragma once
#include "../NetworkLibrary/MyProtocol.h"

#include <concurrent_queue.h>

class ClientSocket;
class ClientNetworkManager;

typedef int SessionId;

struct ManageStartTurn
{
	bool bserverEnter = false;
};

struct ManageSessionTurn
{
	bool bserverEnter = false;
	char bHost;
};

struct ManageRestart
{
	bool bserverEnter = false;
	bool restart = false;
};

struct ManageReady
{
	bool bserverEnter = false;
	bool ready = false;
};

class Session
{
public:
	Session();
	~Session();

	void Write(char* pData, int len);
	// recv�ؼ� ���� ���۰� ��ȿ�� ��Ŷ���� Ȯ���ϱ�.
	void Read(char* pData);

	void ReadUpdate();

	void SetClient(std::shared_ptr<ClientNetworkManager> pClient);

	bool GetReadyState() { return m_bReady; }
	void SetReadyState(bool ready) { m_bReady = ready; }

	bool GetEnterState() { return m_bStageEnter; }
	void SetEnterState(bool stageEnter) { m_bStageEnter = stageEnter; }

	SessionId GetSessionId() { return m_sessionId; }

	bool PopRecvQueue(std::pair<char*, int>& value) { return m_recvQueue.try_pop(value); }
	bool PopSendQueue(std::pair<char*, int>& value) { return m_sendQueue.try_pop(value); }

	void PushSendQueue(char* c, int len) { m_sendQueue.push({ c, len }); }
	void PushRecvQueue(char* c, int len) { m_recvQueue.push({ c, len }); }

	bool IsSendQueueEmpty() { return m_sendQueue.empty(); }
	bool IsRecvQueueEmpty() { return m_recvQueue.empty(); }

	void SetHostSession() { m_bHost = true; }
	bool GetIsHostSession() { return m_bHost; }

	// ������ ������ ���Դ��� Ȯ�����ֱ� ���� �Լ���
	void SetSessionTurn(char bHost, bool enter) { m_sessionTurn.bHost = bHost; m_sessionTurn.bserverEnter = enter; }
	void SetStartTurn(bool enter) { m_startTurn.bserverEnter = enter; }
	// �÷��̾� ���� �������� üũ
	void SetMovement(bool isHost) { if (isHost) m_movement[0] = true; else m_movement[1] = true; }
	// �÷��̾�� �����Ѱ� �ʱ�ȭ
	void SetMovementFalse();
	// ���� ��ü�� ���� �� �ϰ� ü������ �� �� �ִ� ����
	void SetSessionMovementEnd(bool moveEnd) { m_bMovementEnd = moveEnd; }
	void SetRestart(bool restart, bool enter) { m_restart.restart = restart; m_restart.bserverEnter = enter; }
	void SetGoalIn(bool goalIn) { m_bGoalIn = goalIn; }
	void SetGoalEnd(bool goalEnd) { m_bGoalEnd = goalEnd; }
	void SetReady(bool ready, bool enter) { m_ready.ready = ready; m_ready.bserverEnter = enter; }

	ManageSessionTurn& GetSessionTurn() { return m_sessionTurn; }
	ManageStartTurn& GetStartTurn() { return m_startTurn; }
	ManageRestart& GetReStart() { return m_restart; }
	ManageReady& GetReady() { return m_ready; }
	bool GetAllMovementEnd();
	bool GetSessionMovementEnd() { return m_bMovementEnd; }
	bool GetGoalIn() { return m_bGoalIn; }
	bool GetGoalEnd() { return m_bGoalEnd; }

	int Send(char* packet, int len);

	void Finalize();

private:
	// ���۶� ������ length�� ���� �ִ� ť
	Concurrency::concurrent_queue<std::pair<char*, int>> m_sendQueue;
	Concurrency::concurrent_queue<std::pair<char*, int>> m_recvQueue;

	char m_sendBuffer[SND_BUF_SIZE];
	char m_recvBuffer[RCV_BUF_SIZE];

	int m_recvBytes = 0;
	int m_sendBytes = 0;

	std::shared_ptr<ClientNetworkManager> m_pPeerClient = nullptr;
	SessionId m_sessionId = 0;
	static SessionId s_sessionIdCounter;

	bool m_bHost = false;
	bool m_bReady = false;
	bool m_bStageEnter = false;
	bool m_bMovementEnd = false;
	bool m_bGoalIn = false;
	bool m_bGoalEnd = false;

	std::string m_NickName = {};
	ManageSessionTurn m_sessionTurn;
	ManageStartTurn m_startTurn;
	ManageRestart m_restart;
	ManageReady m_ready;

	// 0�� �ε����� ȣ��Ʈ�÷��̾� 1�� �ε����� �Խ�Ʈ�÷��̾�
	bool m_movement[2] = { false, false };
};

