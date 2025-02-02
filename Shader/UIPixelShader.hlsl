struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : UV;
};

cbuffer FadeCB : register(b7)
{
    float deltaTime;
    int IsDim;
    int UseDim;
    int IsFade;
    int UseFadeOut;
    int UseFadeIn;
    float durationTime = 0.0f;
    float pad;
}

Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);
SamplerState samLinear1 : register(s1);
SamplerState samLinear2 : register(s2);

float currentTime = 0.0f;

float4 main(PS_INPUT input) : SV_TARGET
{
    if (IsFade)
    {
        if (UseFadeOut)  // FadeOut
        {
        // DeltaTime
            float currentTime = deltaTime;
            float FadeDuration = durationTime; // FadeIn �� FadeOut ���� �ð� (��)    

        // FadeIn �� FadeOut ȿ���� ����մϴ�.
            float alpha = saturate(currentTime / FadeDuration); // FadeIn �ð����� 0���� 1��
   
        // Fade ����
            float3 FadeColor = float3(0, 0, 0); // FadeIn �� FadeOut ����
            float4 finalColor = float4(FadeColor, alpha);
    
            return finalColor;
        }
    
        if (UseFadeIn)  // FadeIn
        {
        // DeltaTime
            float currentTime = deltaTime;
            float FadeDuration = durationTime; // FadeIn �� FadeOut ���� �ð� (��)    

        // FadeIn �� FadeOut ȿ���� ����մϴ�.
            float alpha = saturate(currentTime / FadeDuration); // FadeIn �ð����� 0���� 1��
   
        // Fade ����
            float3 FadeColor = float3(0, 0, 0); // FadeIn �� FadeOut ����
            float4 finalColor = float4(FadeColor, 1 - alpha);
    
            return finalColor;
        }
        
        discard;
    }
    
    
    if (IsDim)
    {
        if(UseDim)
        {            
            float3 DimColor = float3(0, 0, 0);
            float4 finalColor = float4(DimColor, 0.7f); // alpha

            return finalColor;
        }
        
        discard;
    }
    
    
    float4 textureColor = txDiffuse.Sample(samLinear, input.Tex);
     
    // ����� ȸ���� ��쿡 �ȼ��� ������.
    // �÷�Ʈ ���� ��Ȯ�� ������ �ȵǼ� 0.01���� ���� �ٿ����ϴ�. 
    // �� ���� ��� ������ ������...
    if (textureColor.r >= 0.49f && textureColor.r <= 0.51f && textureColor.g >= 0.49f && textureColor.g <= 0.51f &&textureColor.b >= 0.49f && textureColor.b <= 0.51f)
    {
        discard;
    }
    
    return textureColor;

}