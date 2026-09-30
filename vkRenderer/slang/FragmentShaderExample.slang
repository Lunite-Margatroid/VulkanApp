#include "CommonVertexOutput.slang"

[shader("fragment")]
float4 FragMain(SVertexOutput sVertOut) : SV_Target{
	float4 cOutColor = float4(0,0,0,1);
	cOutColor.xyz = u_MtlProp.ColorDiffuse;
	cOutColor.a = u_MtlProp.Alpha;
	return cOutColor;
}