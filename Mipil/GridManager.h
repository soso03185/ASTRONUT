#pragma once

class TestPlayerObject;
class PlayerObject;
class GameObject;
enum class eObjectType;

enum class eGridType
{
	EMPTY,
	BLOCK,
	PLAYER,
	PART,

	END
};

struct GridPos
{
	int x;
	int y;
	int z;

	GridPos& operator+(const GridPos& other) const
	{
		GridPos rt = { x + other.x, y + other.y, z + other.z };
		return rt;
	}

	GridPos& operator-(const GridPos& other) const
	{
		GridPos rt = { x - other.x, y - other.y, z - other.z };
		return rt;
	}

	bool operator==(const GridPos& other) const
	{
		if (x == other.x && y == other.y && z == other.z)
			return true;
		return false;
	}

	GridPos& operator+(const Vector3& other) const
	{
		GridPos rt = { x + static_cast<int>(other.x), y + static_cast<int>(other.y), z + static_cast<int>(other.z) };
		return rt;
	}

	GridPos& operator-(const Vector3& other) const
	{
		GridPos rt = { x - static_cast<int>(other.x), y - static_cast<int>(other.y), z - static_cast<int>(other.z) };
		return rt;
	}
};

enum class eMinimapType
{
	EMPTY,	// ������ ���� empty�� �ش�. �̴ϸ��� "��ǰ"�� �÷��̾� ��ġ�� ǥ���ϱ� ����
	HOST,
	GUEST,
	OVERLAP,
	PART,

	END
};

struct MinimapPos
{
	int x;
	int z;
};

/// <summary>
///	CreateMap�� ���� ó�� ���� �԰��� ���Ѵ�.
/// �׸��� �ʿ��� ������ ������Ʈ���� SetMapState�� ���� �� ������ ����.
/// �׸��� ���� �Ʒ���� �� �ϴ� �������� ��ġ�ȴ�.
/// DX ��ǥ�� �������� ���� row�� Z��, col�� X���̴�.
/// </summary>
class GridManager
{
public:
	GridManager() = default;
	~GridManager() = default;

	// ó�� ��, �׸��� ũ��, ���� ���� ��ġ ����
	void CreateMap(int col, int height, int row, float startXPos, float startYPos, float startZPos, float gridSize);

	// ������Ʈ�κ��� ��ġ �о �׸��� ��ħ Ȯ�� �� �ʿ� ����
	void SetMapState(std::shared_ptr<GameObject> object); 
	void SetMapState(std::vector<std::shared_ptr<GameObject>>& objects);

	// ���ڷ� �̵� Ÿ�� ���� ��, ���ɼ� Ȯ�� �� �̵�ó��. �׸��� �� ����
	//void MoveOnGrid(TestPlayerObject* player, Vector3 vec);	// �׽�Ʈ��
	void MoveOnGrid(PlayerObject* player, Vector3 vec);

	//void FallToGround(TestPlayerObject* player);	// �׽�Ʈ��
	void FallToGround(PlayerObject* player);	// �׽�Ʈ��

	bool IsGround(Vector3 pos) const;
	bool IsPlayerAbove(Vector3 pos) const;

	int GetNumCol() const { return m_numCol; }
	int GetNumRow() const { return m_numRow; }

	std::vector<std::pair<MinimapPos, eMinimapType>>& GetMinimapInfo();
	void ClearMinimapInfo();

	GridPos& ConvertToGridPos(Vector3 position) const;	// vector3 ��ǥ �׸��� ��ǥ�� ��ȯ

private:
	Vector3& ConvertToDXPos(GridPos position) const;

	bool CheckDuplicated(GridPos& gridPos) const;
	eGridType MatchingGridType(eObjectType type) const;

	bool CheckMovePossibility(GridPos& gridPos, Vector3 vec);	// �̵� ���� ���ο� ���� ����

	int CalculateIdx(int x, int z) const { return z * m_numCol + x; }


	int m_numCol;		// x
	int m_numHeight;	// y
	int m_numRow;		// z
	float m_startXPos;	// ���� ������ �ƴ� �������� CreateMap���� �����Ѵ�.
	float m_startYPos;
	float m_startZPos;
	float m_gridSize;

	int m_fallCnt;
	bool m_bGetPart = false;

	std::vector<std::vector<eGridType>> m_gridMap;

	std::vector<std::vector<eMinimapType>> m_miniMap;
	std::vector<std::pair<MinimapPos, eMinimapType>> m_miniMapInfos;
};