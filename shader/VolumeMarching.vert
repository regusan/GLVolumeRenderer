#version 420 core

layout(location=0)in vec3 position;

out vec4 positionWS;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

void main()
{
    // フラグメントシェーダへ渡す
    positionWS=model*vec4(position,1.);
    gl_Position=projection*view*positionWS;
    
}