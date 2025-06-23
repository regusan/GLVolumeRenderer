#version 420 core

in vec4 fragColor;
in vec2 centorSS;
in float o_pointSize;
//in vec2 viewPort;
out vec4 color;

void main(){
    
    vec2 coord=gl_PointCoord*2.-1.;// [-1, 1] に変換（中心が 0）
    //#XXX:Windows環境ではgl_PointCoordがゼロになる!!!????
    color=fragColor;
    return;//WIndows環境ではしょうがないので早期リターン
    
    if(dot(coord,coord)>1.){
        discard;// 円形の外は描画しない
    }
    
    // 2Dガウス分布
    float sigma=.5;// 標準偏差
    float r2=dot(coord,coord);
    float rate=exp(-r2/(2.*sigma*sigma));
    
    color=fragColor*rate;
}