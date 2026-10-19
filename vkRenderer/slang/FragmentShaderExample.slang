#include "CommonVertexOutput.slang"

[shader("fragment")]
float4 FragMain(SVertexOutput sVertOut) : SV_Target{
	float4 cOutColor = float4(0,0,0,1);
	
#ifdef VERT_NORMAL
	cOutColor.xyz = (sVertOut.fragNormal + float3(1.0)) * u_MtlProp.ColorDiffuse / 2;
#else
	cOutColor.xyz = u_MtlProp.ColorDiffuse;
#endif

#ifdef MTL_PROP_TexDiffuse
	cOutColor.xyz = float3(1,1,1);
#endif

#ifdef MTL_PROP_TexSpecular
	cOutColor.xyz = float3(1,1,0);
#endif

#ifdef MTL_PROP_TexIOR
	cOutColor.xyz = float3(1,0,0);
#endif

#ifdef MTL_PROP_TexMeltalness
	cOutColor.xyz = float3(0,1,0);
#endif

#ifdef MTL_PROP_TexRoughness
	cOutColor.xyz = float3(0,0,1);
#endif

#ifdef MTL_PROP_TexAlpha
	cOutColor.xyz = float3(0,1,1);
#endif
	
	cOutColor.a = u_MtlProp.Alpha;
	return cOutColor;
}