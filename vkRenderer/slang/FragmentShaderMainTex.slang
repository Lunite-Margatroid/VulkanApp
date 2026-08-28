#include "CommonVertexOutput.slang"

// (binding, set)
// binding = 0
// set = 1 : FragmentShader
[[vk::binding(1, 1)]]
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