#version 420 core

layout(location=0)in vec3 inPosition;
layout(location=1)in float alpha;

out vec4 fragColor;
out vec2 centorSS;
out float o_pointSize;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec2 alphaRange;
uniform float pointSize;
uniform int volumeResolution;

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
    // アルファ範囲チェック
    if(alpha<alphaRange.x||alpha>alphaRange.y)
    {
        gl_ClipDistance[0]=-1.;// クリップ
    }
    else
    {
        gl_ClipDistance[0]=1.;
    }
    
    // 位置設定
    gl_Position=projection*view*model*vec4(inPosition,1.);
    centorSS=gl_Position.xy / gl_Position.w * 0.5 + 0.5;
    gl_PointSize = 10;
    gl_PointSize=clamp(3./gl_Position.z,1.,100.)*pointSize;
    o_pointSize=gl_PointSize;
    
    float smoothedAlpha=smoothstep(alphaRange.x,alphaRange.y,alpha);
    fragColor=vec4(HSVtoRGB((1-smoothedAlpha)*260,1.,smoothedAlpha),smoothedAlpha);
}
