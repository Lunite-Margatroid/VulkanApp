#include "CommonVertexOutput.slang"

[[vk::binding(0)]]
Sampler2D s_Texture;

[shader("fragment")]
float4 FragMain(SVertexOutput sVertOut) : SV_Target{
	float4 cOutColor = float4(0,0,0,1);
#ifdef VERT_UV0
	cOutColor = s_Texture.Sample(sVertOut.fragUV0);
#else
	float2 uvTemp = sVertOut.fragWorldPos.xy;
	cOutColor = s_Texture.Sample(uvTemp);
#endif
	
	return cOutColor;
	
}