#version 420 core

//画面全てでピクセルシェーダーを起動することが目的であるため、パラメータ渡し以外何もしない

layout(location = 0) in vec3 position; // 画面上下左右を覆う頂点
layout(location = 1) in vec2 inTexCoord; // 画面上下左右に0~1をとるUV

out vec2 texCoord;

void main()
{
    // フラグメントシェーダへ渡す
    gl_Position = vec4(position, 1.0);
    texCoord = inTexCoord; 
}