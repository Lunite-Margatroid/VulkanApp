// BRDF相关函数
// Fresnel: Schlick近似
// 法线分布函数(NDF): Phong Distribution
// 几何函数: Cook-Torrance
#ifndef BRDFLIB_SLANG
#define BRDFLIB_SLANG

#include "ShadeLib.slang"

static const float BRDF_PI = 3.14159265358979;

// 计算F0(入射角0度时的菲涅尔值)
// 非金属由折射率推导 金属F0取高光颜色 按金属度插值
float3 ComputeF0(float3 cSpecularColor, float fMetalness, float fIOR){
	float fF0Dielectric = (fIOR - 1.0) / (fIOR + 1.0);
	fF0Dielectric = fF0Dielectric * fF0Dielectric;
	return lerp(float3(fF0Dielectric, fF0Dielectric, fF0Dielectric), cSpecularColor, fMetalness);
}

// 菲涅尔项 Schlick近似
// fCosTheta: V·H cF0: F0
float3 FresnelSchlick(float fCosTheta, float3 cF0){
	return cF0 + (float3(1.0, 1.0, 1.0) - cF0) * pow(max(0.0, 1.0 - fCosTheta), 5.0);
}

// 法线分布函数 Phong Distribution
// fRoughness: 粗糙度 映射为镜面反射指数 n = 2/α² - 2
// fNDotH: N·H
// 归一化系数(n+2)/(2π) 保证∫D cosθ dω = 1
float PhongDistribution(float fRoughness, float fNDotH){
	float fAlpha = max(fRoughness, 0.001); // 防止除零
	float fShininess = 2.0 / (fAlpha * fAlpha) - 2.0;
	return (fShininess + 2.0) / (2.0 * BRDF_PI) * pow(max(fNDotH, 0.0), fShininess);
}

// 几何函数 Cook-Torrance
// fNDotV: N·V fNDotL: N·L fNDotH: N·H fVDotH: V·H
float CookTorranceGeometry(float fNDotV, float fNDotL, float fNDotH, float fVDotH){
	float fRecip = 2.0 * fNDotH / max(fVDotH, 1e-5);
	float g1 = fRecip * fNDotV;
	float g2 = fRecip * fNDotL;
	return min(1.0, min(g1, g2));
}

// 高光BRDF Cook-Torrance
// fNDotV: N·V fNDotL: N·L fNDotH: N·H fVDotH: V·H
float3 CookTorranceSpecular(float fNDotV, float fNDotL, float fNDotH, float fVDotH, float fRoughness, float3 cF0){
	float3 cF = FresnelSchlick(max(fVDotH, 0.0), cF0);
	float fD = PhongDistribution(fRoughness, fNDotH);
	float fG = CookTorranceGeometry(fNDotV, fNDotL, fNDotH, fVDotH);
	// 分母 4·N·V·N·L 防止除零
	return cF * (fD * fG) / max(4.0 * fNDotV * fNDotL, 1e-5);
}

// 漫反射BRDF Lambert
float3 LambertDiffuse(float3 cDiffuseColor){
	return cDiffuseColor / BRDF_PI;
}

// 计算一次直接光照的BRDF响应
// vLightDir: 从frag指向光源的向量(无需归一化 内部会归一化)
// cLightColor: 灯光颜色(已包含衰减)
// 输出漫反射贡献和高光贡献
void EvaluateDirectLight(in LocalSurfaceInfo surface, float3 vLightDir, float3 cLightColor, out float3 cSpecular, out float3 cDiffuse){
	float3 vN = normalize(surface.vNormal);
	float3 vV = normalize(surface.vView);
	float3 vL = normalize(vLightDir);
	float3 vH = normalize(vV + vL);

	float fNDotV = max(dot(vN, vV), 0.0);
	float fNDotL = max(dot(vN, vL), 0.0);
	float fNDotH = max(dot(vN, vH), 0.0);
	float fVDotH = max(dot(vV, vH), 0.0);

	float3 cF0 = ComputeF0(surface.cSpecularColor, surface.fMetalness, surface.fIOR);
	// 金属度抑制漫反射
	float3 cDiffuseAlbedo = surface.cDiffuseColor * (1.0 - surface.fMetalness);

	cDiffuse = cLightColor * LambertDiffuse(cDiffuseAlbedo) * fNDotL;
	cSpecular = cLightColor * CookTorranceSpecular(fNDotV, fNDotL, fNDotH, fVDotH, surface.fRoughness, cF0) * fNDotL;
}

#endif // BRDFLIB_SLANG
