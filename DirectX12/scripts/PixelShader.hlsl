//出力ファイルをassets/%(Filename).csoに変更
#include <Header.hlsli>
#define MIN_DIST (0.01)
#define MIN_LIGHT_INTENSITY (0.2)
#define MAX_LIGHT_INTENSITY (2.5)

//ポイントライト
//ポイントライトのカリングをスムーズに
float smoothDistanceAttenuation(
    float squareDistance,   //ライトからの距離の二乗
    float invSqrAttRadius //ライトが届く距離の二乗の逆数
)
{
    float factor = squareDistance * invSqrAttRadius;
    float smoothFactor = saturate(1.0f - factor * factor);
    return smoothFactor * smoothFactor;
}

//ポイントライトの距離による減衰を計算    
float getDistanceAttenuation(
    float3 unnormalizedLightVector, //ライト位置からピクセル位置へのベクトル
    float invSqrAttRadius //ライトが届く距離の二乗の逆数
)
{
    float sqrDist = dot(unnormalizedLightVector, unnormalizedLightVector);
    float attenuation = 1.0f / (max(sqrDist, MIN_DIST * MIN_DIST));
    
    attenuation *= smoothDistanceAttenuation(sqrDist, invSqrAttRadius);
    return attenuation;
}

float3 evaluatePointLight(
    float3 surfacePos, //ピクセル位置
    float3 lightPos, //ライト位置
    float3 lightColor, //ライト色
    float lightIntensity, //ライト強度
    float lightInvRadiusSq //ライトの届く距離の二乗の逆数
)
{
    float att = getDistanceAttenuation(lightPos - surfacePos, lightInvRadiusSq);
    
    return lightColor * lightIntensity * att;
}

//スポットライト
//角度による減衰
float getAngleAttenuation(
float cos_s, //ライト方向ベクトルと光源ベクトルの内積
float cos_p, //内側角のcos
float cos_u //外側角のcos
)
{
    float d = max(cos_p - cos_u, MIN_DIST);
    float t = saturate((cos_s - cos_u) / d);
    return t * t;
}

//距離減衰
float3 evaluateSpotLight(
    float3 worldPos, //ピクセル位置
    float3 lightPos, //ライト位置
    float lightInvRadiusSq, //ライトの届く距離の二乗の逆数
    float3 lightForward, //ライトの向き
    float3 lightColor, //ライト色
    float lightIntensity, //ライト強度
    float lightInnerCos, //内側角のcos
    float lightOuterCos //外側角のcos
)
{
    float3 unnnormalizedLightVector = lightPos - worldPos;
    float3 L = normalize(unnnormalizedLightVector);
    float att = getAngleAttenuation(dot(-L, lightForward), lightInnerCos, lightOuterCos);
    att *= getDistanceAttenuation(unnnormalizedLightVector, lightInvRadiusSq);
    return lightColor * lightIntensity * att;
}

float3 evaluateColor
(
    float3 lightVector,  //ライトベクトル
    float3 ViewVector,  //視線ベクトル
    float3 normalVector, //法線ベクトル
    float3 diffuseColor,
    float3 specularColor,
    float shininess,
    float3 brightness
)
{
    float3 color = float3(0, 0, 0);
    
    //diffuse
    color += (diffuseColor * brightness) * max(0, dot(normalVector, normalize(lightVector))) * 0.8f;
    
    //specular
    float3 halfWayVector = normalize(normalize(lightVector) + normalize(ViewVector));
    color += specularColor * brightness * pow(max(0, dot(halfWayVector, normalVector)), shininess) * 0.3f;
    
    return color;

}


float4 main(
	in float4 i_svpos : SV_POSITION,
    in float2 i_uv : TEXCOORD0,
    in float4 i_wpos : TEXCOORD1,
    in float4 i_wnormal : TEXCOORD2
	) : SV_TARGET
{
    float att = 0.0f;
    float3 brightness = float3(0, 0, 0);
    float3 outputColor = float3(0, 0, 0);
    
    //ポイントライト
    for (int i = 0; i < NUM_LIGHTS; i++)
    {
        if (pointLights[i].setValue.x == 1)
        {
            brightness = evaluatePointLight(i_wpos.xyz, pointLights[i].position.xyz, pointLights[i].color.xyz, pointLights[i].setValue.y, 1.0f / (pointLights[i].setValue.z * pointLights[i].setValue.z));
            outputColor += evaluateColor(pointLights[i].position.xyz - i_wpos.xyz, CameraPos.xyz - i_wpos.xyz, i_wnormal.xyz, Diffuse.xyz, Specular.xyz, Specular.w, brightness);
        }
        else continue;
    }
    
    //スポットライト
    for (int j = 0; j < NUM_LIGHTS; j++)
    {
        if (spotLights[j].setValue.x == 1)
        {
            brightness = evaluateSpotLight(i_wpos.xyz, spotLights[j].position.xyz, 1.0f / (spotLights[j].setValue.z * spotLights[j].setValue.z), spotLights[j].direction.xyz, spotLights[j].color.xyz, spotLights[j].setValue.y, cos(spotLights[j].attAngle.x), cos(spotLights[j].attAngle.y));
            outputColor += evaluateColor(spotLights[j].position.xyz - i_wpos.xyz, CameraPos.xyz - i_wpos.xyz, i_wnormal.xyz, Diffuse.xyz, Specular.xyz, Specular.w, brightness);

        }
        else continue;
    }
    
    //光が強すぎる場合の補正
    if (outputColor.r > MAX_LIGHT_INTENSITY || outputColor.g > MAX_LIGHT_INTENSITY || outputColor.b > MAX_LIGHT_INTENSITY)
    {
        outputColor = outputColor / max(outputColor.r, max(outputColor.g, outputColor.b)) * MAX_LIGHT_INTENSITY; //最大の色成分をMAX_LIGHT_INTENSITYに合わせる
    }
    //光が弱すぎる場合の補正
    outputColor = max(outputColor, MIN_LIGHT_INTENSITY);
    
    //点滅処理
    outputColor = lerp(outputColor, playerFlashColor, playerFlashIntensity);
    outputColor = lerp(outputColor, FlashColor, FlashIntensity);
    
    return Texture.Sample(Sampler, i_uv) * float4(outputColor, 1.0f);
}
