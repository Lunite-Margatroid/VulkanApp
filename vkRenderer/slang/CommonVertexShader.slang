#include "CommonVertexInput.slang"
#include "CommonVertexOutput.slang"



struct ModelNormalMat {
	float4x4 modelMat;
	float3x3 normalMat;
};

struct VPMatrix{
	float4x4 viewMat;
	float4x4 projectiondMat;
	float4x4 vpMat;
	float3 cameraPos;
};


[[vk::push_constant]]
ModelNormalMat pc_ModelMat;

// (binding, set)
// binding = 0
// set = 0 : TransBuffer
[[vk::binding(0, 0)]]
ConstantBuffer<VPMatrix, Std140DataLayout> u_VPMatix;

[shader("vertex")]
SVertexOutput VertMain(SVertexInput vertIn){
	SVertexOutput output;
// 顶点坐标
#ifdef VERT_POSITION
	output.fragPos = mul(mul(u_VPMatix.vpMat, pc_ModelMat.modelMat), float4(vertIn.vertPosition, 1.0));
	output.fragWorldPos = mul(pc_ModelMat.modelMat, float4(vertIn.vertPosition, 1.0)).xyz;
#else
	output.fragPos = mul(mul(u_VPMatix.vpMat, pc_ModelMat.modelMat), float4(0.0, 0.0, 0.0, 1.0));
	output.fragWorldPos = mul(pc_ModelMat.modelMat, float4(0.0, 0.0, 0.0, 1.0)).xyz;
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
	output.fragNormal = mul(pc_ModelMat.normalMat, vertIn.vertNormal);
#endif

#ifdef VERT_TANGENT
	output.fragTangent = mul(pc_ModelMat.normalMat, vertIn.vertTangent);
#endif

#ifdef VERT_BITANGENT
	output.fragBitangent = mul(pc_ModelMat.normalMat, vertIn.vertBitangent);
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
	output.cameraWorldPos = u_VPMatix.cameraPos;

// 观察向量
	output.viewVec = normalize(u_VPMatix.cameraPos - output.fragWorldPos);

// 模型坐标
#ifdef VERT_POSITION
	output.modelPos = vertIn.vertPosition;
#else
	output.modelPos = float3(0.0, 0.0, 0.0);
#endif
	return output;
}