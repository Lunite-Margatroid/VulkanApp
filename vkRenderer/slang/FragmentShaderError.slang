#include "CommonVertexOutput.slang"

[shader("fragment")]
float4 FragMain(SVertexOutput sVertOut) : SV_Target{
	float cellSize = 1.0;  // 棋盘格大小

    // 1. 将世界坐标按格子大小缩放，取整得到格子索引
    int3 grid = int3(floor(sVertOut.modelPos / cellSize));

    // 2. 奇偶判断：三个坐标之和为偶数则品红，奇数则黑色
    bool isMagenta = ((grid.x + grid.y + grid.z) % 2 == 0);

    // 3. 返回颜色
    float3 magenta = float3(1.0, 0.0, 1.0);
    float3 black   = float3(0.0, 0.0, 0.0);
    
    return float4(isMagenta ? magenta : black, 1.0);
}