static float2 position[3] = float2[](
	float2(0.0, -0.5),
	float2(-0.5, 0.5),
	float2(0.5, 0.5)
);

static float4 colors[3] = float4[](
	float4(1, 0, 0, 1),
	float4(0, 1, 0, 1),
	float4(0, 0, 1, 1)
);

struct VertexOutput{
	float4 cv_vertexColor : VERTEX_COLOR0;	// costomed output
	float4 sv_position : SV_Position;	// system value Position
};

[shader("vertex")]
VertexOutput VertMain(uint vid : SV_VertexID){
	VertexOutput output;
	output.sv_position = float4(position[vid], 0.0, 1.0);
	output.cv_vertexColor = colors[vid];
	return output;
}

[shader("fragment")]
float4 FragMain(VertexOutput sVertOut):SV_Target
{
	return sVertOut.cv_vertexColor;
	//return vecVertColor;
}