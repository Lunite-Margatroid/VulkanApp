#include "CommonVertexOutput.slang"

[shader("fragment")]
float4 FragMain(SVertexOutput sVertOut) : SV_Target{
	return sVertOut.fragVertexColor;
}