// 定义顶点着色器输出
struct SVertexOutput{
	float4 fragPos : SV_Position;
#ifdef VERT_UV0
	float2 fragUV0;
#endif

#ifdef VERT_UV1
	float2 fragUV1;
#endif

#ifdef VERT_UV2
	float3 fragUV2;
#endif

#ifdef VERT_UV3
	float3 fragUV3;
#endif

#ifdef VERT_UV4
	float3 fragUV4;
#endif

#ifdef VERT_NORMAL
	float3 fragNormal;
#endif

#ifdef VERT_TANGENT
	float3 fragTangent;
#endif

#ifdef VERT_BITANGENT
	float3 fragBitangent;
#endif

#ifdef VERT_COLOR
	float4 fragVertexColor;
#endif

#ifdef VERT_AO
	float fragAO;
#endif
	// 片段世界坐标
	float3 fragWorldPos;
	// 相机世界坐标
	float3 cameraWorldPos;
	// 观察向量 单位向量
	// 从frag指向相机
	float3 viewVec;
	// 模型坐标
	float3 modelPos;
}; 