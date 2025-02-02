#include "pch.h"
#include "ModelResource.h"

#include "NodeAnimation.h"
#include "Mesh.h"
#include "Material.h"
#include "Bone.h"
#include "SkeletonResource.h"


ModelResource::ModelResource()
{
}

ModelResource::~ModelResource()
{
	m_Meshes.clear();
	m_Materials.clear();
}

bool ModelResource::ReadFile(std::string filePath, ModelType modelType)
{
	Assimp::Importer importer;

	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, 0);

	unsigned int importFlags;
	if(modelType == ModelType::SKELETAL)
	{
		importFlags = aiProcess_Triangulate | // �ﰢ������ ��ȯ
			aiProcess_GenNormals |   // �븻 ����
			aiProcess_GenUVCoords |  // UV ����
			aiProcess_CalcTangentSpace |  // ź��Ʈ ����
			aiProcess_LimitBoneWeights |  // ���� ������ �޴� ������ �ִ� ������ 4���� ����
			aiProcess_GenBoundingBoxes |  // �ٿ�� �ڽ� ����
			aiProcess_ConvertToLeftHanded;   // �޼� ��ǥ��� ��ȯ
	}
	else
	{
		importFlags = aiProcess_Triangulate | // �ﰢ������ ��ȯ
			aiProcess_GenNormals |   // �븻 ����
			aiProcess_GenUVCoords |  // UV ����
			aiProcess_CalcTangentSpace |  // ź��Ʈ ����
			aiProcess_LimitBoneWeights |  // ���� ������ �޴� ������ �ִ� ������ 4���� ����
			aiProcess_GenBoundingBoxes |  // �ٿ�� �ڽ� ����
			aiProcess_PreTransformVertices |  // �������� ������ ���� �������� �޾ƿͼ� ����� ���·� ��
			aiProcess_ConvertToLeftHanded;   // �޼� ��ǥ��� ��ȯ
	}

	const aiScene* fbxModel = importer.ReadFile(filePath, importFlags);

	if (!fbxModel)
	{
		LOG_ERRORA("Error loading FBX file: %s", importer.GetErrorString());
		return false;
	}

	// �̰����� ���̷���(����)�� ������ݴϴ�.
	if (modelType == ModelType::SKELETAL)
		m_Skeleton.Create(fbxModel);

	// fbx�� ����� ��(�ؽ���) ���� �������� ����.
	m_Materials.resize(fbxModel->mNumMaterials);
	for (int i = 0; i < fbxModel->mNumMaterials; i++)
	{
		m_Materials[i].Create(fbxModel->mMaterials[i]);
	}

	// vertex , index ���� ���ε�
	m_Meshes.resize(fbxModel->mNumMeshes);
	for (int i = 0; i < fbxModel->mNumMeshes; i++)
	{
		aiMesh* fbxMesh = fbxModel->mMeshes[i];

		if (modelType == ModelType::STATIC)
		{
			m_Meshes[i].Create(fbxMesh);
		}
		else if (modelType == ModelType::SKELETAL)
		{
			m_Meshes[i].CreateBoneWeight(fbxMesh, &m_Skeleton);
		}
	}

	// SceneResource�� �⺻ �ִϸ��̼� �߰��Ѵ�.
	// ToDO :: ����  �ִϸ��̼� ������ �������� ��� ó�����ֱ�.
	 //assert(fbxModel->mNumAnimations < 2); // �ִϸ��̼��� ���ų� 1�������Ѵ�. 
	// ����� �ִϸ��̼��� �ϳ��� ��ġ�� ����� FBX export���� NLA��Ʈ��,��� �׼� �ɼ��� ���ϴ�.
	if (fbxModel->HasAnimations())
	{
		const aiAnimation* pAiAnimation = fbxModel->mAnimations[0];
		// ä�μ��� aiAnimation �ȿ��� �ִϸ��̼� ������  ǥ���ϴ� aiNode�� �����̴�.
		assert(pAiAnimation->mNumChannels >= 1); // �ִϸ��̼��� �ִٸ� aiNode �� �ϳ� �̻� �־���Ѵ�.

		shared_ptr<Animation> anim = make_shared<Animation>();
		anim->Create(filePath, pAiAnimation);
		m_Animations.push_back(anim);
	}

	float absMax = 0.f;
	for (UINT i = 0; i < fbxModel->mNumMeshes; i++)
	{
		aiMesh* pMesh = fbxModel->mMeshes[i];
		Math::Vector3 meshMin = Math::Vector3(pMesh->mAABB.mMin.x, pMesh->mAABB.mMin.y, pMesh->mAABB.mMin.z);
		Math::Vector3 meshMax = Math::Vector3(pMesh->mAABB.mMax.x, pMesh->mAABB.mMax.y, pMesh->mAABB.mMax.z);

		BoundingBox boundingBox;
		boundingBox.Center = (meshMin + meshMax) / 2;
		boundingBox.Extents = (meshMax - meshMin) / 2;
		m_InstanceBoundingBox.push_back(boundingBox);

		m_AABBmin = Math::Vector3::Min(m_AABBmin, meshMin);
		m_AABBmax = Math::Vector3::Max(m_AABBmax, meshMax);
	}
	absMax = max(m_AABBmax.Length(), m_AABBmin.Length());
	m_BoundingBoxMin = Math::Vector3(-absMax, -absMax, -absMax);
	m_BoundingBoxMax = Math::Vector3(absMax, absMax, absMax);

	importer.FreeScene();
	return true;
}

Material* ModelResource::GetMeshMaterial(UINT index)
{
	assert(index < m_Meshes.size());
	UINT mindex = m_Meshes[index].m_MaterialIndex;
	assert(mindex < m_Materials.size());
	return &m_Materials[mindex];
}

Math::Vector3 ModelResource::GetBoundingBoxDimensions() const
{
	return Math::Vector3(m_AABBmax.x - m_AABBmin.x, m_AABBmax.y - m_AABBmin.y, m_AABBmax.z - m_AABBmin.z);
}