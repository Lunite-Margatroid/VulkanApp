#include "CommonVertexInput.slang"
#include "CommonVertexOutput.slang"

struct ModelMat2D{
	float2 modelMat_col0;
	float2 modelMat_col1;
	float2 offset;
};

struct VPMatrix2D{
	float2 vpMat_col0;
	float2 vpMat_col1;
	float2 offset;
};

[[vk::push_constant]]
ModelMat2D pc_ModelMat2D;

[[vk::binding(0,0)]]
ConstantBuffer<VPMatrix2D, Std140DataLayout> u_VPMatrix2D;

[shader("vertex")]
SVertexOutput VertMain(SVertexInput vertIn){
	SVertexOutput output;

	float2x2 vpMat = float2x2(u_VPMatrix2D.vpMat_col0.x, u_VPMatrix2D.vpMat_col0.y, u_VPMatrix2D.vpMat_col1.x, u_VPMatrix2D.vpMat_col1.y);
	float2x2 modelMat = float2x2(pc_ModelMat2D.modelMat_col0.x, pc_ModelMat2D.modelMat_col0.y, pc_ModelMat2D.modelMat_col1.x, pc_ModelMat2D.modelMat_col1.y);

// UV
#ifdef VERT_UV0
	output.fragUV0 = vertIn.vertUV0;
	float2 vec2FragPos = 
	mul(
		vpMat,
		mul(modelMat, vertIn.vertUV0) + pc_ModelMat2D.offset
		) + u_VPMatrix2D.offset;
	output.fragPos = float4(vec2FragPos, 1.0, 1.0);
#else
	float2 vec2FragPos = 
	mul(
		vpMat,
		mul(modelMat, float2(0,0)) + pc_ModelMat2D.offset
		) + u_VPMatrix2D.offset;
	output.fragPos = float4(vec2FragPos, 1.0, 1.0);
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
	output.fragNormal = mul(pc_ModelMat2D.normalMat, vertIn.vertNormal);
#endif

#ifdef VERT_TANGENT
	output.fragTangent = mul(pc_ModelMat2D.normalMat, vertIn.vertTangent);
#endif

#ifdef VERT_BITANGENT
	output.fragBitangent = mul(pc_ModelMat2D.normalMat, vertIn.vertBitangent);
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
	output.cameraWorldPos = float3(-u_VPMatrix2D.offset, 0);

// 观察向量
	output.viewVec = output.cameraWorldPos - float3(-pc_ModelMat2D.offset, 0);

// 模型坐标
#ifdef VERT_UV0
	output.modelPos = float3(vertIn.vertUV0, 0);
#else
	output.modelPos = float3(0.0, 0.0, 0.0);
#endif
	return output;
}