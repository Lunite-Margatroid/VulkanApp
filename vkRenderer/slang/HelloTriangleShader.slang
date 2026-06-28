struct VertexInput{
	float2 vertPosition;
	float3 vertColor;
}; 

struct VertexOutput{
	float4 cv_vertexColor : VERTEX_COLOR0;	// costomed output
	float4 sv_position : SV_Position;	// system value Position
};

struct MVPMatrix{
	float4x4 modelMat;
	float4x4 viewMat;
	float4x4 projectiondMat;
};

ConstantBuffer<MVPMatrix> u_MVPMatix;



[shader("vertex")]
VertexOutput VertMain(VertexInput vertIn){
	VertexOutput output;
	output.sv_position = 
		mul(u_MVPMatix.projectiondMat, 
			mul(u_MVPMatix.viewMat, 
				mul(u_MVPMatix.modelMat, float4(vertIn.vertPosition, 0.0, 1.0))
				)
			);
	//output.sv_position = float4(vertIn.vertPosition, 0.0, 1.0);
	//output.cv_vertexColor =float4(u_MVPMatix.modelMat[0].xy,1.0, 1.0);
	output.cv_vertexColor = float4(vertIn.vertColor, 1.0);
	return output;
}

[shader("fragment")]
float4 FragMain(VertexOutput sVertOut):SV_Target
{
	return sVertOut.cv_vertexColor;
	//return vecVertColor;
}