#include "Shared_UI.hlsli"

// �ȼ� ���̴�

float4 main(VS_INPUT input) : SV_TARGET
{
    // DeltaTime
    float currentTime = deltaTime;
    float FadeDuration = 2.0; // FadeIn �� FadeOut ���� �ð� (��)    

    // FadeIn �� FadeOut ȿ���� ����մϴ�.
    float alpha = saturate(currentTime / FadeDuration); // FadeIn �ð����� 0���� 1��
   
    // FadeOut �ð����� 1���� 0��
    alpha = (currentTime > FadeDuration) ? (2.0 - alpha) : alpha;

    // Fade ����
    float3 FadeColor = float3(0, 0, 0); // FadeIn �� FadeOut ����
    float4 finalColor = float4(FadeColor, alpha);
    
    return finalColor;
}