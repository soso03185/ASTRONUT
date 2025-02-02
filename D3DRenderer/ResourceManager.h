#pragma once

#include "ModelResource.h"
#include "Shader.h"
#include "ParticleShader.h"

#include <string>

class Material;
class ModelResource;
class MaterialTexture;
class Animation;
class Shader;
class Texture;
class ParticleShader;

class ResourceManager
{
public:
	~ResourceManager() = default;


private:
	ResourceManager() = default;
	static ResourceManager* Instance;

public:
	static ResourceManager* GetInstance()
	{
		if (!Instance)
			Instance = new ResourceManager;

		return Instance;
	}
	std::shared_ptr<Material> CreateMaterial(std::string key);
	std::shared_ptr<MaterialTexture> CreateMaterialTexture(std::wstring filePath);
	std::shared_ptr<ModelResource> CreateModelResource(std::string filePath, ModelType modelType);
	std::shared_ptr<Animation> CreateAnimationResource(std::string filePath, bool isLoop);

	// ���� ���͸��� �������� ���� �ؽ�ó�� ����Ҽ� �����Ƿ� ���� ��Ͽ��� ���ϰ�� Ȯ���ؼ� ������
	std::shared_ptr<Texture> CreateTextureResource(std::wstring filePath);
	std::shared_ptr<Shader> CreateShaderResource(std::string key, eShaderType shaderType);
	std::shared_ptr<ParticleShader> CreateParticleShader(std::wstring key, eParticleType type);

	void Finalize();

public:
	std::unordered_map<std::string, std::shared_ptr<Material>> m_MaterialMap;					// Material
	std::unordered_map<std::wstring, std::shared_ptr<MaterialTexture>> m_MaterialTextureMap;	// MaterialTexture
	std::unordered_map<std::string, std::weak_ptr<ModelResource>> m_StaticModelMap;				// StaticModel
	std::unordered_map<std::string, std::weak_ptr<ModelResource>> m_SkeletalModelMap;			// SkeletalModel
	std::unordered_map<std::string, std::weak_ptr<Animation>> m_AnimationMap;					// Animation
	std::unordered_map<std::wstring, std::shared_ptr<Texture>> m_TextureMap;					// UI Texture
	std::unordered_map<std::string, std::shared_ptr<Shader>> m_ShaderMap;						// Shader
	std::unordered_map<std::wstring, std::shared_ptr<ParticleShader>> m_ParticleShaderMap;		// ParticleShader
};
