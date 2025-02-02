#pragma once
#include <cstddef>
#include <string>

#pragma pack (push, 1)

#define MAX_LEN 100

enum EPacketId : short
{
	PACKETID_START,

	C2S_BROADCAST_MSG,
	S2C_BROADCAST_MSG,

	// �������� ���� host��� �������ֱ�(�׷��� Session���� m_bHost�� true�� �������)
	C2S_IAM_HOST_MSG,

	// Ŭ�� ���� ��ư ������ ��
	C2S_READY,

	// ��� Ŭ�� ��ŸƮ�����̵� ��ǥ������ �����ߴ���
	C2S_IS_ALL_GOAL,
	S2C_IS_ALL_GOAL,

	// ��� Ŭ�� ���������̵� ��ǥ������ �����ߴ���
	C2S_IS_ALL_END,
	S2C_IS_ALL_END,

	// ��� Ŭ�� ���������� ���Դ���
	C2S_IS_ALL_ENTER,
	S2C_IS_ALL_ENTER,

	// ��� Ŭ�� �������� Ȯ��
	C2S_IS_ALL_READY,
	S2C_IS_ALL_READY,

	// �ڽ��� �ൿ���� ��ε�ĳ��Ʈ
	C2S_BROADCAST_ACTION,
	S2C_BROADCAST_ACTION,

	// ���������� �� �������ֱ�
	C2S_SET_TURN,
	S2C_SET_TURN,

	// �� �����̴�
	C2S_START_TURN,
	S2C_START_TURN,

	// �� �ٲٱ�
	C2S_CHANGE_TURN,
	S2C_CHANGE_TURN,

	// �ൿ�� �����ִ��� Ȯ��
	C2S_CHECK_ACTION,
	S2C_CHECK_ACTION,

	// ��� �ൿ�� ������.
	C2S_END_ACTION,
	S2C_END_ACTION,

	// �������� ������.
	C2S_MOVEMENT_END,
	S2C_MOVEMENT_END,

	// �ٽ� ����
	C2S_RESTART_GAME,
	S2C_RESTART_GAME,

	// ĳ���Ͱ� ���� �̵��ϴ��� Broadcast
	S2C_BROADCAST_MOVE,

	PACKETID_END,
};

struct PacketHeader
{
	short size;
	EPacketId id;
};

struct PacketC2S_Access : PacketHeader
{
	char nickname[MAX_LEN];
};

struct PacketS2C_Access : PacketHeader
{
	char result;
};

struct PacketC2S_BroadcastMsg : PacketHeader
{
	char clientMessage[MAX_LEN];
};

struct PacketS2C_BroadcastMsg : PacketHeader
{
	char serverMessage[MAX_LEN];
};

// ���ǿ��� ȣ��Ʈ���� �˷��ֱ� ���ؼ�
struct PacketC2S_IamHost : PacketHeader
{
	char host;
};
// Ŭ�󿡼� ���� ��ư ������ ��
struct PacketC2S_READY : PacketHeader
{
	char clickedReady;
};

struct PacketC2S_IsAllEnd : PacketHeader
{
};

struct PacketS2C_IsAllEnd : PacketHeader
{
};

struct PacketC2S_IsAllGoal : PacketHeader
{
};

struct PacketS2C_IsAllGoal : PacketHeader
{
};

struct PacketC2S_IsAllEnter : PacketHeader
{
};
// 
struct PacketS2C_IsAllEnter : PacketHeader
{
};

// Ŭ�󿡼� ���� ���� �� ��� Ŭ�� ����������� ��û
struct PacketC2S_IsAllReady : PacketHeader
{
};
// ������ Ŭ������ ��� ����� true �ƴϸ� false �����ش�.
struct PacketS2C_IsAllReady : PacketHeader
{
};

// Ŭ�󿡼� �������� � �ൿ���� �˷��ֱ�
struct PacketC2S_BroadcastAction : PacketHeader
{
	char who;
	char action[4];
};

// �������� � Ŭ�� � �ൿ���� ��ε�ĳ��Ʈ���ֱ�.
struct PacketS2C_BroadcastAction : PacketHeader
{
	char who;
	char action[4];
};

struct PacketC2S_SetTurn : PacketHeader
{
	// '0'�̸� host, '1'�̸� Ŭ���̾�Ʈ
	char who;
};

struct PacketS2C_SetTurn : PacketHeader
{
	// '0'�̸� host, '1'�̸� Ŭ���̾�Ʈ
	char setTurn;
};

struct PacketC2S_StartTurn : PacketHeader
{
	// �� �����ϱ�
};

struct PacketC2S_EndTurn : PacketHeader
{
	// ȣ��Ʈ�� ���� ��û�ߴ���, �Խ�Ʈ�� ���� ��û�ߴ���
};

struct PacketS2C_EndTurn : PacketHeader
{
	// �ൿ�� ������ �� �����ϱ�(client�� �ִ� turn �Ұ��� false�� �ٲ��ش�.
};

struct PacketC2S_ChangeTurn : PacketHeader
{
	// ȣ��Ʈ�� ���� ��û�ߴ���, �Խ�Ʈ�� ���� ��û�ߴ���
};

struct PacketS2C_ChangeTurn : PacketHeader
{
	// �ൿ�� ������ �� �����ϱ�(client�� �ִ� turn �Ұ��� false�� �ٲ��ش�.
};

struct PacketS2C_StartGame : PacketHeader
{
	// ������ ����
};

struct PacketC2S_CheckAction : PacketHeader
{
	// �ൿ�� �����ִ��� Ȯ��
};

struct PacketS2C_CheckAction : PacketHeader
{
	// �ൿ�� �����ִ��� Ȯ��
};

struct PacketC2S_EndAction : PacketHeader
{
	// ������ ��
};

struct PacketS2C_EndAction : PacketHeader
{
	// ������ ��
};

struct PacketC2S_MovementEnd : PacketHeader
{
	// �������� ������
	char who;
};

struct PacketS2C_MovementEnd : PacketHeader
{
	// �������� ������.
};

struct PacketC2S_RestartGame : PacketHeader
{
	// ���� �ٽ� ����
	char restart; // 0�̸� �ٽ� ���� �� ��, 1�̸� �ٽ� ����
};

struct PacketS2C_RestartGame : PacketHeader
{
	// ���� �ٽ� ����
	char restart; // 0�̸� �ٽ� ���� �� ��, 1�̸� �ٽ� ����
};

#pragma pack (pop)