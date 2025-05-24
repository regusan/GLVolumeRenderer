#version 420 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in float alpha;

out vec4 fragColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec2 alphaRange;
uniform float pointSize;

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
    // アルファ範囲チェック
    if (alpha < alphaRange.x || alpha > alphaRange.y)
        gl_ClipDistance[0] = -1.0; // クリップ
    else
        gl_ClipDistance[0] = 1.0;

    // 位置設定
    gl_Position = projection * view * model * vec4(inPosition, 1.0);
    gl_PointSize = clamp(3.0 / gl_Position.z, 1.0, 100.0)*pointSize;
    fragColor = vec4(HSVtoRGB((1-alpha)*260, 1.0, alpha),alpha);
}
