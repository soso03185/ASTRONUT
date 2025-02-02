#pragma once
#include "Graphics.h"
#include "PointLightInstance.h"

struct DebugPoint
{
	Vector3 m_Position;
	Color m_Color;
};

struct DebugLine
{
	Vector3 m_StartLine;
	Color m_StartColor;
	Vector3 m_EndLine;
	Color m_EndColor;
};

struct CB_BoolBuffer
{
	int UseNormalMap;
	int UseSpecularMap;
	int UseGamma;
	int UseDiffuseMap; //16

	int UseEmissiveMap;
	int UseOpacityMap;
	int UseMetalnessMap;
	int UseRoughnessMap; //16

	int UseCubeMap;
	float MetalnessValue;
	float RoughnessValue;
	int IsGlass = false;

	float ValueIBL = 1.0f;
	float ValueLightIBL = 1.0f;
	float boolPad[2];
};

struct CB_TransformW
{
	Matrix mWorld;
};
struct CB_TransformVP
{
	Matrix m_View;
	Matrix m_Projection;
	Matrix m_ShadowView;
	Matrix m_ShadowProjection;
};

struct CB_LightDirBuffer
{
	Vector4 vLightDir;
	Vector4 vLightColor;
	Vector4 mWorldCameraPosition;
	Vector4 pad[1];
};

struct CB_IBL
{
	int UseIBL = false;				// 4
	int UseLightIBL = false;        // 4 
	float AmbientOcclusion = 1.0f;	// 4
	int isSkeletal =false;						// 4	16
 	Matrix m_TestLocal;
};

struct CB_UIMesh
{
	int m_PlayerNumber;
	Vector2 m_UIPosition;
	int m_OutLine;
};

// ���ؽ� ���̴��� �Ű��ֱ� ���� ����ϴ�, �ֹ߼� ���� ����.
// �޽��� ����ϰ� �� ���� �޽��� ����ϰ� �ݺ�...�ϱ⶧����
// �ֹ߼� ���� ������ �ϳ��� �־ ��.
//
// ���⿡ ����Ǵ� ����. �޽��� �����ϴ� ���ؽ����� �����ϴ� ��� bone���� offset * boneWorld ����.
// �Ǵ��� bone�� name�̳� index�� ������ �Ǵ��ؼ� �����.
struct CB_MatrixPalette
{
	Matrix Array[128];
};

static const int pointlightMaxCount = 10;

struct CB_PointLight // register (6)
{
	struct
	{
		Vector4 lightPosition = {};	// 16

		float lightRange = 0.0f;
		float linearTerm = 0.0f;;
		float quadraticTerm = 0.0f;;
		float lightIntensity = 0.0f;;	// 16

		Vector3 lightColor = {};  // 16
		float lightPad;
 	} pointLights[pointlightMaxCount];
};
 
class EnvironmentModel;
class EnvironmentMeshComponent;
class SkeletalMeshInstance;
class SkeletalMeshModel;
class StaticMeshModel;
class StaticMeshInstance;
class Material;
class ResourceManager;
class Graphics;
class PipeLine;
class UIInstance;
class TextRenderer;
class ParticleSystem;
class PointLightInstance;
enum class eShaderType;

class D3DRenderManager
{
public:
	D3DRenderManager(UINT Width, UINT Height);
	~D3DRenderManager();

public:
	ComPtr<ID3D11Device> GetDevice() { return m_pDevice; }
	ComPtr<ID3D11DeviceContext> GetDevcon() { return m_pDeviceContext; }
	ComPtr<IDXGISwapChain> GetSwapChain() { return m_pGraphics->GetSwapChain(); }
	ComPtr<ID2D1RenderTarget> GetD2DRenderTarget() { return m_pGraphics->GetD2DRenderTarget(); }

	void SetViewMatrix(Math::Matrix viewMatrix);
	void SetCamMatrix(Math::Vector3 camMatrix);
	void SetEnvironmentModel(const std::shared_ptr<EnvironmentModel>& environmentModel);
	void SetSecondIBL();
	void SetIsOutLine(int PlayerNumber, bool isOutLine) { 
		if (PlayerNumber == 1) m_bIsUIMesh_1_OutLine = isOutLine;
		if (PlayerNumber == 2) m_bIsUIMesh_2_OutLine = isOutLine;
	}

	void AddStaticMeshModel(std::shared_ptr<StaticMeshModel> staticMeshModel) { m_StaticMeshModels.emplace_back(staticMeshModel); }
	void AddSkeletalMeshModel(std::shared_ptr<SkeletalMeshModel> skeletalMeshModel) { m_SkeletalMeshModels.emplace_back(skeletalMeshModel); }
	void AddUIInstance(std::shared_ptr<UIInstance> uiInstance) { m_UIInstanceList.emplace_back(uiInstance); }
	void AddParticle(std::shared_ptr<ParticleSystem> particle) { m_ParticleList.emplace_back(particle); }
	void AddDebugPoint(std::vector<DebugPoint>& point) { m_DebugPointList = point; }
	void AddDebugLine(std::vector<DebugLine>& line) { m_DebugLineList = line; }
	void AddUISkeletalMeshModel(std::shared_ptr<SkeletalMeshModel> skeletalMeshUIModel, int playerNumber, bool isOutLine)
	{
		SetIsOutLine(playerNumber, isOutLine);
		m_UISkeletalMeshModels.emplace_back(skeletalMeshUIModel, playerNumber); 
	}
	void AddPointLightInstance(std::shared_ptr<PointLightInstance> pLight) { m_PointLightInstance.emplace_back(pLight); }


public:
	//  Need  //
	bool Initialize(HWND hWnd);
	bool InitImGUI();
	bool InitD3D();
	void InitScene();

	//   Main Loop   //
	void Update(float deltaTime);		// ������Ʈ
	void RenderShadow();				// �׸��� ����
	void RenderOutLine();				// �ܰ��� ����
	void RenderUIMesh();				// 3D UI Mesh ����
	void RenderPostProcessing();		// ��ó�� ����
	void Render();						// �𵨸� ����

	//  Utility  //
	void ApplyMaterial(Material* pMaterial);												// ���͸��� GPU�� �����ϱ�
	void AddMeshInstance(std::shared_ptr<StaticMeshModel> pModel);							// �޽� �ν��Ͻ� �����ϱ�
	void AddMeshInstance(std::shared_ptr<SkeletalMeshModel> pModel);						// �޽� �ν��Ͻ� �����ϱ�
	void AddShadowMeshInstance(std::shared_ptr<StaticMeshModel> pModel);					// �׸��� �޽� �ν��Ͻ� �����ϱ�
	void AddShadowMeshInstance(std::shared_ptr<SkeletalMeshModel> pModel);					// �׸��� �޽� �ν��Ͻ� �����ϱ�
	void AddOutLineMeshInstance(std::shared_ptr<StaticMeshModel> pModel);					// �ܰ��� �޽� �ν��Ͻ� �����ϱ�
	void AddOutLineMeshInstance(std::shared_ptr<SkeletalMeshModel> pModel);					// �ܰ��� �޽� �ν��Ͻ� �����ϱ�
	void AddUIMeshInstance(std::shared_ptr<StaticMeshModel> pModel, int playerNumber);		// UI �޽� �ν��Ͻ� �����ϱ�
	void AddUIMeshInstance(std::shared_ptr<SkeletalMeshModel> pModel, int playerNumber);	// UI �޽� �ν��Ͻ� �����ϱ�
	void ConstantBuffUpdate();																// ��� ���� ������Ʈ
public:
	void ClearWorldResource();	// ���� �̵� �� ���ҽ� ����

private:
	void CreateConstantBuffer();
	void SortMeshInstance();

	// Environment //
	void SetEnvironment();

	//?   Render    ///
	void ImguiRender();													// ImGui ����
	void RenderStaticMeshInstance(eShaderType shadertype);				// ������ ����ƽ �޽� ����
	void RenderSkeletalMeshInstance(eShaderType shadertype);			// ������ ���̷�Ż �޽� ����
	void RenderStaticMeshInstanceOpacity(eShaderType shadertype);		// ������ ����ƽ �޽� ����
	void RenderSkeletalMeshInstanceOpacity(eShaderType shadertype);		// ������ ���̷�Ż �޽� ����
	void RenderUIMesh_Instacne(int playerNumber);						// Player1 UI 3D �𵨸� �޽� ����
	void RenderUIInstance();											// UI �ν��Ͻ� ����
	void RenderUIMeshTexture_Instance();								// UI �ν��Ͻ� ����
	void RenderDebugDraw();												// ����� ����
	void RenderEnvironment();											// ȯ��� ����
	void RenderParticleEffect();										// ��ƼŬ ����
	void RenderPointLight();											// ����Ʈ����Ʈ ����

	//?   Util     ///
	void GetVideoMemoryInfo(std::string& out);							// ���� �޸� ������ ���ϱ�
	void GetSystemMemoryInfo(std::string& out);							// �ý��� �޸� ������ ���ϱ�

public:
	void DrawTextRequest(std::wstring_view text,
		FLOAT x,
		FLOAT y,
		D2D1::ColorF color,
		std::wstring_view fontFamily,
		FLOAT fontSize,
		DWRITE_FONT_WEIGHT fontWeight = DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE fontStyle = DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH fontStretch = DWRITE_FONT_STRETCH_NORMAL);

public:
	///   D3D   ///
	static D3DRenderManager* Instance;
	static ComPtr<ID3D11Device> m_pDevice;					 // ����̽�

private:
	ComPtr<ID3D11DeviceContext> m_pDeviceContext;
	ComPtr<IDXGIFactory4> m_pDXGIFactory;		// DXGI���丮
	ComPtr<IDXGIAdapter3> m_pDXGIAdapter;		// ����ī�� ������ ���� ������ �������̽�
	UINT m_ClientWidth;
	UINT m_ClientHeight;
	HWND m_hWnd;

	//?   CB Data   ///
	ComPtr<ID3D11Buffer> m_pBoolBuffer = nullptr;		    // ��� ����.
	ComPtr<ID3D11Buffer> m_pLightBuffer = nullptr;		    // ��� ����.
	ComPtr<ID3D11Buffer> m_pMatPalette = nullptr;		    // ��� ����.
	ComPtr<ID3D11Buffer> m_pIBL_Buffer = nullptr;			// ��� ����.
	ComPtr<ID3D11Buffer> m_pPointLight_Buffer = nullptr;	// ��� ����.
	ComPtr<ID3D11Buffer> m_pTransformW_Buffer = nullptr;	// ��� ����.
	ComPtr<ID3D11Buffer> m_pTransformVP_Buffer = nullptr;	// ��� ����.
	ComPtr<ID3D11Buffer> m_pUIAnimation_Buffer = nullptr;	// ��� ����.
	ComPtr<ID3D11Buffer> m_pFadeInOut_Buffer = nullptr;		// ��� ����.
	ComPtr<ID3D11Buffer> m_pParticleBuffer = nullptr;		// ��� ����.
	ComPtr<ID3D11Buffer> m_pParticleDataBuffer = nullptr;	// ��� ����.
	ComPtr<ID3D11Buffer> m_pUIMeshBuffer = nullptr;			// ��� ����.

	CB_TransformVP m_TransformVP;
	CB_TransformW m_TransformW;
	CB_BoolBuffer m_bIsMapUse;
	CB_IBL m_IBL;

	BoundingFrustum m_FrustumCamera;
	BoundingFrustum m_FrustumShadow;
	///   RENDERER   ///
	std::unique_ptr<TextRenderer> m_textRenderer;

	///   CONTAINER   ///
	std::list<std::shared_ptr<StaticMeshModel>>	m_StaticMeshModels;								//  �������� �𵨵��� ������ �����صд�.
	std::list<std::shared_ptr<StaticMeshModel>>	m_DebugStaticMeshModels;						//  ����� �������� �𵨵��� ������ �����صд�.
	std::list<std::shared_ptr<SkeletalMeshModel>> m_SkeletalMeshModels;							//  �������� �𵨵��� ������ �����صд�.
	std::list<std::shared_ptr<SkeletalMeshModel>> m_DebugSkeletalMeshModels;					//  ����� �������� �𵨵��� ������ �����صд�.
	std::list<std::pair<std::shared_ptr<SkeletalMeshModel>, int>>	m_UISkeletalMeshModels;		//  �������� UI�𵨵��� ������ �����صд�.
	std::list<std::shared_ptr<PointLightInstance>> m_PointLightInstance;						// ����Ʈ����Ʈ���� ������ ���� 

	std::list<StaticMeshInstance*> m_StaticMeshInstance;										//  �������� �޽õ��� ������ �����صд�.
	std::list<StaticMeshInstance*> m_ShadowStaticMeshInstance;									//  �������� �׸��� �޽õ��� ������ �����صд�.
	std::list<StaticMeshInstance*> m_OutLineStaticMeshInstance;									//  �������� �׸��� �޽õ��� ������ �����صд�.
	std::list<StaticMeshInstance*> m_StaticMeshInstanceOpacity;									//  �������� ������ �޽õ��� ������ �����صд�.
	std::list<SkeletalMeshInstance*> m_SkeletalMeshInstance;									//  �������� �޽õ��� ������ �����صд�.
	std::list<SkeletalMeshInstance*> m_ShadowSkeletalMeshInstance;								//  �������� �׸��� �޽õ��� ������ �����صд�.
	std::list<SkeletalMeshInstance*> m_OutLineSkeletalMeshInstance;								//  �������� �׸��� �޽õ��� ������ �����صд�.
	std::list<SkeletalMeshInstance*> m_SkeletalMeshInstanceOpacity;								//  �������� ������ �޽õ��� ������ �����صд�.

	std::list<std::shared_ptr<UIInstance>> m_UIInstanceList;									//  �������� UI���� ������ �����صд�.
	std::list<std::shared_ptr<ParticleSystem>> m_ParticleList;									//  �������� ��ƼŬ���� ������ �����صд�.
	std::vector<DebugPoint> m_DebugPointList;													//  �������� ������ ����� ����Ʈ���� �����͵��� �����صд�.
	std::vector<DebugLine> m_DebugLineList;														//  �������� ������ ����� ���ε��� �����͵��� �����صд�.
	std::list<std::pair<SkeletalMeshInstance*, int>> m_UISkeletalMesh_Instance;					//  UIâ�� �������� Player1 3D �𵨸� �����͵��� �����صд�.
	std::list<std::pair<SkeletalMeshInstance*, int>> m_UISkeletalMesh_InstanceOpacity;			//  UIâ�� �������� Player1 3D �𵨸� �����͵��� �����صд�.

	std::shared_ptr<EnvironmentModel> m_pEnvironmentModel;										// IBL
	std::shared_ptr<EnvironmentModel> m_pLightEnvironment;										// IBL

	std::shared_ptr<UIInstance> m_PostProcessRenderInstance;									// 3D ���� �� �� ���� ȭ�鿡 ��� ȭ�� ũ���� 2D �簢�� ����
	std::shared_ptr<UIInstance> m_UIMesh_1_RenderInstance;										// 3D ���� �� �� UI�� ��� ȭ�� ũ���� 2D �簢�� ����
	std::shared_ptr<UIInstance> m_UIMeshOutLine_1_RenderInstance;								// 3D ���� �� �� UI�� ��� ȭ�� ũ���� 2D �簢�� ����
	std::shared_ptr<UIInstance> m_UIMesh_2_RenderInstance;										// 3D ���� �� �� UI�� ��� ȭ�� ũ���� 2D �簢�� ����
	std::shared_ptr<UIInstance> m_UIMeshOutLine_2_RenderInstance;								// 3D ���� �� �� UI�� ��� ȭ�� ũ���� 2D �簢�� ����
	bool m_bIsUIMesh_1_OutLine = false;
	bool m_bIsUIMesh_2_OutLine = false;	
	bool m_bIsUIMesh_1_Move = false;
	bool m_bIsUIMesh_2_Move = false;
	float m_UIMesh_1_MoveTime;
	float m_UIMesh_2_MoveTime;
	Matrix m_Test;
	Matrix m_UITransform;

private:
	///  FOR SHADER  ///
	Matrix   m_View;					// ī�޶���ǥ�� �������� ��ȯ�� ���� ���.
	Matrix   m_Projection;				// ������ġ��ǥ��( Normalized Device Coordinate) �������� ��ȯ�� ���� ���.
	Matrix   m_UIView;					// ī�޶���ǥ�� �������� ��ȯ�� ���� ���.
	Matrix   m_UIProjection;			// ������ġ��ǥ��( Normalized Device Coordinate) �������� ��ȯ�� ���� ���.
	Matrix   m_Orthographic;			// 2D �������� ���� ���翵 ���

	Vector3 m_UICamBasePosition;		// UI Mesh �⺻ ī�޶� ��ġ
	Vector3 m_UICamUpPosition;			// UI Mesh ����� ī�޶� ��ġ
	Math::Vector3   m_CamPosition;		// ī�޶� ��ġ
	 

	//float m_Cam[3] = { 0.0f, 0.0f, -500.0f };
	float m_Fov = 45.0f;
	float m_Near = 0.1f;
	float m_Far = 1300000.0f;

	float m_SpecularPower = 4.0f;
	float m_Ambient = 0.369f;

	float m_vLightColor[3] = { 1, 1, 1 };
	float m_vLightDir[3] = { 0, -1, 1 };

	//float m_PointLightPos[3] = { 0,0,0 };
	//float m_PointLightColor[3] = { 1, 1, 1 };
	//float m_PointLightRange = 100.0f;
	//float m_PointLightLinearTerm = 1.0f;
	//float m_PointLightQuadraTicTerm = 0.0f;
	//float m_PointLightIntencity = 0.0f;

	bool m_isUseEdit = false;
	//bool m_isUsePointLight = false;

	bool m_isUseIBL = true;
	bool m_isCubeMap = true;
	bool m_isNormalMap = true;
	bool m_isSpecularMap = true;
	bool m_isGamma = true;
	bool m_isDiffuse = true;
	bool m_isEmissive = true;
	bool m_isOpacity = true;
	bool m_isMetalness = true;
	bool m_isRoughness = true;
	bool m_UseLightIBL = true;

	float m_MetalnessValue = 1.0f;
	float m_RoughnessValue = 0.5f;
	float m_LightIBLValue = 1.0f;
	float m_IBLValue = 1.0f;

	int m_nDrawRenderCount;

	bool m_IsFreezeCulling = false;
	bool m_IsDrawDebugCulling = false;
	bool m_IsDrawShadowFrustum = true;
	bool m_IsDrawDebugCollision = true;

	// ���� : Shadow
	Matrix m_ShadowView;
	Matrix m_ShadowProjection;
	Vector3 m_ShadowLookAt;
	Vector3 m_ShadowPos;
	float m_ShadowForwardDistFromCamera = 1500.f;
	float m_ShadowUpDistFromLookAt = 5000.f;

	float m_GameTime;

private:
	// ���� : �׷��Ƚ�, ����������
	std::shared_ptr<Graphics> m_pGraphics;
	std::shared_ptr<PipeLine> m_pPipeLine;
};
