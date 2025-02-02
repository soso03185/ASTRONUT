#include "pch.h"
#include "Helper.h"
#include "StaticMeshModel.h"
#include "SkeletalMeshModel.h"
#include "D3DRenderManager.h"
#include "Material.h"
#include "ModelResource.h"
#include "EnvironmentModel.h"
#include "ResourceManager.h"
#include "Graphics.h"
#include "PipeLine.h"
#include "DebugDraw.h"
#include "UIInstance.h"
#include "TextRenderer.h"
#include "ParticleSystem.h"

#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#pragma comment (lib, "DXGI.lib")  // for  CreateDXGIFactory1 �Լ�
#pragma comment (lib, "d2d1.lib")
#pragma comment (lib, "dwrite.lib")

#define USE_FLIPMODE 1

// DirectX::BoundingSphere 
// ToDo :: ���� -> �̰� �������� (����Ʈ����Ʈ �ø�)

D3DRenderManager* D3DRenderManager::Instance = nullptr;
ComPtr<ID3D11Device> D3DRenderManager::m_pDevice = nullptr;

// ModelResource -> ������ �������� ������ ���
// MeshInstance  -> ������ �ϱ� ���� �� �޽��� ������
// MeshComponent -> �����͸� ��� �ִ� ������


/// ������ ///
D3DRenderManager::D3DRenderManager(UINT Width, UINT Height)
{
	m_ClientHeight = {};
	m_ClientWidth = {};
	m_hWnd = {};
	m_GameTime = 0.f;

	assert(Instance == nullptr);
	Instance = this;

	m_ClientWidth = Width;
	m_ClientHeight = Height;

	//m_IsDrawDebugCulling = false;
}


/// �Ҹ��� ///
D3DRenderManager::~D3DRenderManager()
{
	// Model clear
	{
		for (auto& modelPtr : m_SkeletalMeshModels)
		{
			modelPtr.reset();
		}
		for (auto& modelPtr : m_StaticMeshModels)
		{
			modelPtr.reset();
		}
		for (auto& modelPtr : m_DebugStaticMeshModels)
		{
			modelPtr.reset();
		}
		for (auto& modelPtr : m_DebugSkeletalMeshModels)
		{
			modelPtr.reset();
		}
		for (auto& modelPtr : m_UISkeletalMeshModels)
		{
			modelPtr.first.reset();
		}

		m_StaticMeshModels.clear();				// Skeletal Mesh Model Ŭ����
		m_SkeletalMeshModels.clear();			// Static Mesh Model Ŭ����
		m_DebugStaticMeshModels.clear();		// Debug Static Mesh Model Ŭ����
		m_DebugSkeletalMeshModels.clear();		// Debug Skeletal Mesh Model Ŭ����
		m_UISkeletalMeshModels.clear();			// UI Skeletal Mesh Model Ŭ����
	}

	ResourceManager::GetInstance()->Finalize();

	DebugDraw::Uninitialize();

	// Cleanup Imgui
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}


/// Update ///
void D3DRenderManager::Update(float deltaTime)
{
	m_GameTime += deltaTime;
	float fov = m_Fov / 180.0f * 3.14f;
	float aspectRatio = (FLOAT)m_ClientWidth / (FLOAT)m_ClientHeight;
	m_Projection = XMMatrixPerspectiveFovLH(fov, aspectRatio, m_Near, m_Far);
	m_UIProjection = XMMatrixPerspectiveFovLH(fov, aspectRatio, m_Near, m_Far);

	ConstantBuffUpdate();

	m_nDrawRenderCount = 0;

	// ����ƽ �޽� �ν��Ͻ� �߰� �� �׸��� �ø�
	for (const auto& StaticMeshModel : m_StaticMeshModels)	// deleteGameObject �Ŀ� �� ������ ����
	{
		m_DebugStaticMeshModels.emplace_back(StaticMeshModel);

		DirectX::BoundingBox boundingBox = StaticMeshModel->m_BoundingBox;
		boundingBox.Center.x += StaticMeshModel->m_worldMatrix->Translation().x;
		boundingBox.Center.y += StaticMeshModel->m_worldMatrix->Translation().y;
		boundingBox.Center.z += StaticMeshModel->m_worldMatrix->Translation().z;

		if (m_FrustumShadow.Intersects(boundingBox))
		{
			AddShadowMeshInstance(StaticMeshModel);
		}

		if (StaticMeshModel->m_IsOutLine)
		{
			AddOutLineMeshInstance(StaticMeshModel);
		}

		if (m_FrustumCamera.Intersects(boundingBox))
		{
			// �ϳ��� �޽� ������Ʈ�� �������� �޽� Instance �� ����������.
			AddMeshInstance(StaticMeshModel);
			StaticMeshModel->m_IsCulled = true;
			m_nDrawRenderCount++;
		}
		else
		{
			StaticMeshModel->m_IsCulled = false;
		}
	}

	// ���̷�Ż �� �ν��Ͻ� �߰� �� �׸��� �ø�
	for (const auto& SkeletalMeshModel : m_SkeletalMeshModels)
	{
		m_DebugSkeletalMeshModels.emplace_back(SkeletalMeshModel);
		DirectX::BoundingBox boundingBox = SkeletalMeshModel->m_BoundingBox;
		boundingBox.Center.x += SkeletalMeshModel->m_worldMatrix->Translation().x;
		boundingBox.Center.y += SkeletalMeshModel->m_worldMatrix->Translation().y;
		boundingBox.Center.z += SkeletalMeshModel->m_worldMatrix->Translation().z;

		// �ϳ��� �޽� ������Ʈ�� �������� �޽� Instance �� ����������.
		AddMeshInstance(SkeletalMeshModel);
		SkeletalMeshModel->m_IsCulled = true;
		m_nDrawRenderCount++;

		if (m_FrustumShadow.Intersects(boundingBox))
		{
			AddShadowMeshInstance(SkeletalMeshModel);
		}

		if (SkeletalMeshModel->m_IsOutLine)
		{
			AddOutLineMeshInstance(SkeletalMeshModel);
		}
	}
	// UI Mesh Model �ν��Ͻ� �߰�
	for (const auto& SkeletalMeshModel : m_UISkeletalMeshModels)
	{
		SkeletalMeshModel.first->SetWorldMatrix(&m_UITransform);
		AddUIMeshInstance(SkeletalMeshModel.first, SkeletalMeshModel.second);
		SkeletalMeshModel.first->m_IsCulled = true;
	}

	SortMeshInstance();

	// UIInstance Animation ����� ���� ������Ʈ
	for (const auto& ui : m_UIInstanceList)
	{
		ui->Update(deltaTime);
	}

	// ��ƼŬ ����� ���� ������Ʈ
	for (auto& particle : m_ParticleList)
	{
		particle->SetEyePos(m_CamPosition);
		particle->Update(deltaTime, m_GameTime);
	}

	//m_DebugSkeletalMeshModels.clear();
	//m_DebugStaticMeshModels.clear();
	m_StaticMeshModels.clear();
	m_SkeletalMeshModels.clear();
	m_UISkeletalMeshModels.clear();
}


/// Constant Buffer Update ( ��, ī�޶� ) ///
void D3DRenderManager::ConstantBuffUpdate()
{
	///  ConstantBuffer Binding  ///
	CB_BoolBuffer CB_Bool;
	CB_Bool.UseGamma = m_isGamma;

	CB_LightDirBuffer CB_Light;
	CB_Light.vLightColor.x = m_vLightColor[0];
	CB_Light.vLightColor.y = m_vLightColor[1];
	CB_Light.vLightColor.z = m_vLightColor[2];
	CB_Light.vLightColor.w = 1.0f;

	CB_Light.vLightDir.x = m_vLightDir[0];
	CB_Light.vLightDir.y = m_vLightDir[1];
	CB_Light.vLightDir.z = m_vLightDir[2];
	CB_Light.vLightDir.w = 1.0f;

	CB_Light.vLightDir.Normalize();

	// ToDo �ӽ÷� ī�޶� ��ġ ��������. ������ �ð� ������ ����.
	CB_Light.mWorldCameraPosition = XMVectorSet(m_CamPosition.x, m_CamPosition.y, m_CamPosition.z, 0.0f);

	// ī�޶� �ø� �������� ī�޶� �����
	if (!m_IsFreezeCulling)
	{
		BoundingFrustum::CreateFromMatrix(m_FrustumCamera, m_Projection);
		m_FrustumCamera.Transform(m_FrustumCamera, m_View.Invert());
	}
	// ī�޶�
	{
		m_TransformVP.m_View = XMMatrixTranspose(m_View);
		m_TransformVP.m_Projection = XMMatrixTranspose(m_Projection);
	}
	// �׸���
	{
		Vector3 lightDir;
		memcpy(&lightDir, m_vLightDir, sizeof(Vector3));

		Vector3 forward = m_View.Forward() * m_ShadowForwardDistFromCamera / 1.2;

		// �׸��� View ���, Projection ��� �����
		m_ShadowProjection = XMMatrixPerspectiveFovLH(XM_PIDIV4, SHADOWMAP_SIZE / SHADOWMAP_SIZE, 4000.f, 200000.f);
		m_ShadowLookAt = { m_CamPosition.x + forward.x, 0.f, m_CamPosition.z - forward.z };
		m_ShadowPos = m_ShadowLookAt + (-lightDir * m_ShadowUpDistFromLookAt);
		m_ShadowView = XMMatrixLookAtLH(m_ShadowPos, m_ShadowLookAt, Vector3(0.f, 1.f, 0.f));
		m_ShadowView = m_ShadowView;

		// ��� ���ۿ� �׸��� View���, Projection ��� �����ϱ�
		m_TransformVP.m_ShadowView = XMMatrixTranspose(m_ShadowView);
		m_TransformVP.m_ShadowProjection = XMMatrixTranspose(m_ShadowProjection);

		// �׸��� ����ü �ø��� ���� �׸��� ����ü �����
		BoundingFrustum::CreateFromMatrix(m_FrustumShadow, m_ShadowProjection);
		m_FrustumShadow.Transform(m_FrustumShadow, m_ShadowView.Invert());
	}

	// ī�޶�, �׸��� ��,�������� ��� ���� ����
	// IBL ��� ���� ����
	// Light ��� ���� ����
	m_pGraphics->GetDeviceContext()->UpdateSubresource(m_pTransformVP_Buffer.Get(), 0, nullptr, &m_TransformVP, 0, 0);
	m_pGraphics->GetDeviceContext()->UpdateSubresource(m_pIBL_Buffer.Get(), 0, nullptr, &m_IBL, 0, 0);
	m_pGraphics->GetDeviceContext()->UpdateSubresource(m_pLightBuffer.Get(), 0, nullptr, &CB_Light, 0, 0);
}

// �޽� �ν��Ͻ� ����
void D3DRenderManager::AddMeshInstance(std::shared_ptr<StaticMeshModel> pModel)
{
	// ����ƽ �޽� �߿��� ���Ľ�Ƽ(����)�� �ִ� �޽ô� StaticMeshInstanceOpacity ����Ʈ�� ����
	// �ƴϸ� �������� ����ƽ �޽ôϱ� StaticMeshInstance ����Ʈ�� ����
	for (size_t i = 0; i < pModel->m_MeshInstances.size(); i++)
	{
		auto* pMeshInstance = &pModel->m_MeshInstances[i];

		DirectX::BoundingBox boundingBox = *pMeshInstance->m_pBoundingBox;
		boundingBox.Center.x += pMeshInstance->m_pNodeWorldTransform->Translation().x;
		boundingBox.Center.y += pMeshInstance->m_pNodeWorldTransform->Translation().y;
		boundingBox.Center.z += pMeshInstance->m_pNodeWorldTransform->Translation().z;

		if (m_FrustumCamera.Intersects(boundingBox))
		{
			m_nDrawRenderCount++;
			if (pMeshInstance->m_pMaterial->m_MaterialMapFlags & MaterialMapFlags::OPACITY)
			{
				m_StaticMeshInstanceOpacity.emplace_back(&pModel->m_MeshInstances[i]);
			}
			else
			{
				m_StaticMeshInstance.emplace_back(&pModel->m_MeshInstances[i]);
			}
		}
	}
}


/// Mesh Instance �߰� ///
void D3DRenderManager::AddMeshInstance(std::shared_ptr<SkeletalMeshModel> pModel)
{
	// ����ƽ �޽� �߿��� ���Ľ�Ƽ(����)�� �ִ� �޽ô� m_SkeletalMeshInstanceOpacity ����Ʈ�� ����
	// �ƴϸ� �������� ����ƽ �޽ôϱ� m_SkeletalMeshInstance ����Ʈ�� ����
	for (size_t i = 0; i < pModel->m_MeshInstances.size(); i++)
	{
		auto* pMeshInstance = &pModel->m_MeshInstances[i];

		assert(pMeshInstance->m_pMaterial != nullptr);

		if (pMeshInstance->m_pMaterial != nullptr && pMeshInstance->m_pMaterial->m_MaterialMapFlags & MaterialMapFlags::OPACITY)
		{
			m_SkeletalMeshInstanceOpacity.emplace_back(&pModel->m_MeshInstances[i]);
		}
		else
		{
			m_SkeletalMeshInstance.emplace_back(&pModel->m_MeshInstances[i]);
		}
	}
}


/// Shadow Mesh Instance �߰� ///
void D3DRenderManager::AddShadowMeshInstance(std::shared_ptr<SkeletalMeshModel> pModel)
{
	// �׸��� �ø��� �޽� �ν��Ͻ�
	for (size_t i = 0; i < pModel->m_MeshInstances.size(); i++)
	{
		auto* pMeshInstance = &pModel->m_MeshInstances[i];

		assert(pMeshInstance->m_pMaterial != nullptr);

		if (!(pMeshInstance->m_pMaterial->m_MaterialMapFlags & MaterialMapFlags::OPACITY))
		{
			m_ShadowSkeletalMeshInstance.emplace_back(&pModel->m_MeshInstances[i]);
		}
	}
}


/// Shadow Mesh Instance �߰� ///
void D3DRenderManager::AddShadowMeshInstance(std::shared_ptr<StaticMeshModel> pModel)
{
	// �׸��� �ø��� �޽� �ν��Ͻ�
	for (size_t i = 0; i < pModel->m_MeshInstances.size(); i++)
	{
		auto* pMeshInstance = &pModel->m_MeshInstances[i];

		assert(pMeshInstance->m_pMaterial != nullptr);

		DirectX::BoundingBox boundingBox = *pMeshInstance->m_pBoundingBox;
		boundingBox.Center.x += pMeshInstance->m_pNodeWorldTransform->Translation().x;
		boundingBox.Center.y += pMeshInstance->m_pNodeWorldTransform->Translation().y;
		boundingBox.Center.z += pMeshInstance->m_pNodeWorldTransform->Translation().z;

		if (m_FrustumShadow.Intersects(boundingBox) && boundingBox.Extents.y < 700.f)
		{
			if (!(pMeshInstance->m_pMaterial->m_MaterialMapFlags & MaterialMapFlags::OPACITY))
			{
				m_ShadowStaticMeshInstance.emplace_back(pMeshInstance);
			}
		}
	}
}
 

/// OutLine Mesh Instance �߰� ///
void D3DRenderManager::AddOutLineMeshInstance(std::shared_ptr<StaticMeshModel> pModel)
{
	// �ܰ��� �޽� �ν��Ͻ�
	for (auto& meshInstance : pModel->m_MeshInstances)
	{
		m_OutLineStaticMeshInstance.emplace_back(&meshInstance);
	}
}


/// OutLine Mesh Instance �߰� ///
void D3DRenderManager::AddOutLineMeshInstance(std::shared_ptr<SkeletalMeshModel> pModel)
{
	// �ܰ��� �޽� �ν��Ͻ�
	for (auto& meshInstance : pModel->m_MeshInstances)
	{
		m_OutLineSkeletalMeshInstance.emplace_back(&meshInstance);
	}
}


/// UI Mesh Instance �߰� ///
void D3DRenderManager::AddUIMeshInstance(std::shared_ptr<SkeletalMeshModel> pModel, int playerNumber)
{
	// UI ���̷�Ż �޽� �ν��Ͻ�
	for (size_t i = 0; i < pModel->m_MeshInstances.size(); i++)
	{
		auto* pMeshInstance = &pModel->m_MeshInstances[i];

		assert(pMeshInstance->m_pMaterial != nullptr);

		if (playerNumber == 1)
		{
			if (pMeshInstance->m_pMaterial != nullptr && pMeshInstance->m_pMaterial->m_MaterialMapFlags & MaterialMapFlags::OPACITY)
			{
				m_UISkeletalMesh_Instance.emplace_back(&pModel->m_MeshInstances[i], playerNumber);
			}
			else
			{
				m_UISkeletalMesh_InstanceOpacity.emplace_back(&pModel->m_MeshInstances[i], playerNumber);
			}
		}
		if (playerNumber == 2)
		{
			if (pMeshInstance->m_pMaterial != nullptr && pMeshInstance->m_pMaterial->m_MaterialMapFlags & MaterialMapFlags::OPACITY)
			{
				m_UISkeletalMesh_Instance.emplace_back(&pModel->m_MeshInstances[i], playerNumber);
			}
			else
			{
				m_UISkeletalMesh_InstanceOpacity.emplace_back(&pModel->m_MeshInstances[i], playerNumber);
			}
		}
	}
}


/// Shadow Render ( �׸��ڿ� ���� ���۸� ���� ���� ) ///
void D3DRenderManager::RenderShadow()
{
	m_pGraphics->RenderBeginShadow();
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->VSSetConstantBuffers(3, 1, m_pTransformVP_Buffer.GetAddressOf());

	// ���������ο� ����ƽ �޽� ���̴��� InputLayout, VS_Shader, PS_Shader ����
	RenderStaticMeshInstance(eShaderType::STATIC_SHADOW_SHADER);				// ������ ����ƽ �޽�
	RenderSkeletalMeshInstance(eShaderType::SKELETAL_SHADOW_SHADER);			// ������ ���̷�Ż �޽�
}


/// OutLine Render ( �ܰ��� ���� ���۸� ���� ���� )	///
void D3DRenderManager::RenderOutLine()
{
	// ���� �� ũ�� ����
	m_pGraphics->RenderBeginOutLine();
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->VSSetConstantBuffers(3, 1, m_pTransformVP_Buffer.GetAddressOf());

	RenderStaticMeshInstance(eShaderType::STATIC_OUTLINE_SHADER);						// ����ƽ �޽�
	RenderSkeletalMeshInstance(eShaderType::SKELETAL_OUTLINE_SHADER);					// ���̷�Ż �޽�
	RenderStaticMeshInstanceOpacity(eShaderType::STATIC_OUTLINE_SHADER);				// ������ ����ƽ �޽�
	RenderSkeletalMeshInstanceOpacity(eShaderType::SKELETAL_OUTLINE_SHADER);			// ������ ���̷�Ż �޽�

	// �ܰ��� ũ�� ����
	m_pGraphics->RenderBeginOutLineOver();
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->VSSetConstantBuffers(3, 1, m_pTransformVP_Buffer.GetAddressOf());

	RenderStaticMeshInstance(eShaderType::STATIC_OVER_OUTLINE_SHADER);					// ����ƽ �޽�
	RenderSkeletalMeshInstance(eShaderType::SKELETAL_OVER_OUTLINE_SHADER);				// ���̷�Ż �޽�
	RenderStaticMeshInstanceOpacity(eShaderType::STATIC_OVER_OUTLINE_SHADER);			// ������ ����ƽ �޽�
	RenderSkeletalMeshInstanceOpacity(eShaderType::SKELETAL_OVER_OUTLINE_SHADER);		// ������ ���̷�Ż �޽�
}


/// 3D UI Mesh Render ( 3D UI Mesh ���� ) ///
void D3DRenderManager::RenderUIMesh()
{
	m_TransformVP.m_View = XMMatrixTranspose(m_UIView);
	m_TransformVP.m_Projection = XMMatrixTranspose(m_UIProjection);
	m_pDeviceContext->UpdateSubresource(m_pTransformVP_Buffer.Get(), 0, nullptr, &m_TransformVP, 0, 0);
	m_pGraphics->GetDeviceContext()->OMSetBlendState(m_pPipeLine->GetAlphaBlendState().Get(), nullptr, 0xffffffff); // ���ĺ��� ���¼��� , �ٸ��ɼ��� �⺻��

	m_pGraphics->RenderBeginUIMesh_1();
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->VSSetConstantBuffers(3, 1, m_pTransformVP_Buffer.GetAddressOf());
	m_pDeviceContext->VSSetConstantBuffers(5, 1, m_pMatPalette.GetAddressOf());
	RenderUIMesh_Instacne(1);

	m_pGraphics->RenderBeginUIMesh_2();
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->VSSetConstantBuffers(3, 1, m_pTransformVP_Buffer.GetAddressOf());
	m_pDeviceContext->VSSetConstantBuffers(5, 1, m_pMatPalette.GetAddressOf());
	RenderUIMesh_Instacne(2);

	m_TransformVP.m_View = XMMatrixTranspose(m_View);
	m_TransformVP.m_Projection = XMMatrixTranspose(m_Projection);
	m_pDeviceContext->UpdateSubresource(m_pTransformVP_Buffer.Get(), 0, nullptr, &m_TransformVP, 0, 0);
	m_pGraphics->GetDeviceContext()->OMSetBlendState(nullptr, nullptr, 0xffffffff);	// �������� , �ٸ��ɼ��� �⺻��
}


/// ���� ���� Ÿ�ٿ� �׸��� ( �ܰ��� ó���� �ϱ� ���� ) ///
void D3DRenderManager::RenderPostProcessing()
{
	m_pGraphics->RenderBeginPostProcessing();
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->VSSetConstantBuffers(3, 1, m_pTransformVP_Buffer.GetAddressOf());
	m_pDeviceContext->UpdateSubresource(m_pTransformVP_Buffer.Get(), 0, nullptr, &m_TransformVP, 0, 0);

	shared_ptr<Shader> shader = ResourceManager::GetInstance()->CreateShaderResource("FinalShader", eShaderType::FINAL_SHADER);
	shader->PipeLineSetting();
	m_PostProcessRenderInstance->Render(m_pDeviceContext.Get());
}


/// Render ///
void D3DRenderManager::Render()
{
	// �׸��� �� ����
	RenderShadow();
	// �ܰ��� �� ����
	RenderOutLine();
	// 3D UI Mesh ����
	RenderUIMesh();

	/// ������ �ϱ� ���� ���������� ���� ���� ///
	// RenderTargetView�ϰ� SwapChain�� �ִ� Graphics���� RenderBegin�� RenderEnd�Լ� ȣ��
	{
		m_pGraphics->RenderBegin();
		m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// vertex shader
		m_pDeviceContext->VSSetConstantBuffers(0, 1, m_pIBL_Buffer.GetAddressOf());
		m_pDeviceContext->VSSetConstantBuffers(1, 1, m_pBoolBuffer.GetAddressOf());
		m_pDeviceContext->VSSetConstantBuffers(2, 1, m_pTransformW_Buffer.GetAddressOf());
		m_pDeviceContext->VSSetConstantBuffers(3, 1, m_pTransformVP_Buffer.GetAddressOf());
		m_pDeviceContext->VSSetConstantBuffers(4, 1, m_pLightBuffer.GetAddressOf());
		m_pDeviceContext->VSSetConstantBuffers(5, 1, m_pMatPalette.GetAddressOf());
		m_pDeviceContext->VSSetConstantBuffers(6, 1, m_pUIAnimation_Buffer.GetAddressOf());
		m_pDeviceContext->VSSetConstantBuffers(8, 1, m_pParticleBuffer.GetAddressOf());
		m_pDeviceContext->VSSetConstantBuffers(9, 1, m_pParticleDataBuffer.GetAddressOf());
		m_pDeviceContext->VSSetConstantBuffers(10, 1, m_pUIMeshBuffer.GetAddressOf());

		// pixel shader
		m_pDeviceContext->PSSetConstantBuffers(0, 1, m_pIBL_Buffer.GetAddressOf());
		m_pDeviceContext->PSSetConstantBuffers(1, 1, m_pBoolBuffer.GetAddressOf());
		m_pDeviceContext->PSSetConstantBuffers(2, 1, m_pTransformW_Buffer.GetAddressOf());
		m_pDeviceContext->PSSetConstantBuffers(3, 1, m_pTransformVP_Buffer.GetAddressOf());
		m_pDeviceContext->PSSetConstantBuffers(4, 1, m_pLightBuffer.GetAddressOf());
		m_pDeviceContext->PSSetConstantBuffers(6, 1, m_pPointLight_Buffer.GetAddressOf());
		m_pDeviceContext->PSSetConstantBuffers(7, 1, m_pFadeInOut_Buffer.GetAddressOf());
		m_pDeviceContext->PSSetConstantBuffers(8, 1, m_pParticleBuffer.GetAddressOf());
		m_pDeviceContext->PSSetConstantBuffers(9, 1, m_pParticleDataBuffer.GetAddressOf());
		m_pDeviceContext->PSSetConstantBuffers(10, 1, m_pUIMeshBuffer.GetAddressOf());

		// Pixel Shader Resource
		m_pDeviceContext->PSSetShaderResources(11, 1, m_pGraphics->GetShaderResourceView().GetAddressOf());
		m_pDeviceContext->PSSetShaderResources(15, 1, m_pGraphics->GetOutLineResourceView().GetAddressOf());
		m_pDeviceContext->PSSetShaderResources(16, 1, m_pGraphics->GetOutLineOverResourceView().GetAddressOf());

		// Geometry shader
		m_pDeviceContext->GSSetConstantBuffers(8, 1, m_pParticleBuffer.GetAddressOf());
		m_pDeviceContext->GSSetConstantBuffers(9, 1, m_pParticleDataBuffer.GetAddressOf());
	}

	/// ȯ��� ���� ///
	{
		RenderEnvironment();
	}

	/// 3D ���� ///
	// ����ť ����
	// ���������ο� ����ƽ �޽� ���̴��� InputLayout, VS_Shader, PS_Shader ����
	{
		m_IBL.UseLightIBL = false;																			//
		m_IBL.isSkeletal = false;																			//
		m_pGraphics->GetDeviceContext()->UpdateSubresource(m_pIBL_Buffer.Get(), 0, nullptr, &m_IBL, 0, 0);	//
		RenderStaticMeshInstance(eShaderType::STATIC_SHADER);				// ������ ����ƽ �޽�

		m_IBL.UseLightIBL = true;																			//
		m_IBL.isSkeletal = true;																			//
		m_pGraphics->GetDeviceContext()->UpdateSubresource(m_pIBL_Buffer.Get(), 0, nullptr, &m_IBL, 0, 0);	//
		RenderSkeletalMeshInstance(eShaderType::SKELETAL_SHADER);			// ������ ���̷�Ż �޽�
		m_pDeviceContext->OMSetDepthStencilState(m_pPipeLine->GetNoDepthWritesDSS().Get(), 0);

		m_IBL.UseLightIBL = false;																			//
		m_IBL.isSkeletal = false;																			//
		m_pGraphics->GetDeviceContext()->UpdateSubresource(m_pIBL_Buffer.Get(), 0, nullptr, &m_IBL, 0, 0);	//
		RenderStaticMeshInstanceOpacity(eShaderType::STATIC_SHADER);		// ������ ����ƽ �޽�

		m_IBL.UseLightIBL = true;																			//
		m_IBL.isSkeletal = true;																			//
		m_pGraphics->GetDeviceContext()->UpdateSubresource(m_pIBL_Buffer.Get(), 0, nullptr, &m_IBL, 0, 0);  //
		RenderSkeletalMeshInstanceOpacity(eShaderType::SKELETAL_SHADER);	// ������ ���̷�Ż �޽�
		
		m_IBL.UseLightIBL = false;																			//
		m_IBL.isSkeletal = false;																			//
		m_pGraphics->GetDeviceContext()->UpdateSubresource(m_pIBL_Buffer.Get(), 0, nullptr, &m_IBL, 0, 0);	//

		m_pDeviceContext->OMSetDepthStencilState(m_pPipeLine->GetLessEqualDSS().Get(), 0);
		RenderParticleEffect();

		m_pDeviceContext->OMSetDepthStencilState(m_pPipeLine->GetDisableDSS().Get(), 0);
		RenderPostProcessing();
		m_pDeviceContext->OMSetDepthStencilState(m_pPipeLine->GetLessEqualDSS().Get(), 0);

		RenderPointLight();
	}

	/// UI ���� ///
	{
		// 3D�� �ƴ� 2D�� �������ϱ� ���� ���� -> Orthographic
		m_TransformVP.m_Projection = m_Orthographic;
		RenderUIMeshTexture_Instance();
		RenderUIInstance();													// TODO : ���̹���?���� �ƹ�ư ���� ����� �ٲٰ� �ʱ�ȭ�ϰ� �ϵ��� ����  //  UI ����
		m_textRenderer->DrawTextInQueue();									// Text ����
		RenderDebugDraw();													// ����� ����
#ifdef _DEBUG
		ImguiRender();														// ImGui ����
#endif
	}

	/// Ŭ���� ���� ///
	{
		m_DebugSkeletalMeshModels.clear();
		m_DebugStaticMeshModels.clear();
		m_StaticMeshInstance.clear();				// Static Mesh Instance Ŭ����
		m_SkeletalMeshInstance.clear();				// Skeletal Mesh Instance Ŭ����
		m_StaticMeshInstanceOpacity.clear();		// Static Mesh Opacity Instance Ŭ����
		m_SkeletalMeshInstanceOpacity.clear();		// Skeletal Mesh Opacity Instance Ŭ����
		m_PointLightInstance.clear();				// Point Light Instance Ŭ����
		m_ShadowStaticMeshInstance.clear();			// Shadow Static Mesh Instance Ŭ����
		m_ShadowSkeletalMeshInstance.clear();		// Shadow Skeletal Mesh Instance Ŭ����
		m_OutLineSkeletalMeshInstance.clear();		// OutLine Skeletal Mesh Instance Ŭ����
		m_OutLineStaticMeshInstance.clear();		// OutLine Static Mesh Instance Ŭ����
		m_ParticleList.clear();						// Particle List Ŭ����
		m_UIInstanceList.clear();					// UI Instance Ŭ����
		m_UISkeletalMesh_Instance.clear();		// UI Skeletal Mesh Instance Ŭ����
		m_UISkeletalMesh_InstanceOpacity.clear();	// UI Skeletal Mesh Instance Opacity Ŭ����
		m_DebugPointList.clear();					// Debug Point List Ŭ����
		m_DebugLineList.clear();					// Debug Line List Ŭ����
	}

	// ���� : ���� ����
	m_pGraphics->RenderEnd();
}


/// Opaque Static Mesh Instance ���� ///
void D3DRenderManager::RenderStaticMeshInstance(eShaderType shadertype)
{
	// ���̴� (�׸��� ����) ����
	shared_ptr<Shader> staticshader;
	Material* pPrevMaterial = nullptr;

	if (shadertype == eShaderType::STATIC_SHADER)
	{
		staticshader = ResourceManager::GetInstance()->CreateShaderResource("StaticMeshShader", shadertype);
		staticshader->PipeLineSetting();


		// ���� : ���� ���͸���� ���� ���͸����� ���� ���͸����� �ٲ� ��쿡 ���͸��� ����
		for (const auto& meshInstance : m_StaticMeshInstance)
		{
			if (pPrevMaterial != meshInstance->m_pMaterial)
			{
				ApplyMaterial(meshInstance->m_pMaterial);	// ���͸��� ����
				pPrevMaterial = meshInstance->m_pMaterial;
			}

			// ������Ʈ ��ġ ��� ���� ������Ʈ
			m_TransformW.mWorld = meshInstance->m_pNodeWorldTransform->Transpose();
			m_pDeviceContext->UpdateSubresource(m_pTransformW_Buffer.Get(), 0, nullptr, &m_TransformW, 0, 0);

			// ����ƽ �޽� ����
			meshInstance->Render(m_pDeviceContext.Get());
		}
	}
	else if (shadertype == eShaderType::STATIC_SHADOW_SHADER)			// �׸���
	{
		staticshader = ResourceManager::GetInstance()->CreateShaderResource("ShadowStaticMeshShader", shadertype);
		staticshader->PipeLineSetting();

		// ���� : ���� ���͸���� ���� ���͸����� ���� ���͸����� �ٲ� ��쿡 ���͸��� ����
		for (const auto& meshInstance : m_ShadowStaticMeshInstance)
		{
			// ������Ʈ ��ġ ��� ���� ������Ʈ
			m_TransformW.mWorld = meshInstance->m_pNodeWorldTransform->Transpose();
			m_pDeviceContext->UpdateSubresource(m_pTransformW_Buffer.Get(), 0, nullptr, &m_TransformW, 0, 0);

			// ����ƽ �޽� ����
			meshInstance->Render(m_pDeviceContext.Get());
		}
	}
	else if (shadertype == eShaderType::STATIC_OUTLINE_SHADER)			// �ƿ����� �� ���� ����
	{
		staticshader = ResourceManager::GetInstance()->CreateShaderResource("OutLineStaticMeshShader", shadertype);
		staticshader->PipeLineSetting();

		// ���� : ���� ���͸���� ���� ���͸����� ���� ���͸����� �ٲ� ��쿡 ���͸��� ����
		for (const auto& meshInstance : m_OutLineStaticMeshInstance)
		{
			// ������Ʈ ��ġ ��� ���� ������Ʈ
			m_TransformW.mWorld = meshInstance->m_pNodeWorldTransform->Transpose();
			m_pDeviceContext->UpdateSubresource(m_pTransformW_Buffer.Get(), 0, nullptr, &m_TransformW, 0, 0);

			// ����ƽ �޽� ����
			meshInstance->Render(m_pDeviceContext.Get());
		}
	}
	else if (shadertype == eShaderType::STATIC_OVER_OUTLINE_SHADER)		// �ƿ����� �� ���� ����
	{
		staticshader = ResourceManager::GetInstance()->CreateShaderResource("OverOutLineStaticMeshShader", shadertype);
		staticshader->PipeLineSetting();

		// ���� : ���� ���͸���� ���� ���͸����� ���� ���͸����� �ٲ� ��쿡 ���͸��� ����
		for (const auto& meshInstance : m_OutLineStaticMeshInstance)
		{
			// ������Ʈ ��ġ ��� ���� ������Ʈ
			m_TransformW.mWorld = meshInstance->m_pNodeWorldTransform->Transpose();
			m_pDeviceContext->UpdateSubresource(m_pTransformW_Buffer.Get(), 0, nullptr, &m_TransformW, 0, 0);

			// ����ƽ �޽� ����
			meshInstance->Render(m_pDeviceContext.Get());
		}
	}
}


/// Opaque Skeletal Mesh Instance ���� ///
void D3DRenderManager::RenderSkeletalMeshInstance(eShaderType shadertype)
{
	// ���̴� (�׸��� ����) ����
	shared_ptr<Shader> skeletalshader;
	Material* pPrevMaterial = nullptr;

	if (shadertype == eShaderType::SKELETAL_SHADER)
	{
		skeletalshader = ResourceManager::GetInstance()->CreateShaderResource("SkeletalMeshShader", shadertype);
		skeletalshader->PipeLineSetting();

		// ���� ���͸���� ���� ���͸����� ���� ���͸����� �ٲ� ��쿡 ���͸��� ����
		for (const auto& meshInstance : m_SkeletalMeshInstance)
		{
			if (pPrevMaterial != meshInstance->m_pMaterial)
			{
				ApplyMaterial(meshInstance->m_pMaterial);	// ���͸��� ����
				pPrevMaterial = meshInstance->m_pMaterial;
			}

			// ���̷��� �� �迭 ��� ���� ������Ʈ
			CB_MatrixPalette CB_MatPalatte;
			meshInstance->UpdateMatrixPallete(CB_MatPalatte.Array);
			m_pDeviceContext->UpdateSubresource(m_pMatPalette.Get(), 0, nullptr, &CB_MatPalatte, 0, 0);

			// ���̷�Ż �޽� ������
			meshInstance->Render(m_pDeviceContext.Get());
		}
	}
	else if (shadertype == eShaderType::SKELETAL_SHADOW_SHADER)				// �׸���
	{
		skeletalshader = ResourceManager::GetInstance()->CreateShaderResource("ShadowSkeletalMeshShader", shadertype);
		skeletalshader->PipeLineSetting();

		for (const auto& meshInstance : m_ShadowSkeletalMeshInstance)
		{
			// ���̷�Ż �� �迭 ��� ���� ������Ʈ
			CB_MatrixPalette CB_MatPalatte;
			meshInstance->UpdateMatrixPallete(CB_MatPalatte.Array);
			m_pDeviceContext->UpdateSubresource(m_pMatPalette.Get(), 0, nullptr, &CB_MatPalatte, 0, 0);

			// ���̷�Ż �޽� ������
			meshInstance->Render(m_pDeviceContext.Get());
		}
	}
	else if (shadertype == eShaderType::SKELETAL_OUTLINE_SHADER)			// �ܰ��� �� ���� ����
	{
		skeletalshader = ResourceManager::GetInstance()->CreateShaderResource("OutLineSkeletalMeshShader", shadertype);
		skeletalshader->PipeLineSetting();

		for (const auto& meshInstance : m_OutLineSkeletalMeshInstance)
		{
			// ���̷�Ż �� �迭 ��� ���� ������Ʈ
			CB_MatrixPalette CB_MatPalatte;
			meshInstance->UpdateMatrixPallete(CB_MatPalatte.Array);
			m_pDeviceContext->UpdateSubresource(m_pMatPalette.Get(), 0, nullptr, &CB_MatPalatte, 0, 0);

			// ���̷�Ż �޽� ������
			meshInstance->Render(m_pDeviceContext.Get());
		}
	}
	else if (shadertype == eShaderType::SKELETAL_OVER_OUTLINE_SHADER)		// �ܰ��� �� ���� ����
	{
		skeletalshader = ResourceManager::GetInstance()->CreateShaderResource("OutLineOverSkeletalMeshShader", shadertype);
		skeletalshader->PipeLineSetting();

		for (const auto& meshInstance : m_OutLineSkeletalMeshInstance)
		{
			// ���̷�Ż �� �迭 ��� ���� ������Ʈ
			CB_MatrixPalette CB_MatPalatte;
			meshInstance->UpdateMatrixPallete(CB_MatPalatte.Array);
			m_pDeviceContext->UpdateSubresource(m_pMatPalette.Get(), 0, nullptr, &CB_MatPalatte, 0, 0);

			// ���̷�Ż �޽� ������
			meshInstance->Render(m_pDeviceContext.Get());
		}
	}
}


/// Translucence Staic Mesh Instance ���� ///
void D3DRenderManager::RenderStaticMeshInstanceOpacity(eShaderType shadertype)
{
	// ���̴� (�׸��� ����) ����
	shared_ptr<Shader> staticshader;
	Material* pPrevMaterial = nullptr;
	if (shadertype == eShaderType::STATIC_SHADER)
	{
		staticshader = ResourceManager::GetInstance()->CreateShaderResource("StaticMeshShader", shadertype);
		staticshader->PipeLineSetting();

		// ���� ���͸���� ���� ���͸����� ���� ���͸����� �ٲ� ��쿡 ���͸��� ����
		for (const auto& meshInstance : m_StaticMeshInstanceOpacity)
		{
			if (pPrevMaterial != meshInstance->m_pMaterial)
			{
				ApplyMaterial(meshInstance->m_pMaterial);	// ���͸��� ����
				pPrevMaterial = meshInstance->m_pMaterial;
			}

			// �޽� ���� Ʈ������ ��� ���� ������Ʈ
			m_TransformW.mWorld = meshInstance->m_pNodeWorldTransform->Transpose();
			m_pGraphics->GetDeviceContext()->UpdateSubresource(m_pTransformW_Buffer.Get(), 0, nullptr, &m_TransformW, 0, 0);

			// �޽� ����
			meshInstance->Render(m_pDeviceContext.Get());
		}
	}
	else if (shadertype == eShaderType::STATIC_OUTLINE_SHADER)
	{
		staticshader = ResourceManager::GetInstance()->CreateShaderResource("OutLineStaticMeshShader", shadertype);
		staticshader->PipeLineSetting();

		// ���� ���͸���� ���� ���͸����� ���� ���͸����� �ٲ� ��쿡 ���͸��� ����
		for (const auto& meshInstance : m_OutLineStaticMeshInstance)
		{
			// �޽� ���� Ʈ������ ��� ���� ������Ʈ
			m_TransformW.mWorld = meshInstance->m_pNodeWorldTransform->Transpose();
			m_pGraphics->GetDeviceContext()->UpdateSubresource(m_pTransformW_Buffer.Get(), 0, nullptr, &m_TransformW, 0, 0);

			// �޽� ����
			meshInstance->Render(m_pDeviceContext.Get());
		}
	}
	else if (shadertype == eShaderType::STATIC_OVER_OUTLINE_SHADER)
	{
		staticshader = ResourceManager::GetInstance()->CreateShaderResource("OverOutLineStaticMeshShader", shadertype);
		staticshader->PipeLineSetting();

		// ���� ���͸���� ���� ���͸����� ���� ���͸����� �ٲ� ��쿡 ���͸��� ����
		for (const auto& meshInstance : m_OutLineStaticMeshInstance)
		{
			// �޽� ���� Ʈ������ ��� ���� ������Ʈ
			m_TransformW.mWorld = meshInstance->m_pNodeWorldTransform->Transpose();
			m_pGraphics->GetDeviceContext()->UpdateSubresource(m_pTransformW_Buffer.Get(), 0, nullptr, &m_TransformW, 0, 0);

			// �޽� ����
			meshInstance->Render(m_pDeviceContext.Get());
		}
	}
}


/// Translucence Skeletal Mesh Instance ���� ///
void D3DRenderManager::RenderSkeletalMeshInstanceOpacity(eShaderType shadertype)
{
	// ���̴� (�׸��� ����) ����
	shared_ptr<Shader> skeletalshader;
	Material* pPrevMaterial = nullptr;
	if (shadertype == eShaderType::SKELETAL_SHADER)
	{
		skeletalshader = ResourceManager::GetInstance()->CreateShaderResource("SkeletalMeshShader", shadertype);
		skeletalshader->PipeLineSetting();

		// ���� ���͸���� ���� ���͸����� ���� ���͸����� �ٲ� ��쿡 ���͸��� ����
		for (const auto& meshInstance : m_SkeletalMeshInstanceOpacity)
		{
			if (pPrevMaterial != meshInstance->m_pMaterial)
			{
				ApplyMaterial(meshInstance->m_pMaterial);	// ���͸��� ����
				pPrevMaterial = meshInstance->m_pMaterial;
			}

			// �� �ķ�Ʈ ��� ���� ������Ʈ
			CB_MatrixPalette CB_MatPalatte;
			meshInstance->UpdateMatrixPallete(CB_MatPalatte.Array);
			m_pDeviceContext->UpdateSubresource(m_pMatPalette.Get(), 0, nullptr, &CB_MatPalatte, 0, 0);

			// �޽� ����
			meshInstance->Render(m_pDeviceContext.Get());
		}
	}
	else if (shadertype == eShaderType::SKELETAL_OUTLINE_SHADER)
	{
		skeletalshader = ResourceManager::GetInstance()->CreateShaderResource("OutLineSkeletalMeshShader", shadertype);
		skeletalshader->PipeLineSetting();

		// ���� ���͸���� ���� ���͸����� ���� ���͸����� �ٲ� ��쿡 ���͸��� ����
		for (const auto& meshInstance : m_OutLineSkeletalMeshInstance)
		{
			// �� �ķ�Ʈ ��� ���� ������Ʈ
			CB_MatrixPalette CB_MatPalatte;
			meshInstance->UpdateMatrixPallete(CB_MatPalatte.Array);
			m_pDeviceContext->UpdateSubresource(m_pMatPalette.Get(), 0, nullptr, &CB_MatPalatte, 0, 0);

			// �޽� ����
			meshInstance->Render(m_pDeviceContext.Get());
		}
	}
	else if (shadertype == eShaderType::SKELETAL_OVER_OUTLINE_SHADER)
	{
		skeletalshader = ResourceManager::GetInstance()->CreateShaderResource("OutLineOverSkeletalMeshShader", shadertype);
		skeletalshader->PipeLineSetting();

		// ���� ���͸���� ���� ���͸����� ���� ���͸����� �ٲ� ��쿡 ���͸��� ����
		for (const auto& meshInstance : m_OutLineSkeletalMeshInstance)
		{
			// �� �ķ�Ʈ ��� ���� ������Ʈ
			CB_MatrixPalette CB_MatPalatte;
			meshInstance->UpdateMatrixPallete(CB_MatPalatte.Array);
			m_pDeviceContext->UpdateSubresource(m_pMatPalette.Get(), 0, nullptr, &CB_MatPalatte, 0, 0);

			// �޽� ����
			meshInstance->Render(m_pDeviceContext.Get());
		}
	}
}


///  3D UI SkeletalMesh Instance ���� ///
void D3DRenderManager::RenderUIMesh_Instacne(int playerNumber)
{
	shared_ptr<Shader> shader = ResourceManager::GetInstance()->CreateShaderResource("UIMeshShader", eShaderType::UI_MESH_SHADER);
	shader->PipeLineSetting();

	m_pDeviceContext->OMSetDepthStencilState(m_pPipeLine->GetLessEqualDSS().Get(), 0);

	Material* pPrevMaterial = nullptr;
	// ������ UI �޽�
	for (const auto& uiInstance : m_UISkeletalMesh_Instance)
	{
		if (uiInstance.second == playerNumber)
		{
			// ���� ���͸���� ���� ���͸����� ���� ���͸����� �ٲ� ��쿡 ���͸��� ����
			if (pPrevMaterial != uiInstance.first->m_pMaterial)
			{
				ApplyMaterial(uiInstance.first->m_pMaterial);	// ���͸��� ����
				pPrevMaterial = uiInstance.first->m_pMaterial;
			}

			// �� �ķ�Ʈ ��� ���� ������Ʈ
			CB_MatrixPalette CB_MatPalatte;
			uiInstance.first->UpdateMatrixPallete(CB_MatPalatte.Array);
			m_pDeviceContext->UpdateSubresource(m_pMatPalette.Get(), 0, nullptr, &CB_MatPalatte, 0, 0);

			// �޽� ����
			uiInstance.first->Render(m_pDeviceContext.Get());
		}
	}

	//m_pDeviceContext->OMSetDepthStencilState(m_pPipeLine->GetNoDepthWritesDSS().Get(), 0);

	// ������ UI �޽� 
	for (const auto& uiInstance : m_UISkeletalMesh_InstanceOpacity)
	{
		if (uiInstance.second == playerNumber)
		{
			// ���� ���͸���� ���� ���͸����� ���� ���͸����� �ٲ� ��쿡 ���͸��� ����
			if (pPrevMaterial != uiInstance.first->m_pMaterial)
			{
				ApplyMaterial(uiInstance.first->m_pMaterial);	// ���͸��� ����
				pPrevMaterial = uiInstance.first->m_pMaterial;
			}

			// �� �ķ�Ʈ ��� ���� ������Ʈ
			CB_MatrixPalette CB_MatPalatte;
			uiInstance.first->UpdateMatrixPallete(CB_MatPalatte.Array);
			m_pDeviceContext->UpdateSubresource(m_pMatPalette.Get(), 0, nullptr, &CB_MatPalatte, 0, 0);

			// �޽� ����
			uiInstance.first->Render(m_pDeviceContext.Get());
		}
	}

	m_pDeviceContext->OMSetDepthStencilState(m_pPipeLine->GetLessEqualDSS().Get(), 0);
}


/// 2D UI ���� ///
void D3DRenderManager::RenderUIInstance()
{
	m_pGraphics->GetDeviceContext()->OMSetBlendState(m_pPipeLine->GetAlphaBlendState().Get(), nullptr, 0xffffffff); // ���ĺ��� ���¼��� , �ٸ��ɼ��� �⺻��

	// ���������ο� ���̷�Ż �޽� ���̴��� InputLayout, VS_Shader, PS_Shader ����
	shared_ptr<Shader> shader = ResourceManager::GetInstance()->CreateShaderResource("UIShader", eShaderType::UI_SHADER);
	shader->PipeLineSetting();

	m_pDeviceContext->UpdateSubresource(m_pTransformVP_Buffer.Get(), 0, nullptr, &m_TransformVP, 0, 0);
	for (const auto& uiInstance : m_UIInstanceList)
	{
		// ������Ʈ ��ġ ��� ���� ������Ʈ
		// UI �ִϸ��̼� ��� ���� ������Ʈ
		// ���̵� �� �ƿ� ��� ���� ������Ʈ
		CB_UIAnimationKeyframe CB_animationKeyFrame;
		CB_FadeInOut CB_fadeInOut;

		uiInstance->UIAnimationPlay(CB_animationKeyFrame);
		uiInstance->UIFadeInOut(CB_fadeInOut);
		m_TransformW.mWorld = uiInstance->GetWorldMatrix()->Transpose();

		m_pDeviceContext->UpdateSubresource(m_pTransformW_Buffer.Get(), 0, nullptr, &m_TransformW, 0, 0);
		m_pDeviceContext->UpdateSubresource(m_pUIAnimation_Buffer.Get(), 0, nullptr, &CB_animationKeyFrame, 0, 0);
		m_pDeviceContext->UpdateSubresource(m_pFadeInOut_Buffer.Get(), 0, nullptr, &CB_fadeInOut, 0, 0);

		// UI �ν��Ͻ� ����
		uiInstance->Render(m_pDeviceContext.Get());
	}

	m_pGraphics->GetDeviceContext()->OMSetBlendState(nullptr, nullptr, 0xffffffff);	// �������� , �ٸ��ɼ��� �⺻��
}


/// UI Mesh Texture ���� ///
void D3DRenderManager::RenderUIMeshTexture_Instance()
{
	shared_ptr<Shader> shader = ResourceManager::GetInstance()->CreateShaderResource("UIMeshTextureShader", eShaderType::UI_MESH_TEXTURE_SHADER);
	shader->PipeLineSetting();

	CB_UIMesh CB_UIMesh;
	CB_UIMesh.m_PlayerNumber = 1;

	// 1P UI Mesh
	if (m_bIsUIMesh_1_OutLine)
	{
		CB_UIMesh.m_OutLine = 1;
		m_pDeviceContext->UpdateSubresource(m_pUIMeshBuffer.Get(), 0, nullptr, &CB_UIMesh, 0, 0);
		m_UIMeshOutLine_1_RenderInstance->Render(m_pDeviceContext.Get());
	}
	CB_UIMesh.m_OutLine = 0;
	m_pDeviceContext->UpdateSubresource(m_pUIMeshBuffer.Get(), 0, nullptr, &CB_UIMesh, 0, 0);
	m_UIMesh_1_RenderInstance->Render(m_pDeviceContext.Get());

	// 2P UI Mesh
	CB_UIMesh.m_PlayerNumber = 2;
	if (m_bIsUIMesh_2_OutLine)
	{
		CB_UIMesh.m_OutLine = 1;
		m_pDeviceContext->UpdateSubresource(m_pUIMeshBuffer.Get(), 0, nullptr, &CB_UIMesh, 0, 0);
		m_UIMeshOutLine_2_RenderInstance->Render(m_pDeviceContext.Get());
	}
	CB_UIMesh.m_OutLine = 0;
	m_pDeviceContext->UpdateSubresource(m_pUIMeshBuffer.Get(), 0, nullptr, &CB_UIMesh, 0, 0);
	m_UIMesh_2_RenderInstance->Render(m_pDeviceContext.Get());
}

 
/// ȯ��� ���� ///
void D3DRenderManager::RenderEnvironment()
{
	if (m_bIsMapUse.UseCubeMap == false) return;
	if (m_pEnvironmentModel == nullptr)	return;

	m_pDeviceContext->RSSetState(m_pPipeLine->GetRasterizerStateCCW().Get()); // ������ ������ ���� CCW

	// ���� : ���������ο� ���̷�Ż �޽� ���̴��� InputLayout, VS_Shader, PS_Shader ����
	shared_ptr<Shader> shader = ResourceManager::GetInstance()->CreateShaderResource("EnvironmentShader", eShaderType::ENVIRONMENT_SHADER);
	shader->PipeLineSetting();

	// ���� ���Ľ� ���´� �̰͸� ��������.
	m_pDeviceContext->VSSetConstantBuffers(0, 1, m_pTransformW_Buffer.GetAddressOf()); //debugdraw���� ������Ѽ� �����Ѵ�.

	m_TransformW.mWorld = m_pEnvironmentModel->m_worldMatrix->Transpose();
	m_pDeviceContext->UpdateSubresource(m_pTransformW_Buffer.Get(), 0, nullptr, &m_TransformW, 0, 0);
	m_pEnvironmentModel->m_MeshInstance.Render(m_pDeviceContext.Get());

	m_pDeviceContext->RSSetState(m_pPipeLine->GetRasterizerStateCW().Get()); // ������ ������ ���� CW
}


/// ��ƼŬ ���� ///
void D3DRenderManager::RenderParticleEffect()
{
	//m_pDeviceContext->RSSetState(0);

	for (auto& particle : m_ParticleList)
	{
		// ��ƼŬ ��� ���� ������Ʈ
		CB_PerFrame perFrame;
		CB_ParticleData particleData;

		perFrame.ViewProjection = m_View * m_Projection;
		perFrame.ViewProjection = perFrame.ViewProjection.Transpose();
		particle->UpdatePerFrame(perFrame);
		particle->UpdatePerticleData(particleData);

		m_pDeviceContext->UpdateSubresource(m_pParticleBuffer.Get(), 0, nullptr, &perFrame, 0, 0);
		m_pDeviceContext->VSSetConstantBuffers(8, 1, m_pParticleBuffer.GetAddressOf());
		m_pDeviceContext->PSSetConstantBuffers(8, 1, m_pParticleBuffer.GetAddressOf());
		m_pDeviceContext->GSSetConstantBuffers(8, 1, m_pParticleBuffer.GetAddressOf());
		m_pDeviceContext->UpdateSubresource(m_pParticleDataBuffer.Get(), 0, nullptr, &particleData, 0, 0);
		m_pDeviceContext->VSSetConstantBuffers(9, 1, m_pParticleDataBuffer.GetAddressOf());
		m_pDeviceContext->PSSetConstantBuffers(9, 1, m_pParticleDataBuffer.GetAddressOf());
		m_pDeviceContext->GSSetConstantBuffers(9, 1, m_pParticleDataBuffer.GetAddressOf());

		// ��ƼŬ �ý��� ����
		particle->Render(m_pDeviceContext, m_pPipeLine);
	}

	//m_pDeviceContext->RSSetState(m_pPipeLine->GetRasterizerStateCW().Get()); // ������ ������ ���� CW
}

void D3DRenderManager::RenderPointLight()
{
	CB_PointLight cbPointLight;

	int i = 0;	

	for (auto& pointLightData : m_PointLightInstance)
	{	
		assert(i < pointlightMaxCount);
		if (i >= pointlightMaxCount) break;

		cbPointLight.pointLights[i].lightPosition.x = pointLightData->m_PointLightPos.x;
		cbPointLight.pointLights[i].lightPosition.y = pointLightData->m_PointLightPos.y;
		cbPointLight.pointLights[i].lightPosition.z = pointLightData->m_PointLightPos.z;
		cbPointLight.pointLights[i].lightPosition.w = 1;

		cbPointLight.pointLights[i].lightColor.x = pointLightData->m_PointLightColor.x;
		cbPointLight.pointLights[i].lightColor.y = pointLightData->m_PointLightColor.y;
		cbPointLight.pointLights[i].lightColor.z = pointLightData->m_PointLightColor.z;

		cbPointLight.pointLights[i].lightRange = pointLightData->m_PointLightRange;
		cbPointLight.pointLights[i].linearTerm = pointLightData->m_PointLightLinearTerm;
		cbPointLight.pointLights[i].quadraticTerm =  pointLightData->m_PointLightQuadraTicTerm;
		cbPointLight.pointLights[i].lightIntensity = pointLightData->m_PointLightIntencity;

		i++;
	}

	m_pDeviceContext->UpdateSubresource(m_pPointLight_Buffer.Get(), 0, nullptr, &cbPointLight, 0, 0);
}

// ����� ��ο�
void D3DRenderManager::RenderDebugDraw()
{
	DebugDraw::g_BatchEffect->Apply(m_pGraphics->GetDeviceContext().Get());
	DebugDraw::g_BatchEffect->SetView(m_View);
	DebugDraw::g_BatchEffect->SetProjection(m_Projection);

	m_pGraphics->GetDeviceContext()->IASetInputLayout(DebugDraw::g_pBatchInputLayout.Get());
	m_pDeviceContext->OMSetDepthStencilState(m_pPipeLine->GetDisableDSS().Get(), 0);

	DebugDraw::g_Batch->Begin();

	// Culling �ڽ��� �������� �ڽ���
	if (m_IsDrawDebugCulling)
	{
		DebugDraw::Draw(DebugDraw::g_Batch.get(), m_FrustumCamera, Colors::Yellow); // BoundingBox
		//DebugDraw::DrawGrid(DebugDraw::g_Batch.get(), Vector3(10000.f, 0.f, 0.f), Vector3(0.f, 0.f, 10000.f), Vector3(0.f, 0.f, 0.f), 200, 200, Colors::Gray); // BoundingBox

		//for (auto& Point : m_DebugPointList)
		//{
		//	DebugDraw::DrawRay(DebugDraw::g_Batch.get(), Point.m_Position, { 0, 0, 0 }, Point.m_Color);
		//}

		//for (auto& line : m_DebugLineList)
		//{
		//	DebugDraw::DrawRay(DebugDraw::g_Batch.get(), line.m_StartLine, line.m_EndLine - line.m_StartLine, false, line.m_StartColor);
		//}

		for (auto& model : m_DebugSkeletalMeshModels)
		{
			DirectX::BoundingBox boundingBox = model->m_BoundingBox;
			boundingBox.Center.x += model->m_worldMatrix->Translation().x;
			boundingBox.Center.y += model->m_worldMatrix->Translation().y;
			boundingBox.Center.z += model->m_worldMatrix->Translation().z;

			//DebugDraw::Draw(DebugDraw::g_Batch.get(), boundingBox,
				//model->m_IsCulled ? Colors::Red : Colors::Blue); // BoundingBox
		}
		for (auto& model : m_DebugStaticMeshModels)
		{
			DirectX::BoundingBox boundingBox;

			for (auto& instance : model->m_MeshInstances)
			{
				boundingBox = *instance.m_pBoundingBox;
				boundingBox.Center.x += instance.m_pNodeWorldTransform->Translation().x;
				boundingBox.Center.y += instance.m_pNodeWorldTransform->Translation().y;
				boundingBox.Center.z += instance.m_pNodeWorldTransform->Translation().z;

				if (m_FrustumCamera.Intersects(boundingBox))
					DebugDraw::Draw(DebugDraw::g_Batch.get(), boundingBox, Colors::Red); // BoundingBox
			}

		}

	}

	DebugDraw::g_Batch->End();
	m_pDeviceContext->OMSetDepthStencilState(m_pPipeLine->GetLessEqualDSS().Get(), 0);
}


/// �޽� �ν��Ͻ� ���͸��󺰷� ���� ///
void D3DRenderManager::SortMeshInstance()
{
	// ���������ο� �����ϴ� ���͸����� �ؽ��� ������ �ּ�ȭ �ϱ����� ���͸��� ���� �����Ѵ�.
	m_SkeletalMeshInstance.sort([](const SkeletalMeshInstance* lhs, const SkeletalMeshInstance* rhs)
		{
			return lhs->m_pMaterial < rhs->m_pMaterial;
		});

	// ���������ο� �����ϴ� ���͸����� �ؽ��� ������ �ּ�ȭ �ϱ����� ���͸��� ���� �����Ѵ�.
	m_StaticMeshInstance.sort([](const StaticMeshInstance* lhs, const StaticMeshInstance* rhs)
		{
			return lhs->m_pMaterial < rhs->m_pMaterial;
		});

	// ���������ο� �����ϴ� ���͸����� �ؽ��� ������ �ּ�ȭ �ϱ����� ���͸��� ���� �����Ѵ�.
	m_SkeletalMeshInstanceOpacity.sort([](const SkeletalMeshInstance* lhs, const SkeletalMeshInstance* rhs)
		{
			return lhs->m_pMaterial < rhs->m_pMaterial;
		});

	// ���������ο� �����ϴ� ���͸����� �ؽ��� ������ �ּ�ȭ �ϱ����� ���͸��� ���� �����Ѵ�.
	m_StaticMeshInstanceOpacity.sort([](const StaticMeshInstance* lhs, const StaticMeshInstance* rhs)
		{
			return lhs->m_pMaterial < rhs->m_pMaterial;
		});

	// ���������ο� �����ϴ� ���͸����� �ؽ��� ������ �ּ�ȭ �ϱ����� ���͸��� ���� �����Ѵ�.
	m_UISkeletalMesh_Instance.sort([](const std::pair<SkeletalMeshInstance*, int> lhs, const std::pair<SkeletalMeshInstance*, int> rhs)
		{
			return lhs.first->m_pMaterial < rhs.first->m_pMaterial;
		});

	// ���������ο� �����ϴ� ���͸����� �ؽ��� ������ �ּ�ȭ �ϱ����� ���͸��� ���� �����Ѵ�.
	m_UISkeletalMesh_InstanceOpacity.sort([](const std::pair<SkeletalMeshInstance*, int> lhs, const std::pair<SkeletalMeshInstance*, int> rhs)
		{
			return lhs.first->m_pMaterial < rhs.first->m_pMaterial;
		});
}


/// ȯ��� ���� ///
void D3DRenderManager::SetEnvironment()
{
	m_pDeviceContext->PSSetShaderResources(7, 1, m_pEnvironmentModel->m_EnvironmentTextureResource->m_pTextureSRV.GetAddressOf());
	m_pDeviceContext->PSSetShaderResources(8, 1, m_pEnvironmentModel->m_IBLDiffuseTextureResource->m_pTextureSRV.GetAddressOf());
	m_pDeviceContext->PSSetShaderResources(9, 1, m_pEnvironmentModel->m_IBLSpecularTextureResource->m_pTextureSRV.GetAddressOf());
	m_pDeviceContext->PSSetShaderResources(10, 1, m_pEnvironmentModel->m_IBLBRDFTextureResource->m_pTextureSRV.GetAddressOf());
}


/// ���� �޸� �б� ///
void D3DRenderManager::GetVideoMemoryInfo(std::string& out)
{
	DXGI_QUERY_VIDEO_MEMORY_INFO videoMemoryInfo;
	m_pDXGIAdapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &videoMemoryInfo);

	out = std::to_string(videoMemoryInfo.CurrentUsage / 1024 / 1024) + " MB" + " / " + std::to_string(videoMemoryInfo.Budget / 1024 / 1024) + " MB";
}


/// �ý��� �޸� �б� ///
void D3DRenderManager::GetSystemMemoryInfo(std::string& out)
{
	HANDLE hProcess = GetCurrentProcess();
	PROCESS_MEMORY_COUNTERS_EX pmc;
	pmc.cb = sizeof(PROCESS_MEMORY_COUNTERS_EX);
	GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
	out = std::to_string((pmc.PagefileUsage) / 1024 / 1024) + " MB";
}

/// Draw Text Request ///
void D3DRenderManager::DrawTextRequest(std::wstring_view text, FLOAT x, FLOAT y, D2D1::ColorF color, std::wstring_view fontFamily, FLOAT fontSize, DWRITE_FONT_WEIGHT fontWeight, DWRITE_FONT_STYLE fontStyle, DWRITE_FONT_STRETCH fontStretch)
{
	m_textRenderer->DrawTextRequest(text, x, y, color, fontFamily, fontSize, m_ClientWidth, m_ClientHeight, fontWeight, fontStyle, fontStretch);
}


/// World Resource Clear ///
void D3DRenderManager::ClearWorldResource()
{
	for (auto& model : m_SkeletalMeshModels)
	{
		model.reset();
	}
	for (auto& model : m_StaticMeshModels)
	{
		model.reset();
	}
	for (auto& componentPtr : m_DebugStaticMeshModels)
	{
		componentPtr.reset();
	}
	for (auto& componentPtr : m_DebugSkeletalMeshModels)
	{
		componentPtr.reset();
	}
	for (auto& componentPtr : m_UISkeletalMeshModels)
	{
		componentPtr.first.reset();
	}

	m_SkeletalMeshModels.clear();				// Skeletal Mesh Model Ŭ����
	m_StaticMeshModels.clear();					// Static Mesh Model Ŭ����
	m_DebugStaticMeshModels.clear();			// Debug Static Mesh Model Ŭ����
	m_DebugSkeletalMeshModels.clear();			// Debug Skeletal Mesh Model Ŭ����
	m_UISkeletalMeshModels.clear();				// UI Skeletal Mesh Model Ŭ����

	m_StaticMeshInstance.clear();				// Static Mesh Instance Ŭ����
	m_SkeletalMeshInstance.clear();				// Skeletal Mesh Instance Ŭ����
	m_StaticMeshInstanceOpacity.clear();		// Static Mesh Opacity Instance Ŭ����
	m_SkeletalMeshInstanceOpacity.clear();		// Skeletal Mesh Opacity Instance Ŭ����
	m_ShadowStaticMeshInstance.clear();			// Shadow Static Mesh Instance Ŭ����
	m_ShadowSkeletalMeshInstance.clear();		// Shadow Skeletal Mesh Instance Ŭ����
	m_ParticleList.clear();						// Particle List Ŭ����
	m_UIInstanceList.clear();					// UI Instance Ŭ����
	m_UISkeletalMesh_Instance.clear();		// UI Skeletal Mesh Instance Ŭ����
	m_UISkeletalMesh_InstanceOpacity.clear();	// UI Skeletal Mesh Instance Opacity Ŭ����
	m_DebugPointList.clear();					// Debug Point List Ŭ����
	m_DebugLineList.clear();					// Debug Line List Ŭ����
}


/// Material ���� ///
void D3DRenderManager::ApplyMaterial(Material* pMaterial)
{
	ID3D11ShaderResourceView* pNullSRV[7] = {
		pMaterial->m_pDiffuseRV != nullptr ? pMaterial->m_pDiffuseRV->m_pTextureSRV.Get() : nullptr,
		pMaterial->m_pNormalRV != nullptr ? pMaterial->m_pNormalRV->m_pTextureSRV.Get() : nullptr,
		pMaterial->m_pSpecularRV != nullptr ? pMaterial->m_pSpecularRV->m_pTextureSRV.Get() : nullptr,
		pMaterial->m_pEmissiveRV != nullptr ? pMaterial->m_pEmissiveRV->m_pTextureSRV.Get() : nullptr,
		pMaterial->m_pOpacityRV != nullptr ? pMaterial->m_pOpacityRV->m_pTextureSRV.Get() : nullptr,
		pMaterial->m_pMetalnessRV != nullptr ? pMaterial->m_pMetalnessRV->m_pTextureSRV.Get() : nullptr,
		pMaterial->m_pRoughnessRV != nullptr ? pMaterial->m_pRoughnessRV->m_pTextureSRV.Get() : nullptr ,
	};
	m_pGraphics->GetDeviceContext()->PSSetShaderResources(0, 7, pNullSRV); // �ѹ��� 7���� �ؽ�ó�� �����Ѵ�.


	if (m_isUseEdit == false)
	{
		m_bIsMapUse.UseDiffuseMap = pMaterial->m_pDiffuseRV != nullptr ? true : false;
		m_bIsMapUse.UseNormalMap = pMaterial->m_pNormalRV != nullptr ? true : false;
		m_bIsMapUse.UseSpecularMap = pMaterial->m_pSpecularRV != nullptr ? true : false;
		m_bIsMapUse.UseEmissiveMap = pMaterial->m_pEmissiveRV != nullptr ? true : false;
		m_bIsMapUse.UseMetalnessMap = pMaterial->m_pMetalnessRV != nullptr ? true : false;
		m_bIsMapUse.UseRoughnessMap = pMaterial->m_pRoughnessRV != nullptr ? true : false;
	}
	else
	{
		m_bIsMapUse.UseDiffuseMap = pMaterial->m_pDiffuseRV != nullptr ? m_isDiffuse : false;
		m_bIsMapUse.UseNormalMap = pMaterial->m_pNormalRV != nullptr ? m_isNormalMap : false;
		m_bIsMapUse.UseSpecularMap = pMaterial->m_pSpecularRV != nullptr ? m_isEmissive : false;
		m_bIsMapUse.UseEmissiveMap = pMaterial->m_pEmissiveRV != nullptr ? m_isEmissive : false;
		m_bIsMapUse.UseMetalnessMap = pMaterial->m_pMetalnessRV != nullptr ? m_isMetalness : false;
		m_bIsMapUse.UseRoughnessMap = pMaterial->m_pRoughnessRV != nullptr ? m_isRoughness : false;
	}

	{
		m_bIsMapUse.UseOpacityMap = pMaterial->m_pOpacityRV != nullptr ? true : false;
		m_bIsMapUse.UseCubeMap = m_isCubeMap;
		m_bIsMapUse.UseGamma = m_isGamma;
		m_IBL.UseIBL = m_isUseIBL;
		m_IBL.UseLightIBL = m_UseLightIBL;

		m_bIsMapUse.MetalnessValue = m_MetalnessValue;
		m_bIsMapUse.RoughnessValue = m_RoughnessValue;
		m_bIsMapUse.ValueLightIBL = m_LightIBLValue;
		m_bIsMapUse.ValueIBL = m_IBLValue;
	}


	m_pDeviceContext->UpdateSubresource(m_pBoolBuffer.Get(), 0, nullptr, &m_bIsMapUse, 0, 0);

	// ���ĺ��� ���¼��� , �ٸ��ɼ��� �⺻��
	if (m_bIsMapUse.UseOpacityMap && m_pPipeLine->GetAlphaBlendState() != nullptr)
		m_pGraphics->GetDeviceContext()->OMSetBlendState(m_pPipeLine->GetAlphaBlendState().Get(), nullptr, 0xffffffff);
	else
		m_pGraphics->GetDeviceContext()->OMSetBlendState(nullptr, nullptr, 0xffffffff);	// �������� , �ٸ��ɼ��� �⺻��
}


/// Environment Model Setting ///
void D3DRenderManager::SetEnvironmentModel(const std::shared_ptr<EnvironmentModel>& environmentModel)
{
	m_pEnvironmentModel = environmentModel;
	m_IBL.UseIBL = true;

	SetEnvironment();
}

void D3DRenderManager::SetSecondIBL()
{
	m_pLightEnvironment = make_shared<EnvironmentModel>(); 	 
	m_pLightEnvironment->ReadIBLDiffuseTextureFromDDS (L"../Resources/Textures/ThreeColorLightDiffuseHDR.dds");
	m_pLightEnvironment->ReadIBLSpecularTextureFromDDS(L"../Resources/Textures/ThreeColorLightSpecularHDR.dds");
	m_pLightEnvironment->ReadIBLBRDFTextureFromDDS    (L"../Resources/Textures/ThreeColorLightBrdf.dds");

	//m_pDeviceContext->PSSetShaderResources(12, 1, m_pEnvironmentModel->m_EnvironmentTextureResource->m_pTextureSRV.GetAddressOf());
	m_pDeviceContext->PSSetShaderResources(12, 1, m_pLightEnvironment->m_IBLDiffuseTextureResource->m_pTextureSRV.GetAddressOf());
	m_pDeviceContext->PSSetShaderResources(13, 1, m_pLightEnvironment->m_IBLSpecularTextureResource->m_pTextureSRV.GetAddressOf());
	m_pDeviceContext->PSSetShaderResources(14, 1, m_pLightEnvironment->m_IBLBRDFTextureResource->m_pTextureSRV.GetAddressOf());
 }

bool D3DRenderManager::Initialize(HWND hWnd)
{
	//m_pResourceManager = std::make_shared<ResourceManager>();
	m_hWnd = hWnd;

	if (!InitD3D())		return false;
	if (!InitImGUI())	return false;

	m_textRenderer = std::make_unique<TextRenderer>();
	m_textRenderer->Initailize(m_pGraphics->GetD2DRenderTarget().Get(), m_pGraphics->GetWriteFactory().Get());

	m_PostProcessRenderInstance = std::make_shared<UIInstance>();
	m_PostProcessRenderInstance->Create(&m_Test, Vector2(2.f, 2.f));
	m_UIMesh_1_RenderInstance = std::make_shared<UIInstance>();
	m_UIMesh_1_RenderInstance->Create(&m_Test, Vector2(1280.f / m_ClientWidth, 680.f / m_ClientHeight));
	m_UIMesh_2_RenderInstance = std::make_shared<UIInstance>();
	m_UIMesh_2_RenderInstance->Create(&m_Test, Vector2(1280.f / m_ClientWidth, 680.f / m_ClientHeight));
	m_UIMeshOutLine_1_RenderInstance = std::make_shared<UIInstance>();
	m_UIMeshOutLine_1_RenderInstance->Create(&m_Test, Vector2(1280.f / m_ClientWidth * 1.04f, 680.f / m_ClientHeight * 1.04f));
	m_UIMeshOutLine_2_RenderInstance = std::make_shared<UIInstance>();
	m_UIMeshOutLine_2_RenderInstance->Create(&m_Test, Vector2(1280.f / m_ClientWidth * 1.04f, 680.f / m_ClientHeight * 1.04f));

	return true;
}


/// D3DRenderManager InitD3D ///
bool D3DRenderManager::InitD3D()
{
	// �����.
	HRESULT hr = 0;

	// Create DXGI factory
	HR_T(CreateDXGIFactory1(__uuidof(IDXGIFactory4), (void**)m_pDXGIFactory.GetAddressOf()));
	HR_T(m_pDXGIFactory->EnumAdapters(0, reinterpret_cast<IDXGIAdapter**>(m_pDXGIAdapter.GetAddressOf())));

	// Graphics ����
	{
		m_pGraphics = make_shared<Graphics>();
		GraphicsInfo info;
		info.m_ClearColor = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
		info.m_ClientHeight = m_ClientHeight;
		info.m_ClientWidth = m_ClientWidth;
		info.m_hwnd = m_hWnd;

		m_pGraphics->Initalize(info);
		m_pDevice = m_pGraphics->GetDevice();
		m_pDeviceContext = m_pGraphics->GetDeviceContext();
	}
	// PipeLine ����
	{
		m_pPipeLine = make_shared<PipeLine>();
		PipeLineInfo info;
		info.m_pDevice = m_pDevice;
		info.m_pDeviceContext = m_pGraphics->GetDeviceContext();
		m_pPipeLine->Initalize(info);
	}
	// Debug ����
	{
		DebugDraw::Initialize(m_pDevice, m_pGraphics->GetDeviceContext());
	}

	// Render() ���� ���������ο� ���ε��� ��� ���� ����  //  Create the constant buffer
	CreateConstantBuffer();

	// ������ �ʱ�ȭ
	InitScene();

	return true;
}


/// D3DRenderManager InitScene ///
void D3DRenderManager::InitScene()
{
	m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, m_ClientWidth / (FLOAT)m_ClientHeight, 0.1f, 100000.0f);
	m_Orthographic = XMMatrixOrthographicLH((float)m_ClientWidth, (float)m_ClientHeight, m_Near, m_Far);

	m_UICamBasePosition = Vector3(0.f, 100.f, -500.f);
	m_UICamUpPosition = Vector3(0.f, 500.f, -500.f);
	m_UIView = XMMatrixLookAtLH(m_UICamBasePosition, Vector3(0.f, 100.f, 0.f), Vector3(0.f, 1.f, 0.f));
	m_UITransform = Matrix::Identity;

	SetSecondIBL();
}


/// View ��� ���� ///
void D3DRenderManager::SetViewMatrix(Math::Matrix viewMatrix)
{
	m_View = viewMatrix;
}


/// Camera ��ġ ���� ///
void D3DRenderManager::SetCamMatrix(Math::Vector3 camMatrix)
{
	m_CamPosition = camMatrix;
}


// ImGui Init ///
bool D3DRenderManager::InitImGUI()
{
	// ImGui �ʱ�ȭ.
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(m_hWnd);
	ImGui_ImplDX11_Init(m_pDevice.Get(), m_pGraphics->GetDeviceContext().Get());

	return true;
}


/// Constant Buffer Create ///
void D3DRenderManager::CreateConstantBuffer()
{
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;

	bd.ByteWidth = sizeof(CB_IBL);
	HR_T(m_pDevice->CreateBuffer(&bd, nullptr, m_pIBL_Buffer.GetAddressOf()));

	bd.ByteWidth = sizeof(CB_BoolBuffer);
	HR_T(m_pDevice->CreateBuffer(&bd, nullptr, m_pBoolBuffer.GetAddressOf()));

	bd.ByteWidth = sizeof(CB_TransformW);
	HR_T(m_pDevice->CreateBuffer(&bd, nullptr, m_pTransformW_Buffer.GetAddressOf()));

	bd.ByteWidth = sizeof(CB_TransformVP);
	HR_T(m_pDevice->CreateBuffer(&bd, nullptr, m_pTransformVP_Buffer.GetAddressOf()));

	bd.ByteWidth = sizeof(CB_LightDirBuffer);
	HR_T(m_pDevice->CreateBuffer(&bd, nullptr, m_pLightBuffer.GetAddressOf()));

	bd.ByteWidth = sizeof(CB_MatrixPalette);
	HR_T(m_pDevice->CreateBuffer(&bd, nullptr, m_pMatPalette.GetAddressOf()));

	bd.ByteWidth = sizeof(CB_UIAnimationKeyframe);
	HR_T(m_pDevice->CreateBuffer(&bd, nullptr, m_pUIAnimation_Buffer.GetAddressOf()));

	bd.ByteWidth = sizeof(CB_FadeInOut);
	HR_T(m_pDevice->CreateBuffer(&bd, nullptr, m_pFadeInOut_Buffer.GetAddressOf()));

	bd.ByteWidth = sizeof(CB_PointLight);
	HR_T(m_pDevice->CreateBuffer(&bd, nullptr, m_pPointLight_Buffer.GetAddressOf()));

	bd.ByteWidth = sizeof(CB_PerFrame);
	HR_T(m_pDevice->CreateBuffer(&bd, nullptr, m_pParticleBuffer.GetAddressOf()));

	bd.ByteWidth = sizeof(CB_ParticleData);
	HR_T(m_pDevice->CreateBuffer(&bd, nullptr, m_pParticleDataBuffer.GetAddressOf()));

	bd.ByteWidth = sizeof(CB_UIMesh);
	HR_T(m_pDevice->CreateBuffer(&bd, nullptr, m_pUIMeshBuffer.GetAddressOf()));
}


/// ImGui Render ///
void D3DRenderManager::ImguiRender()
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	{
		ImGui::Begin("Editor");

		std::string str;
		GetVideoMemoryInfo(str);
		ImGui::Text("VideoMemory: %s", str.c_str());
		GetSystemMemoryInfo(str);
		ImGui::Text("SystemMemory: %s", str.c_str());

		ImGui::Dummy(ImVec2(0.0f, 10.0f));
		//float camPos[3] = { m_View._41, m_View._42, m_View._43 };
		//ImGui::SliderFloat3("Cam_Pos", &camPos, -1000.0f, 1000.0f);

		ImGui::Dummy(ImVec2(0.0f, 10.0f));
		ImGui::Checkbox("Is Use Tex Edit", &m_isUseEdit);
		//ImGui::Checkbox("Is Use PointLight", &m_isUsePointLight);
		ImGui::Checkbox("RenderCubeMap", &m_isCubeMap);
		ImGui::Checkbox("UseIBL", &m_isUseIBL);
		ImGui::SliderFloat("IBL_Value", &m_IBLValue, 0.0f, 2.0f);
		ImGui::Checkbox("UseLightIBL", &m_UseLightIBL);
		ImGui::SliderFloat("IBL_LightValue", &m_LightIBLValue, 0.0f, 2.0f);
		ImGui::Checkbox("Gamma_Correction", &m_isGamma);

		if (m_isUseEdit == true)
		{
			//���Ľ�Ƽ�� �����״ٰ� �����.
			//ImGui::Checkbox("OpacityMap", &m_isOpacity);  
			ImGui::Checkbox("NormalMap", &m_isNormalMap);
			ImGui::Checkbox("SpecularMap", &m_isSpecularMap);
			ImGui::Checkbox("DiffuseMap", &m_isDiffuse);
			ImGui::Checkbox("EmissiveMap", &m_isEmissive);
			ImGui::Checkbox("MetalnessMap", &m_isMetalness);
			ImGui::SliderFloat("Metal_value", &m_MetalnessValue, 0.0f, 5.0f);
			ImGui::Checkbox("RoughnessMap", &m_isRoughness);
			ImGui::SliderFloat("Rough_value", &m_RoughnessValue, 0.0f, 5.0f);
		}

		//if (m_isUsePointLight == true)
		//{
		//	ImGui::SliderFloat3("PL_Pos", m_PointLightPos, -1000, 1000);
		//	ImGui::SliderFloat("PL_Range", &m_PointLightRange, 0, 1000);
		//	ImGui::SliderFloat("PL_Linear", &m_PointLightLinearTerm, 0, 100);
		//	ImGui::SliderFloat("PL_QuadraTic", &m_PointLightQuadraTicTerm, 0, 100);
		//	ImGui::SliderFloat("PL_Intencity", &m_PointLightIntencity, 0, 100);
		//	ImGui::ColorEdit3("PL_Color", m_PointLightColor);
		//}

		ImGui::Dummy(ImVec2(0.0f, 10.0f));
		ImGui::SliderFloat("Far", &m_Far, 1.0f, 100000.0f);
		ImGui::SliderFloat("Near", &m_Near, 0.01f, 2.0f);
		ImGui::SliderFloat("Fov", &m_Fov, -20.0f, 180.0f);

		ImGui::Dummy(ImVec2(0.0f, 10.0f));
		ImGui::SliderFloat3("Light_RGB", m_vLightColor, 0.0f, 1.0f);
		ImGui::SliderFloat3("Light_XYZ", m_vLightDir, -1.0f, 1.0f);

		ImGui::End();
	}

	{
		ImGui::Begin("Debug");

		ImGui::Checkbox("FreezeCulling", &m_IsFreezeCulling);
		ImGui::Checkbox("DrawDebugCulling", &m_IsDrawDebugCulling);
		ImGui::Checkbox("DrawShadowFrustum", &m_IsDrawShadowFrustum);
		ImGui::Checkbox("DrawDebugCollision", &m_IsDrawDebugCollision);

		ImGui::Dummy(ImVec2(0.0f, 10.0f));
		std::string str;
		str = to_string(m_nDrawRenderCount);
		ImGui::Text("RenderDrawCount : %s", str.c_str());

		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

