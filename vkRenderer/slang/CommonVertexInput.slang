// 定义顶点缓冲
struct SVertexInput{
#ifdef VERT_POSITION
	float3 vertPosition;
#endif

#ifdef VERT_UV0
	float2 vertUV0;
#endif

#ifdef VERT_UV1
	float2 vertUV1;
#endif

#ifdef VERT_UV2
	float3 vertUV2;
#endif

#ifdef VERT_UV3
	float3 vertUV3;
#endif

#ifdef VERT_UV4
	float3 vertUV4;
#endif

#ifdef VERT_NORMAL
	float3 vertNormal;
#endif

#ifdef VERT_TANGENT
	float3 vertTangent;
#endif

#ifdef VERT_BITANGENT
	float3 vertBitangent;
#endif

#ifdef VERT_COLOR
	float4 vertColor;
#endif

#ifdef VERT_AO
	float vertAO;
#endif

}; 