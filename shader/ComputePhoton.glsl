#version 450 core

// ローカルワークグループサイズを定義 (8x8x8 = 512スレッド/グループ)
// このサイズはハードウェアによって最適な値が異なる
layout(local_size_x=8,local_size_y=8,local_size_z=8)in;

// 3Dイメージバッファを定義
// binding = 0: C++側で glBindImageTexture の第一引数に指定したユニット番号
// r32f: C++側で指定したフォーマット
// writeonly: ここでは書き込み専用 (readonly, readwrite も可能)
layout(binding=1,r32f)uniform writeonly image3D photonVolume;

void main(){
    // このスレッドが担当するグローバルなID (3D座標) を取得
    ivec3 storePos=ivec3(gl_GlobalInvocationID.xyz);
    
    // バッファのサイズを取得
    ivec3 size=imageSize(photonVolume);
    
    // バッファの範囲内かチェック (念のため)
    if(storePos.x<size.x&&storePos.y<size.y&&storePos.z<size.z){
        
        // 書き込む値を計算 (例: 座標の和を正規化)
        float value=float(storePos.x+storePos.y+storePos.z)/
        float(size.x+size.y+size.z);
        
        // 3Dイメージバッファに値を書き込む
        // storePos: 書き込む3D座標
        // vec4(value, 0.0, 0.0, 0.0): 書き込む値 (r32fなのでx成分のみ使用)
        imageStore(photonVolume,storePos,vec4(1,0.,0.,0.));
    }
}