#version 330 core

layout(location = 0) in vec3 position; // 頂点位置
layout(location = 1) in vec2 inTexCoord; // テクスチャ座標（頂点属性）

out vec2 texCoord;

void main()
{
    gl_Position = vec4(position, 1.0);
    texCoord = inTexCoord; // フラグメントシェーダへ渡す
}