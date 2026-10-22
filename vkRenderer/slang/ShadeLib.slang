#include "CommonVertexOutput.slang"

// 着色基础结构定义 表面信息与光照累积结果
#ifndef SHADELIB_SLANG
#define SHADELIB_SLANG

// 表面局部信息 由片段着色器填充后传入光照计算
struct LocalSurfaceInfo{
	float3 vWorldPos;		// 世界坐标
	float3 vNormal;			// 法线
	float3 cSpecularColor;	// 高光颜色
	float3 cDiffuseColor;	// 漫反射颜色
	float3 vView;			// frag to camera
	float fRoughness;		// 粗糙度
	float fMetalness;		// 金属度
	float fIOR;				// 折射率
	float fAlpha;			// 透明度
	int nLightGroupID;		// 灯光组ID 决定该表面使用哪个灯光组
};

// 光照累积结果
struct FragInfo{
	float4 cColorAcc;	// rgb+alpha cColor.rgb = cSpecular + cDiffuse;
	float3 cSpecularAcc;
	float3 cDiffuseAcc;
};

// 初始化光照累积结果
void FragInfoInit(inout FragInfo frag){
	frag.cColorAcc = float4(0.0, 0.0, 0.0, 0.0);
	frag.cSpecularAcc = float3(0.0, 0.0, 0.0);
	frag.cDiffuseAcc = float3(0.0, 0.0, 0.0);
}

// 累加一次光照贡献
// cSpecular: 本次光照的高光贡献 cDiffuse: 本次光照的漫反射贡献
void FragInfoAccumulate(inout FragInfo frag, float3 cSpecular, float3 cDiffuse){
	frag.cSpecularAcc += cSpecular;
	frag.cDiffuseAcc += cDiffuse;
	frag.cColorAcc.rgb += cSpecular + cDiffuse;
}

LocalSurfaceInfo GenLocalSurface(in SVertexOutput sVertOut)
{
    float2 uvMain = sVertOut.fragUV0;

    LocalSurfaceInfo sSurface;
    sSurface.vWorldPos = sVertOut.fragWorldPos.xyz;
	// 法线
#ifdef VERT_NORMAL
    sSurface.vNormal = sVertOut.fragNormal;
#else
    sSurface.vNormal = float3(1, 0, 0);
#endif

// diffuse Color
#if defined(MTL_PROP_TexDiffuse) && defined(MTL_PROP_ColorDiffuse)
    sSurface.cDiffuseColor = TexDiffuse.Sample(uvMain).rgb * u_MtlProp.ColorDiffuse;
#elif defined(MTL_PROP_TexDiffuse) && !defined(MTL_PROP_ColorDiffuse)
    sSurface.cDiffuseColor = TexDiffuse.Sample(uvMain).rgb;
#elif !defined(MTL_PROP_TexDiffuse) && defined(MTL_PROP_ColorDiffuse)
    sSurface.cDiffuseColor = u_MtlProp.ColorDiffuse;
#else
    sSurface.cDiffuseColor = float3(1.0, 1.0, 1.0);
#endif

// Specular Color
#ifdef MTL_PROP_TexSpecular
	#ifdef MTL_PROP_ColorSpecular
    sSurface.cSpecularColor = TexSpecular.Sample(uvMain).rgb * u_MtlProp.ColorSpecular;
	#else
    sSurface.cSpecularColor = TexSpecular.Sample(uvMain).rgb;
	#endif
#else
	#ifdef MTL_PROP_ColorSpecular
	sSurface.cSpecularColor = u_MtlProp.ColorSpecular;
	#else
    sSurface.cSpecularColor = float3(1.0, 1.0, 1.0);
	#endif
#endif


    sSurface.vView = sVertOut.cameraWorldPos - sVertOut.fragWorldPos.xyz;

// Roughness
#if defined(MTL_PROP_TexRoughness) && defined(MTL_PROP_Roughness)
    sSurface.fRoughness = TexRoughness.Sample(uvMain).r * u_MtlProp.Roughness;
#elif defined(MTL_PROP_TexRoughness) && !defined(MTL_PROP_Roughness)
    sSurface.fRoughness = TexRoughness.Sample(uvMain).r;
#elif !defined(MTL_PROP_TexRoughness) && defined(MTL_PROP_Roughness)
    sSurface.fRoughness = u_MtlProp.Roughness;
#else
    sSurface.fRoughness = 0.5;
#endif

// Metalness
#if defined(MTL_PROP_TexMeltalness) && defined(MTL_PROP_Metalness)
    sSurface.fMetalness = TexMeltalness.Sample(uvMain).r * u_MtlProp.Metalness;
#elif defined(MTL_PROP_TexMeltalness) && !defined(MTL_PROP_Metalness)
    sSurface.fMetalness = TexMeltalness.Sample(uvMain).r;
#elif !defined(MTL_PROP_TexMeltalness) && defined(MTL_PROP_Metalness)
    sSurface.fMetalness = u_MtlProp.Metalness;
#else
    sSurface.fMetalness = 0.0;
#endif

// IOR
#if defined(MTL_PROP_TexIOR) && defined(MTL_PROP_IOR)
    sSurface.fIOR = TexIOR.Sample(uvMain).r * u_MtlProp.IOR;
#elif defined(MTL_PROP_TexIOR) && !defined(MTL_PROP_IOR)
    sSurface.fIOR = TexIOR.Sample(uvMain).r;
#elif !defined(MTL_PROP_TexIOR) && defined(MTL_PROP_IOR)
    sSurface.fIOR = u_MtlProp.IOR;
#else
    sSurface.fIOR = 1.5;
#endif

// Alpha
#if defined(MTL_PROP_TexAlpha) && defined(MTL_PROP_Alpha)
    sSurface.fAlpha = TexAlpha.Sample(uvMain).r * u_MtlProp.Alpha;
#elif defined(MTL_PROP_TexAlpha) && !defined(MTL_PROP_Alpha)
    sSurface.fAlpha = TexAlpha.Sample(uvMain).r;
#elif !defined(MTL_PROP_TexAlpha) && defined(MTL_PROP_Alpha)
    sSurface.fAlpha = u_MtlProp.Alpha;
#else
    sSurface.fAlpha = 1.0;
#endif

    sSurface.nLightGroupID = 0;

    return sSurface;
}

#endif // SHADELIB_SLANG
