#pragma once

struct BoneInfo
{
	Math::Matrix RelativeTransform;			//�θ�κ����� ������� ��ȯ
	Math::Matrix OffsetMatrix;				// �������� �޽��� ������� ��ġ(��ȯ)
	string Name;						// ���̸�
	int NumChildren = 0;						// �ڽ��� ��
	int ParentBoneIndex = -1;
	vector<string> MeshNames;		// ���� ����� �޽����� �̸�
	BoneInfo() {}
	void Set(const aiNode* pNode)
	{
		Name = string(pNode->mName.C_Str());
		RelativeTransform = Math::Matrix(&pNode->mTransformation.a1).Transpose();
		NumChildren = pNode->mNumChildren;
	}
	~BoneInfo()
	{

	}
};

// �� ������ �����ϴ� ����ü
struct SkeletonResource
{
public:
	string Name;
	vector<BoneInfo> Bones;
	map<string, int> BoneMappingTable;
	map<string, int> MeshMappingTable;

	void Create(const aiScene* pScene);
	BoneInfo* AddBone(const aiScene* pScene, const aiNode* pNode);
	BoneInfo* FindBone(const string& name);
	BoneInfo* GetBone(int index);
	int GetBoneIndexByBoneName(const string& boneName);
	int GetBoneIndexByMeshName(const string& meshName);
	int GetBoneCount() {
		return (int)Bones.size();
	}
	const string& GetBoneName(int index) {
		return Bones[index].Name;
	}
	void CountNode(int& Count, const aiNode* pNode);
};
