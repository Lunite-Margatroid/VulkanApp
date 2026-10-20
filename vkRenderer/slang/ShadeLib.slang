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
	uint nLightGroupID;		// 灯光组ID 决定该表面使用哪个灯光组
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

#endif // SHADELIB_SLANG
