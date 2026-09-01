#include "CommonVertexInput.slang"
#include "CommonVertexOutput.slang"


struct MVPMatrix{
	float4x4 modelMat;
	float4x4 viewMat;
	float4x4 projectiondMat;
	float4x4 mvpMat;
	float3x3 normalMat;
	float3 cameraPos;
};

// (binding, set)
// binding = 0
// set = 0 : VertexShader
[[vk::binding(0, 0)]]
ConstantBuffer<MVPMatrix, Std140DataLayout> u_MVPMatix;

[shader("vertex")]
SVertexOutput VertMain(SVertexInput vertIn){
	SVertexOutput output;
// 顶点坐标
#ifdef VERT_POSITION
	output.fragPos = mul(u_MVPMatix.mvpMat, float4(vertIn.vertPosition, 1.0));
	output.fragWorldPos = mul(u_MVPMatix.modelMat, float4(vertIn.vertPosition, 1.0)).xyz;
#else
	output.fragPos = mul(u_MVPMatix.mvpMat, float4(0.0, 0.0, 0.0, 1.0));
	output.fragWorldPos = mul(u_MVPMatix.modelMat, float4(0.0, 0.0, 0.0, 1.0)).xyz;
#endif

// UV
#ifdef VERT_UV0
	output.fragUV0 = vertIn.vertUV0;
#endif

#ifdef VERT_UV1
	output.fragUV1 = vertIn.vertUV1;
#endif

#ifdef VERT_UV2
	output.fragUV2 = vertIn.vertUV2;
#endif

#ifdef VERT_UV3
	output.fragUV3 = vertIn.vertUV3;
#endif

#ifdef VERT_UV4
	output.fragUV4 = vertIn.vertUV4;
#endif

// 法线和切线
#ifdef VERT_NORMAL
	output.fragNormal = mul(u_MVPMatix.normalMat, vertIn.vertNormal);
#endif

#ifdef VERT_TANGENT
	output.fragTangent = mul(u_MVPMatix.normalMat, vertIn.vertTangent);
#endif

#ifdef VERT_BITANGENT
	output.fragBitangent = mul(u_MVPMatix.normalMat, vertIn.vertBitangent);
#endif
	
// 顶点颜色	
#ifdef VERT_COLOR
	output.fragVertexColor = vertIn.vertColor;
#endif

// 顶点AO
#ifdef VERT_AO
	output.fragAO = vertIn.vertAO;
#endif

// 相机世界坐标
	output.cameraWorldPos = u_MVPMatix.cameraPos;

// 观察向量
	output.viewVec = normalize(u_MVPMatix.cameraPos - output.fragWorldPos);

// 模型坐标
#ifdef VERT_POSITION
	output.modelPos = vertIn.vertPosition;
#else
	output.modelPos = float3(0.0, 0.0, 0.0);
#endif
	return output;
}