#include "CommonVertexOutput.slang"

[[vk::binding(1, 0)]]
Sampler2D TexASCII;

[shader("fragment")]
float4 FragMain(SVertexOutput sVertOut) : SV_Target{
	float4 cOutColor = float4(1,1,1,1);
#ifdef VERT_UV1
	cOutColor.a = TexASCII.Sample(sVertOut.fragUV1).r;
#else
	cOutColor.a = TexASCII.Sample(sVertOut.modelPos.xy).r;
#endif

	return cOutColor;
}