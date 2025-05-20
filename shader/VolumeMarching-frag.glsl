#version 330 core

in vec2 texCoord;
out vec4 FragColor;

uniform sampler3D volumeTexture;
uniform mat4 view;
uniform mat4 projection;
uniform vec2 alphaRange;      // alphaRange.x: min threshold, alphaRange.y: max threshold
uniform vec3 cameraPos;
uniform mat4 invViewProj;    // (projection * view)^-1

const int maxSteps = 1000;
const float stepSize = 3.0 / float(maxSteps);

void main()
{
    // スクリーン座標(0~1)からクリップ空間(-1~1)に変換
    vec4 clipPos = vec4(texCoord * 2.0 - 1.0, 0.0, 1.0);

    // クリップ空間からワールド空間へのレイの方向を計算
    vec4 worldPos = invViewProj * clipPos;
    worldPos /= worldPos.w;
    

    vec3 rayDir = normalize(worldPos.xyz - cameraPos);

    vec3 pos = cameraPos + vec3(0.5);

    float alphaAccum = 0.0;
    vec3 colorAccum = vec3(0.0);

    for (int i = 0; i < maxSteps; i++)
    {
    
        float intensity = texture(volumeTexture, pos).r;
        if (any(lessThan(pos, vec3(0.0))) || any(greaterThan(pos, vec3(1.0))))
        {
        pos += rayDir * stepSize;
        continue;
        }

        pos += rayDir * stepSize;

        // alphaRangeで調整
        float alpha = smoothstep(alphaRange.x, alphaRange.y, intensity) * (1.0 - alphaAccum);

        // カラーは強度をそのままグレースケールに
        colorAccum += vec3(intensity) * alpha;
        alphaAccum += alpha;

        if (alphaAccum >= 0.95)
        {
            break;  // 十分不透明になったら終了
        }

    }

    FragColor = vec4(colorAccum, alphaAccum);
}
