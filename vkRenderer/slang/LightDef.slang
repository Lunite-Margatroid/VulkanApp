// 灯光定义与光照计算
// 非点光源(球形光 矩形光)使用最近点近似(Most Representative Point)
#ifndef LIGHTDEF_SLANG
#define LIGHTDEF_SLANG

#include "ShadeLib.slang"
#include "BRDFLib.slang"

//std::140
// 64byte 与C++侧LightUniformBuffer(std::130)内存布局一致
struct LightUniformBuffer {
	float3 position; // 12byte 世界坐标
	float attenuation0; // 4byte 照明范围
	float3 direction; // 16byte 灯光朝向
	float attenuation1; // 4byte 衰减(暂未使用)
	float3 color; // 16byte // 颜色
	int nLightType; // -1标志该块缓冲不起作用
	// 8byte
	float sizeU; // 球形光: 半径 / 矩形光: U方向半宽
	float sizeV; // 矩形光: V方向半高
	// 8byte
	float radianOuter; // 聚光灯外锥角(弧度)
	float radianInner; // 聚光灯内锥角(弧度)
};
//int nLightType:
	//enum class LightType : int {
	//	eNone = -1,
	//	eLightPoint,
	//	eLightSphere,
	//	eLightSpot,
	//	eLightRect,
	//	eLightDirectional,
	//	eLightAmbient
	//};

// 与C++侧LightType枚举对应
static const int LIGHT_TYPE_NONE = -1;
static const int LIGHT_TYPE_POINT = 0;
static const int LIGHT_TYPE_SPHERE = 1;
static const int LIGHT_TYPE_SPOT = 2;
static const int LIGHT_TYPE_RECT = 3;
static const int LIGHT_TYPE_DIRECTIONAL = 4;
static const int LIGHT_TYPE_AMBIENT = 5;

// 灯光组常量缓冲 所有灯光组一次性传入着色器
// 布局: [灯光组][组内灯光]
// 数量与LIGHT_GROUP_MAX_COUNT / LIGHT_MAX_COUNT_PER_GROUP_FORWARD一致
// 总大小 16×16×64 = 16KB 不超过Vulkan保证的maxUniformBufferRange最小值
// 绑定信息需与渲染器侧一致
static const uint LIGHT_GROUP_BINDING_SPACE = 2; // 片段着色器资源空间
static const uint LIGHT_GROUP_BINDING_INDEX = 0;
static const uint LIGHT_GROUP_MAX_COUNT = 16; // 灯光组数量
static const uint LIGHT_GROUP_LIGHT_COUNT = 16; // 每个灯光组的灯光数量

[[vk::binding(LIGHT_GROUP_BINDING_INDEX, LIGHT_GROUP_BINDING_SPACE)]]
ConstantBuffer<LightUniformBuffer[LIGHT_GROUP_MAX_COUNT][LIGHT_GROUP_LIGHT_COUNT]> u_LightGroups;

// 距离衰减 照明范围fRange内从1渐变到0
// fRange<=0时使用平方反比衰减 无范围限制(面光源未配置范围时)
float DistanceAttenuation(float fDist, float fRange){
	if (fRange <= 0.0){
		return 1.0 / max(fDist * fDist, 1e-5);
	}
	float fAtt = saturate(1.0 - fDist / fRange);
	return fAtt * fAtt;
}

// PointLight 点光源
void PointLightIlluminate(in LocalSurfaceInfo surface, in LightUniformBuffer light, inout FragInfo frag){
	float3 vToLight = light.position - surface.vWorldPos;
	float fDist = length(vToLight);
	float3 vLightDir = vToLight / max(fDist, 1e-5);
	float fAtt = DistanceAttenuation(fDist, light.attenuation0);

	float3 cSpecular, cDiffuse;
	EvaluateDirectLight(surface, vLightDir, light.color * fAtt, cSpecular, cDiffuse);
	FragInfoAccumulate(frag, cSpecular, cDiffuse);
}

// 球光代表性点 反射射线上离球面最近的点
float3 SphereRepresentativePoint(float3 vSurfacePos, float3 vReflectDir, float3 cCenter, float fRadius){
	float t = dot(cCenter - vSurfacePos, vReflectDir);
	t = max(t, 0.0);
	float3 vRayPoint = vSurfacePos + vReflectDir * t;
	float3 vOffset = vRayPoint - cCenter;
	float fOffsetLen = length(vOffset);
	// 射线穿过球心时退化为任意方向
	float3 vDirToSphere = fOffsetLen > 1e-5 ? vOffset / fOffsetLen : -vReflectDir;
	return cCenter + vDirToSphere * fRadius;
}

// Sphere Light 球形光
void SphereLightIlluminate(in LocalSurfaceInfo surface, in LightUniformBuffer light, inout FragInfo frag){
	// 最近点近似 用球面上离反射射线最近的点代表整个球光
	float3 vN = normalize(surface.vNormal);
	float3 vReflectDir = reflect(-normalize(surface.vView), vN);
	float3 vMRP = SphereRepresentativePoint(surface.vWorldPos, vReflectDir, light.position, light.sizeU);

	float3 vToMRP = vMRP - surface.vWorldPos;
	float fDist = length(vToMRP);
	float3 vLightDir = vToMRP / max(fDist, 1e-5);
	float fAtt = DistanceAttenuation(fDist, light.attenuation0);

	float3 cSpecular, cDiffuse;
	EvaluateDirectLight(surface, vLightDir, light.color * fAtt, cSpecular, cDiffuse);
	FragInfoAccumulate(frag, cSpecular, cDiffuse);
}

// Spot Light 聚光灯
void SpotLightIlluminate(in LocalSurfaceInfo surface, in LightUniformBuffer light, inout FragInfo frag){
	float3 vToLight = light.position - surface.vWorldPos;
	float fDist = length(vToLight);
	float3 vLightDir = vToLight / max(fDist, 1e-5);

	// 距离衰减
	float fAttDist = DistanceAttenuation(fDist, light.attenuation0);
	// 锥角衰减 外锥角到内锥角之间平滑过渡
	// fCosTheta为-vLightDir与灯光朝向的夹角余弦 内锥角内为1 外锥角外为0
	float fCosTheta = dot(-vLightDir, normalize(light.direction));
	float fAttSpot = smoothstep(cos(light.radianOuter), cos(light.radianInner), fCosTheta);

	float3 cSpecular, cDiffuse;
	EvaluateDirectLight(surface, vLightDir, light.color * (fAttDist * fAttSpot), cSpecular, cDiffuse);
	FragInfoAccumulate(frag, cSpecular, cDiffuse);
}

// 矩形光代表性点 反射射线在矩形平面上的最近点
float3 RectRepresentativePoint(float3 vSurfacePos, float3 vReflectDir, in LightUniformBuffer light, float3 vRight, float3 vUp){
	float t = dot(light.position - vSurfacePos, vReflectDir);
	t = max(t, 0.0);
	float3 vRayPoint = vSurfacePos + vReflectDir * t;
	float3 vToRayPoint = vRayPoint - light.position;
	// 限制在矩形范围内
	return light.position
		+ vRight * clamp(dot(vToRayPoint, vRight), -light.sizeU, light.sizeU)
		+ vUp * clamp(dot(vToRayPoint, vUp), -light.sizeV, light.sizeV);
}

// Rect Light 矩形光
void RectLightIlluminate(in LocalSurfaceInfo surface, in LightUniformBuffer light, inout FragInfo frag){
	// 矩形光局部基向量
	float3 vForward = normalize(light.direction);
	float3 vRight = normalize(cross(vForward, float3(0.0, 1.0, 0.0)));
	if (dot(vRight, vRight) < 1e-4){
		// 朝向与Y轴平行时退化 换用X轴
		vRight = normalize(cross(vForward, float3(1.0, 0.0, 0.0)));
	}
	float3 vUp = cross(vRight, vForward);

	// 最近点近似 用矩形平面上离反射射线最近的点代表整个矩形光
	float3 vN = normalize(surface.vNormal);
	float3 vReflectDir = reflect(-normalize(surface.vView), vN);
	float3 vMRP = RectRepresentativePoint(surface.vWorldPos, vReflectDir, light, vRight, vUp);

	float3 vToMRP = vMRP - surface.vWorldPos;
	float fDist = length(vToMRP);
	float3 vLightDir = vToMRP / max(fDist, 1e-5);
	float fAtt = DistanceAttenuation(fDist, light.attenuation0);

	float3 cSpecular, cDiffuse;
	EvaluateDirectLight(surface, vLightDir, light.color * fAtt, cSpecular, cDiffuse);
	FragInfoAccumulate(frag, cSpecular, cDiffuse);
}

// Directional Light 平行光
void DirectionalLightIlluminate(in LocalSurfaceInfo surface, in LightUniformBuffer light, inout FragInfo frag){
	// 平行光无距离衰减 光照方向取灯光朝向的反方向
	float3 vLightDir = -normalize(light.direction);
	float3 cSpecular, cDiffuse;
	EvaluateDirectLight(surface, vLightDir, light.color, cSpecular, cDiffuse);
	FragInfoAccumulate(frag, cSpecular, cDiffuse);
}

// Ambient Light 环境光
void AmbientLightIlluminate(in LocalSurfaceInfo surface, in LightUniformBuffer light, inout FragInfo frag){
	// 环境光只贡献漫反射 无方向性
	float3 cDiffuse = light.color * surface.cDiffuseColor * (1.0 - surface.fMetalness);
	FragInfoAccumulate(frag, float3(0.0, 0.0, 0.0), cDiffuse);
}

// 遍历表面所属的灯光组 计算组内所有灯光的光照贡献
// 调用前需用FragInfoInit初始化frag 多次调用可累加多个灯光组
void Lighting(in LocalSurfaceInfo surface, inout FragInfo frag) {
    // 若surface.nLightGroupID小于0 则不使用任何灯光组
    int nGroupIndex = min(surface.nLightGroupID, LIGHT_GROUP_MAX_COUNT - 1);
    if (nGroupIndex >= 0)
    {
		for (uint i = 0; i < LIGHT_GROUP_LIGHT_COUNT; i++){
			LightUniformBuffer light = u_LightGroups[nGroupIndex][i];
			if (light.nLightType <= LIGHT_TYPE_NONE){
				continue; // 无效槽位
			}
			switch (light.nLightType){
				case LIGHT_TYPE_POINT:
					PointLightIlluminate(surface, light, frag);
					break;
				case LIGHT_TYPE_SPHERE:
					SphereLightIlluminate(surface, light, frag);
					break;
				case LIGHT_TYPE_SPOT:
					SpotLightIlluminate(surface, light, frag);
					break;
				case LIGHT_TYPE_RECT:
					RectLightIlluminate(surface, light, frag);
					break;
				case LIGHT_TYPE_DIRECTIONAL:
					DirectionalLightIlluminate(surface, light, frag);
					break;
				case LIGHT_TYPE_AMBIENT:
					AmbientLightIlluminate(surface, light, frag);
					break;
				default:
					break;
			}
		}
    }
	// 透明度与光照无关 由表面决定
	frag.cColorAcc.a = surface.fAlpha;
}

#endif // LIGHTDEF_SLANG
