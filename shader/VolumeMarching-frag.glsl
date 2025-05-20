#version 330 core

in vec2 texCoord;
out vec4 FragColor;

uniform sampler3D volumeTexture;
uniform mat4 view;
uniform mat4 projection;
uniform vec2 alphaRange;
uniform vec2 nearFarClip;
uniform vec3 cameraPos;
uniform mat4 invViewProj;    // (projection * view)^-1

const float SQRT3 = sqrt(3);
const int resolution = 256;//ボクセルの解像度
const int maxSteps = int(sqrt(3.0*resolution*resolution));//1ステップで1ボクセル参照するような長さにする
const float maxRayLength = SQRT3;//sqrt(1^2+1^2+1^2)バウンディングボックス内での最長距離


// HSV to RGB conversion
vec3 HSVtoRGB(float h, float s, float v)
{
    float c = v * s;
    float x = c * (1.0 - abs(mod(h / 60.0, 2.0) - 1.0));
    float m = v - c;
    
    vec3 rgb = vec3(0.0);
    if (h < 60.0) rgb = vec3(c, x, 0.0);
    else if (h < 120.0) rgb = vec3(x, c, 0.0);
    else if (h < 180.0) rgb = vec3(0.0, c, x);
    else if (h < 240.0) rgb = vec3(0.0, x, c);
    else if (h < 300.0) rgb = vec3(x, 0.0, c);
    else if (h < 360.0) rgb = vec3(c, 0.0, x);

    return rgb + vec3(m);
}

void main()
{
    // スクリーン座標(0~1)からクリップ空間(-1~1)に変換
    vec4 clipPos = vec4(texCoord * 2.0 - 1.0, 0.0, 1.0);

    // クリップ空間UVからワールド空間へのレイの方向を計算
    vec4 worldPos = invViewProj * clipPos;
    worldPos /= worldPos.w;
    

    vec3 rayDir = normalize(worldPos.xyz - cameraPos);

    //レイの視点をバウンディングボックスへの入射位置と合わせるための計算
    float distanceToCentor = length(cameraPos - vec3(0));//バウンディングボックス中心までの最短距離
    float distanceToBox = distanceToCentor - SQRT3;//バウンディングボックスまでの最短距離の最悪距離
    //バウンディングボックスに接触するまでの距離か、ニアクリップの距離の大きいほうをレイの始点オフセットとする
    float rayStartOffset = max(distanceToBox, nearFarClip.x);

    //レイの始点を計算 = カメラ位置 + バウンディングボックスを中心にずらすvec(0.5) + レイの方向 * レイの始点オフセット
    vec3 initialPos = cameraPos + vec3(0.5) + rayDir * rayStartOffset;

    //レイのステップサイズを計算。レイの最長距離か、ファークリップ距離の短いほうを基準の長さとする
    float stepSize =min( maxRayLength, nearFarClip.y)/ float(maxSteps);

    //最終的な色と透明度
    float alphaAccum = 0.0;
    vec3 colorAccum = vec3(0.0);

    //レイマーチング開始
    for (int i = 0; i < maxSteps; i++)
    {
        vec3 currentPos = rayDir * stepSize * float(i) + initialPos;
        if (any(lessThan(currentPos, vec3(0.0))) || any(greaterThan(currentPos, vec3(1.0))))
        {//ボリューム外なら中断して次のレイを進める
            continue;
        }

        float intensity = texture(volumeTexture, currentPos).r;
        float alpha = smoothstep(alphaRange.x, alphaRange.y, intensity) * (1.0 - alphaAccum);

        colorAccum += HSVtoRGB((1-intensity)*260, 1.0, intensity) * alpha;
        alphaAccum += alpha;

        if (alphaAccum >= 0.95)
        {
            break;  // 十分不透明になったら終了
        }

    }

    //色を出力
    FragColor = vec4(colorAccum, alphaAccum);
}
