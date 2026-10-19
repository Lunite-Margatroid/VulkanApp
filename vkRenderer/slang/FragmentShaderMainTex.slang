#include "CommonVertexOutput.slang"

[shader("fragment")]
float4 FragMain(SVertexOutput sVertOut) : SV_Target{
	float4 cOutColor = float4(0,0,0,1);
	float2 uvMain;
	float3 cDiffuse;
	
#ifdef VERT_UV0
	uvMain = sVertOut.fragUV0;
#else
	uvMain = sVertOut.fragWorldPos.xy;
#endif

#ifdef MTL_PROP_TexDiffuse
	cDiffuse = TexDiffuse.Sample(uvMain);
#else
	cDiffuse = float3(0,0,0);
#endif

	cOutColor.rgb = cDiffuse;
	
	return cOutColor;
	
}