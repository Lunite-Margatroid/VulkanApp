#include "CommonVertexOutput.slang"

[shader("fragment")]
float4 FragMain(SVertexOutput sVertOut) : SV_Target{
	float4 cOutColor = float4(0,0,0,1);
#ifdef VERT_NORMAL
	cOutColor.xyz = (sVertOut.fragNormal + float3(1.0)) / 2;
#else
	cOutColor.xyz = u_MtlProp.ColorDiffuse;
#endif
	
	cOutColor.a = u_MtlProp.Alpha;
	return cOutColor;
}