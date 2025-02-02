cbuffer cbPerFrame : register(b8)
{
    float4x4 ViewProjection;
    float3 EyePosWorld;
    float GameTime;
    float3 EmitPosWorld;
    float TimeStep;
    float3 EmitGenerationRange;
};
    
cbuffer cbParticleData : register(b9)
{
    float2 Size; // ũ��
    float CreatePerSecond; // 1�ʴ� ���� ����
    float LifeTime; // ��ƼŬ ���� �ð�
    float3 InitialSpeed; // �ʱ� �ӵ�
    int PlayParticle; // ��ƼŬ ���
    float3 Acceleration; // ���ӵ�
    int ParticleDirection; // ��ƼŬ ����
};

Texture2D Texture : register(t20);
Texture1D RandomTex : register(t21);
SamplerState samLinear : register(s0);

// ���� �ð��� ���� ������ offset�� ������� ���� ���͸� ���ø����ش�.
float3 RandUnitVec3(float offset)
{
    // ���� �ð� ���ϱ� �������� ������ �ؽ�ó ������ ����
    // �ؽ�ó ��ǥ�� ����Ѵ�.
    float u = (GameTime + offset);
    
    // ���� ���е��� ������ [-1, 1] �̴�.
    float3 v = RandomTex.SampleLevel(samLinear, u, 0).xyz;

    // ���� ���� �����Ѵ�.
    return normalize(v);
}

#define PT_EMITTER 0
#define PT_FLARE 1

struct Particle
{
    float3 InitialPosW : POSITION;
    float3 InitialVelW : VELOCITY;
    float2 SizeW : SIZE;
    float Age : AGE;
    uint Type : TYPE;
};

Particle StreamOutVS(Particle vin)
{
    return vin;
}

// ��Ʈ�� ��� ���� ���� ���̴��� �� ������ ����� ���� ������
// �ı��� ����Ѵ�. ���� �ý��۸��� ������ ����.�ı� ��Ģ�� �ٸ� ���̹Ƿ�,
// �� �κ��� ���� ���� ���� �ý��۸��� �ٸ� �ʿ䰡 �ִ�.
[maxvertexcount(2)]
void StreamOutGS(point Particle gin[1],
                 inout PointStream<Particle> ptStream)
{
    // ���� ��ƼŬ�� ���̸� ������Ʈ
    gin[0].Age += TimeStep;

    // ������ ��ƼŬ�� ���
    if (gin[0].Type == PT_EMITTER && PlayParticle)
    {
        // ��ƼŬ�� Ư�� �ð�(0.005��) �̻� �� ���
        if (gin[0].Age > (1 / CreatePerSecond))
        {
            // 3D �������� ������ ���� ���͸� ����
            float3 vRandom = RandUnitVec3(0.0f);
            // x �� z ������ ����Ͽ� ������ ����
            vRandom.x *= 0.5f;
            vRandom.y *= 0.5f;
            vRandom.z *= 0.5f;
            
            if (vRandom.y < 0)
                vRandom.y = -vRandom.y + 0.5f;
            
            // ���ο� ��ƼŬ�� �����Ͽ� ��Ʈ���� �߰�
            Particle p;
            p.InitialPosW.x = EmitPosWorld.x + vRandom.x * EmitGenerationRange.x ; // �ʱ� ��ġ ����
            p.InitialPosW.y = EmitPosWorld.y + vRandom.y * EmitGenerationRange.y ; // �ʱ� ��ġ ����
            p.InitialPosW.z = EmitPosWorld.z + vRandom.z * EmitGenerationRange.z ; // �ʱ� ��ġ ����
            p.InitialVelW.x = InitialSpeed.x * vRandom.x; // �ʱ� �ӵ� ����
            p.InitialVelW.y = InitialSpeed.y * vRandom.y; // �ʱ� �ӵ� ����
            p.InitialVelW.z = InitialSpeed.z * vRandom.z; // �ʱ� �ӵ� ����
            p.SizeW = Size * (vRandom.y + 0.5); // ũ�� ����
            p.Age = 0.0f; // ���� �ʱ�ȭ
            p.Type = PT_FLARE; // ��ƼŬ ���� ����

            ptStream.Append(p); // ������ ��ƼŬ�� ��Ʈ���� �߰�

            gin[0].Age = 0.0f; // ������ ��ƼŬ�� ���̸� �缳��
        }

        // ����� ���� �ϳ��� �׻� �����Ѵ�
        ptStream.Append(gin[0]);
    }
    else
    {
        // �����Ͱ� �ƴ� ���, ��ƼŬ�� ���̰� 1.0���� �۰ų� ���� ��쿡�� ���� ��Ʈ���� �߰�
        if (gin[0].Age <= LifeTime)
            ptStream.Append(gin[0]);
    }
}

struct VertexOut
{
    float3 PosW : POSITION;
    float2 SizeW : SIZE;
    float4 Color : COLOR;
    uint Type : TYPE;
};

VertexOut DrawVS(Particle vin)
{
    VertexOut vout;

	// ������ ������ ���� ��¥ ��� ���ӵ�
    float3 gAccelW = Acceleration;

    float t = vin.Age;
    vout.PosW = 0.5f * t * t * Acceleration + t * vin.InitialVelW + vin.InitialPosW;
    float opacity = 1.0f - smoothstep(0.0f, 1.0f, t / LifeTime);
    vout.Color = float4(1.0f, 1.0f, 1.0f, opacity);
    vout.SizeW = vin.SizeW / (t + 0.9);
    vout.Type = vin.Type;

    return vout;
}

struct GeoOut
{
    float4 PosH : SV_Position;
    float4 Color : COLOR;
    float2 Tex : TEXCOORD;
};

// �������� ���� ���̴��� �׳� ���� ī�޶� ���� �簢������ Ȯ���Ѵ�.
[maxvertexcount(4)]
void DrawGS(point VertexOut gin[1],
	inout TriangleStream<GeoOut> triStream)
{
    float2 gQuadTexC[4] =
    {
        float2(1.0f, 0.0f),
		float2(1.0f, 1.0f),
		float2(0.0f, 0.0f),
		float2(0.0f, 1.0f)
    };
	
    if (gin[0].Type != PT_EMITTER)
    {
        float3 look = normalize(EyePosWorld.xyz - gin[0].PosW);
        float3 right = normalize(cross(float3(0, 1, 0), look));
        float3 up = cross(look, right);
		
        float halfWidth = 0.5f * gin[0].SizeW.x;
        float halfHeight = 0.5f * gin[0].SizeW.y;

        float4 v[4];
        v[0] = float4(gin[0].PosW + halfWidth * right - halfHeight * up, 1.0f);
        v[1] = float4(gin[0].PosW + halfWidth * right + halfHeight * up, 1.0f);
        v[2] = float4(gin[0].PosW - halfWidth * right - halfHeight * up, 1.0f);
        v[3] = float4(gin[0].PosW - halfWidth * right + halfHeight * up, 1.0f);
		
        GeoOut gout;
		[unroll]
        for (int i = 0; i < 4; ++i)
        {
            gout.PosH = mul(v[i], ViewProjection);
            gout.Tex = gQuadTexC[i];
            gout.Color = gin[0].Color;
            triStream.Append(gout);
        }
    }
}

float4 DrawPS(GeoOut pin) : SV_TARGET
{
    return Texture.Sample(samLinear, pin.Tex) * pin.Color;
}
