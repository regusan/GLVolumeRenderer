#version 420 core

in vec2 texCoord;// UV座標 (0~1)
out vec4 FragColor;

uniform sampler3D volumeTexture;
uniform vec2 alphaRange;// alphaRange.x: min threshold, alphaRange.y: max threshold

const int maxSteps=64;
const float stepSize=1./float(maxSteps);

void main()
{
// レイの始点はUV座標の(x,y), z=0 の面上
vec3 rayPos=vec3(texCoord,0.);

// レイ方向はZ軸正方向
vec3 rayDir=vec3(0.,0.,1.);

float alphaAccum=0.;
vec3 colorAccum=vec3(0.);

for(int i=0;i<maxSteps;++i)
{
    if(rayPos.z>1.)break;// ボリューム外に出たら終了
    
    float intensity=texture(volumeTexture,rayPos).r;
    
    float alpha=smoothstep(alphaRange.x,alphaRange.y,intensity)*(1.-alphaAccum);
    
    colorAccum=vec3(intensity)*alpha;
    alphaAccum+=alpha;
    
    if(alphaAccum>=.95)break;
    
    rayPos+=rayDir*stepSize;
}

FragColor=vec4(colorAccum,alphaAccum);
}
