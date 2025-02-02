
struct VS_INPUT
{
    float4 Pos : POSITION;
    float2 Tex : UV; // ���ؽ� ���̴��� �ø�ƽ�� �ǹ��ִ� �̸����� ���� �� ����.
    float3 Norm : NORMAL;
    float3 TangentWorld : TANGENT;
};

struct PS_INPUT
{
    float4 ProjPos : SV_POSITION;
};

cbuffer TransformW : register(b2)
{
    matrix World;
}

cbuffer TransformVP : register(b3)
{
    matrix View;
    matrix Projection;
    matrix ShadowView;
    matrix ShadowProjection;
}

cbuffer MatrixPalette : register(b5)
{
    matrix MatrixPaletteArray[128];
}

PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;

    // ������Ʈ ���� ��ȯ
    float4 pos = input.Pos;
    pos = mul(pos, World);
    // ndc ���� ���� ��ġ
    pos = mul(pos, ShadowView);
    pos = mul(pos, ShadowProjection);
    output.ProjPos = pos;
    
    return output;
}