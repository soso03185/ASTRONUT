#pragma once

enum class eShaderType
{
	STATIC_SHADER = 0,
	SKELETAL_SHADER,
	ENVIRONMENT_SHADER,
	UI_SHADER,
	UI_MESH_SHADER,
	UI_MESH_TEXTURE_SHADER,
	STATIC_SHADOW_SHADER,
	SKELETAL_SHADOW_SHADER,
	STATIC_OUTLINE_SHADER,
	STATIC_OVER_OUTLINE_SHADER,
	SKELETAL_OUTLINE_SHADER,
	SKELETAL_OVER_OUTLINE_SHADER,
	FINAL_SHADER,
	END,
};

class Shader
{
public:
	Shader();
	~Shader();

public:
	string GetName() { return m_Name; }
	void SetName(string name) { m_Name = name; }

public:
	void Initalize(eShaderType type);
	void PipeLineSetting();

private:
	void CreateVS_IL();
	void CreatePS();

private:
	string m_Name;
	eShaderType m_ShaderType;

	ComPtr<ID3D11VertexShader> m_pVertexShader = nullptr;	// ���� ���̴�.
	ComPtr<ID3D11PixelShader>  m_pPixelShader = nullptr;	// �ȼ� ���̴�.
	ComPtr<ID3D11InputLayout>  m_pInputLayout = nullptr;	// �Է� ���̾ƿ�.

	// ������ ���� ������
	// ComPtr<ID3D11VertexShader> m_pEnvironmentVertexShader = nullptr;	// ȯ�� ���� ���̴�.
	// ComPtr<ID3D11PixelShader>  m_pEnvironmentPixelShader = nullptr;	// ȯ�� �ȼ� ���̴�.
};
