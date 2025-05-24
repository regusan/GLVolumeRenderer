#version 420 core

in vec4 positionWS;
out vec4 FragColor;

uniform sampler3D volumeTexture;
uniform mat4 view;
uniform vec2 alphaRange;
uniform vec2 nearFarClip;
uniform int volumeResolution;

const float SQRT3=sqrt(3);
int maxSteps=int(sqrt(3.*volumeResolution*volumeResolution));//1ステップで1ボクセル参照するような長さにする(最悪でも)
const float boxFarestLength=SQRT3;//sqrt(1^2+1^2+1^2)バウンディングボックス内での最長距離

// HSV to RGB conversion
vec3 HSVtoRGB(float h,float s,float v)
{
    float c=v*s;
    float x=c*(1.-abs(mod(h/60.,2.)-1.));
    float m=v-c;
    
    vec3 rgb=vec3(0.);
    if(h<60.)rgb=vec3(c,x,0.);
    else if(h<120.)rgb=vec3(x,c,0.);
    else if(h<180.)rgb=vec3(0.,c,x);
    else if(h<240.)rgb=vec3(0.,x,c);
    else if(h<300.)rgb=vec3(x,0.,c);
    else if(h<360.)rgb=vec3(c,0.,x);
    
    return rgb+vec3(m);
}

void main()
{
    vec3 cameraPos=vec3(inverse(view)[3]);
    vec3 rayDir=normalize(positionWS.xyz-cameraPos);
    
    //バウンディングボックスに接触するまでの距離か、ニアクリップの距離の大きいほうをレイの開始点オフセットとする
    float rayStartOffset=nearFarClip.x;
    
    //レイの始点を計算
    vec3 initialPos;
    if(gl_FrontFacing)//表面が映ってるならその表面内からレイ開始
    {
        initialPos=positionWS.xyz+vec3(.5);
    }
    else{//背面が映っている＝カメラがボリューム内ならカメラ位置からレイ開始
        initialPos=cameraPos+vec3(.5)+rayDir*rayStartOffset;
    }
    
    // レイの最長距離(ボックス内での最長距離)か、ファークリップ距離の短いほう最短距離とする
    float maxRayDistance=min(boxFarestLength+rayStartOffset,nearFarClip.y);
    // 1回でレイを進める長さを計算。
    float stepSize=maxRayDistance/float(maxSteps);
    
    //加算されていく最終的な色と透明度
    float alphaAccum=0.;
    vec3 colorAccum=vec3(0.);
    
    //レイマーチング開始
    for(int i=0;i<maxSteps;i++)
    {
        vec3 currentPos=rayDir*stepSize*float(i)+initialPos;
        
        //開始は必ずボリューム内なのでボリューム外なら中断。
        if(any(lessThan(currentPos,vec3(0.)))||any(greaterThan(currentPos,vec3(1.))))
        {
            break;
        }
        
        float intensity=texture(volumeTexture,currentPos).r;//サンプル
        //alphamin~alphamaxの範囲を0~1に正規化して、残留αをかけて、現在のアルファ値とする。
        float alpha=smoothstep(alphaRange.x,alphaRange.y,intensity)*(1.-alphaAccum);
        //float alpha = clamp(intensity,alphaRange.x, alphaRange.y) * (1.0 - alphaAccum);
        
        //HSV変換された色を描画
        colorAccum+=HSVtoRGB((1-intensity)*260,1.,intensity)*alpha;
        alphaAccum+=alpha;
        
        // 十分不透明になったら終了
        if(alphaAccum>=.95)
        {
            break;
        }
        
    }
    
    //色を出力
    FragColor=vec4(colorAccum,alphaAccum);
}