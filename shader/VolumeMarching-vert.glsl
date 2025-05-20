#version 330 core

layout(location = 0) in vec3 position; // 頂点位置(画面上下左右)
layout(location = 1) in vec2 inTexCoord; // UV(画面上下左右に0~1)

out vec2 texCoord;

//画面全てでピクセルシェーダーを起動することが目的であるため、パラメータ渡し以外何もしない
void main()
{
    // フラグメントシェーダへ渡す
    gl_Position = vec4(position, 1.0);
    texCoord = inTexCoord; 
}