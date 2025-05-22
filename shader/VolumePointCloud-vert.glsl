#version 420 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in float alpha;

out vec4 fragColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec2 alphaRange;
uniform float pointSize;

void main()
{
    // アルファ範囲チェック
    if (alpha < alphaRange.x || alpha > alphaRange.y)
        gl_ClipDistance[0] = -1.0; // クリップ
    else
        gl_ClipDistance[0] = 1.0;

    // 位置設定
    gl_Position = projection * view * model * vec4(inPosition, 1.0);
    gl_PointSize = clamp(3.0 / gl_Position.z, 1.0, 100.0)*pointSize;
    fragColor =vec4(vec3(mod(inPosition*1000,1)),alpha);
}
