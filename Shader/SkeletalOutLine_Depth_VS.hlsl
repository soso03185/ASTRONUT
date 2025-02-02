struct VS_INPUT
{
    float4 Pos : POSITION;
    float2 Tex : UV; // ���ؽ� ���̴��� �ø�ƽ�� �ǹ��ִ� �̸����� ���� �� ����.
    float3 Norm : NORMAL;
    float3 TangentWorld : TANGENT;
    int4 BlendIndices : BLENDINDICES;
    float4 BlendWeights : BLENDWEIGHT;
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
    float4x4 matWorldBlended = MatrixPaletteArray[0];
    
    // ���� ������ ���� ���� Ʈ������ ��ġ�� ���缭 ��ġ �ƴϸ� ���� ���� Ʈ������ ��ġ�� ��ġ

    // ���� ���� ��ġ
    matWorldBlended = mul(input.BlendWeights.x, MatrixPaletteArray[input.BlendIndices.x]);
    matWorldBlended += mul(input.BlendWeights.y, MatrixPaletteArray[input.BlendIndices.y]);
    matWorldBlended += mul(input.BlendWeights.z, MatrixPaletteArray[input.BlendIndices.z]);
    matWorldBlended += mul(input.BlendWeights.w, MatrixPaletteArray[input.BlendIndices.w]);
    pos = mul(pos, matWorldBlended);
    
    // ndc ���� ���� ��ġ
    pos = mul(pos, View);
    pos = mul(pos, Projection);
    output.ProjPos = pos;
    
    return output;
}